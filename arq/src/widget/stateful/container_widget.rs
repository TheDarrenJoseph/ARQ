use crate::engine::event::container::DropItemsRequest;
use crate::engine::event::container::MoveItemsWithinSourceRequest;
use crate::engine::event::container::OpenContainerRequest;
use crate::engine::event::container::OpenedContainerEventData;
use crate::engine::event::container::OpenedContainerEventType;
use crate::engine::event::container::TakeItemsRequest;
use crate::engine::event::container::{ContainerChoicesRequest, MoveItemsResponseV2};
use crate::engine::event::ui::AppEventType::OpenedContainerEvent;
use crate::engine::event::ui::UIEvent;
use crate::item_list_selection::{ItemListSelection, ListSelection};
use crate::map::objects::container::Container;
use crate::map::objects::items::Item;
use crate::map::position::Area;
use crate::ui::ui_util::build_paragraph;
use crate::view::util::paging::{build_page_count, build_weight_limit};
use crate::view::util::tabling::{build_headings, Column};
use crate::widget::standard::usage_line::UsageCommand;
use log::{error, info};
use ratatui::buffer::Buffer;
use ratatui::layout::Rect;
use ratatui::prelude::{Color, Modifier, StatefulWidget, Style};
use ratatui::widgets::{Block, Borders, Widget};
use std::convert::TryInto;
use termion::event::Key;
use tokio::sync::mpsc;
use tokio::sync::mpsc::UnboundedSender;
use uuid::Uuid;
use crate::engine::command::util::CurrentContainersData;
use crate::widget::stateful::container_choice_widget::ContainerChoiceWidgetData;

#[derive(Debug, Clone)]
pub struct ContainerWidget {
    pub(crate) container_id: Uuid,
    pub(crate) columns : Vec<Column>,
    pub(crate) row_count: i32
}

impl ContainerWidget {
    pub(crate) fn new(container_id: Uuid) -> ContainerWidget {
        ContainerWidget {
            container_id,
            columns: build_columns(),
            row_count: 1,
        }
    }
}

#[derive(Debug, Clone)]
pub struct ContainerWidgetData {
    pub container : Container,
    pub ui_area: Area,
    pub item_list_selection : ItemListSelection,
    pub usage_commands: Vec<UsageCommand>,
    pub event_sender: mpsc::UnboundedSender<UIEvent>
}

pub struct WorldContainerWidgetData {
    pub container_widget_data: ContainerWidgetData,
    pub container_choice_data: Option<ContainerChoiceWidgetData>
}

impl ContainerWidgetData {
    pub(crate) fn new(
        container: Container,
        ui_area: Area,
        line_count: i32,
        usage_commands: Vec<UsageCommand>,
        sender: UnboundedSender<UIEvent>
    ) -> ContainerWidgetData {
        let items = container.to_cloned_item_list();
        let item_list_selection =  ItemListSelection::new(items.clone(), line_count.into());
        ContainerWidgetData {
            container: container.clone(),
            ui_area: ui_area.clone(),
            item_list_selection,
            usage_commands: usage_commands,
            event_sender: sender
        }
    }

}

impl ContainerWidgetData {
    fn get_focused_container(&self) -> Option<&Container> {
        let focused_item = self.item_list_selection.get_focused_item().unwrap();
        if let Some(focused_container) = self.container.find(focused_item) {
            if focused_container.is_true_container() {
                return Some(focused_container);
            }
        }
        None
    }

    // Handles the high level OpenedContainerEventType for a usage command input
    // and emits the fully formed event UIEvent::AppEvent(OpenedContainerEvent..) for it
    pub async fn handle_usage_command(&mut self, usage_command: UsageCommand) {
        // Handle inputs that start usage command based events
        if let Some(container_event_type) = &usage_command.opened_container_event_type {
            match container_event_type {
                OpenedContainerEventType::Close => {
                    // As a priority - close will cancel any selection in progress to allow resetting it
                    if self.item_list_selection.is_selecting()  {
                        self.item_list_selection.cancel_selection();
                    } else {
                        // If there is no state to modify, the escape intention is to close the window
                        self.event_sender.send(UIEvent::AppEvent(OpenedContainerEvent(OpenedContainerEventType::Close, None))).expect("Failed to send event");
                    }
                },
                OpenedContainerEventType::OpenContainer => {
                    if !self.item_list_selection.is_selecting() {
                        // Find and clone either the focused container or item
                        let focused_container_result = self.get_focused_container().map(|c| c.clone());
                        if let Some(focused_container) = focused_container_result {
                            if focused_container.is_true_container() {
                                info!("Opening focused container: {:?}", focused_container.get_self_item().get_id());
                                let source_container_id = self.container.get_self_item().get_id();
                                let data = OpenContainerRequest {
                                    source_container_id,
                                    target: focused_container.clone(),
                                };
                                self.event_sender.send(
                                    UIEvent::AppEvent(OpenedContainerEvent(OpenedContainerEventType::OpenContainer, Some(OpenedContainerEventData::OpenContainer(data))))
                                ).expect("Error sending event");
                            }
                        } else {
                            error!("Could not find focused container to open");
                        }
                    }
                },
                OpenedContainerEventType::TakeItems=> {
                    let selected_items = Vec::from(self.item_list_selection.get_selected_items().clone());
                    if (selected_items.len() == 0) {
                        info!("No items selected, skipping TakeItems");
                        return;
                    }

                    let selected_items = Vec::from(self.item_list_selection.get_selected_items().clone());
                    let data = TakeItemsRequest { source: self.container.clone(), to_take: selected_items, position: None };
                    self.event_sender.send(
                        UIEvent::AppEvent(OpenedContainerEvent(OpenedContainerEventType::TakeItems, Some(OpenedContainerEventData::TakeItems(data))))
                    ).expect("Error sending event");
                },
                OpenedContainerEventType::MoveItems => {
                    let selected_items = Vec::from(self.item_list_selection.get_selected_items().clone());
                    if (selected_items.len() == 0) {
                        info!("No items selected, skipping MoveItems");
                        return;
                    }

                    let focused_item = self.item_list_selection.get_focused_item().unwrap();
                    if (selected_items.contains(focused_item)) {
                        info!("Cannot move to a selected item, skipping MoveItems");
                        return;
                    }

                    focused_item.is_container();

                    // Find and clone either the focused container or item
                    let target_container = self.get_focused_container().map(|c| c.clone());
                    let target_item = self.item_list_selection.get_focused_item().map(|i| i.clone());

                    let data = MoveItemsWithinSourceRequest {
                        source_container: self.container.clone(),
                        to_move: selected_items,
                        target_container,
                        target_position_item: target_item,
                    };
                    self.event_sender.send(
                        UIEvent::AppEvent(OpenedContainerEvent(OpenedContainerEventType::MoveItems, Some(OpenedContainerEventData::MoveItemsWithinSource(data))))
                    ).expect("Error sending event");
                },
                OpenedContainerEventType::DropItems => {
                    let selected_items = Vec::from(self.item_list_selection.get_selected_items().clone());
                    if (selected_items.len() == 0) {
                        info!("No items selected, skipping DropItems");
                        return;
                    }

                    let data = DropItemsRequest { source: self.container.clone(), to_drop: selected_items, position: None };
                    self.event_sender.send(
                        UIEvent::AppEvent(OpenedContainerEvent(OpenedContainerEventType::DropItems, Some(OpenedContainerEventData::DropItems(data))))
                    ).expect("Error sending event");
                },
                OpenedContainerEventType::ShowMoveContainerChoices => {
                    let selected_items = Vec::from(self.item_list_selection.get_selected_items().clone());
                    if (selected_items.len() == 0) {
                        info!("No items selected, skipping ShowMoveContainerChoices");
                        return;
                    }
                    
                    let data = ContainerChoicesRequest { source: self.container.clone(), to_move: selected_items, position: None };
                    self.event_sender.send(
                        UIEvent::AppEvent(OpenedContainerEvent(OpenedContainerEventType::ShowMoveContainerChoices, Some(OpenedContainerEventData::MoveItemsToContainerChoice(data))))
                    ).expect("Error sending event");
                }
                _ => {
                    info!("Unsupported OpenedContainerEventType {:?}", container_event_type)
                }
            }
        }
    }

    pub fn handle_move_items_response(&mut self, response: MoveItemsResponseV2) {
        // Only listen to messages relevant to the container we are displaying
        let current_container_id = self.container.get_self_item().get_id();
        let updated_source_id = response.request.source.get_self_item().get_id();
        if current_container_id == updated_source_id {
            let target = response.request.target;
            let source = response.request.source;

            if let Some(target_container) =  target.get_container() {
                if target_container.id_equals(&self.container)  {
                    let updated_container = response.updated_scopes.target.get_container().unwrap();
                    // Update the current container details to the updated target
                    self.container = updated_container;
                }
            }

            // If this is the source them items have been moved FROM this container
            if source.get_container().id_equals(&self.container)  {
                let updated_container = response.updated_scopes.source.get_container();
                // Otherwise, items have been moved FROM this container
                // Update the current container details to the updated source
                self.container = updated_container.clone();
            }

            self.item_list_selection.cancel_selection();
            self.rebuild_selection();
        }
    }

    pub fn handle_move_items_to_choice_response(&mut self, response: MoveItemsResponseV2) {
        let self_container_id = self.container.get_self_item_id();
        // If this is the source container
        if (self_container_id == response.request.source.get_self_item().get_id()) {
            let updated_source = response.updated_scopes.source;
            // Update the current container details to the updated source
            self.container = updated_source.get_container().clone();
            self.item_list_selection.cancel_selection();
            self.rebuild_selection();
        }

        // If this is the target container
        if (self_container_id == response.request.target.get_self_item().get_id()) {
            let updated_target = response.updated_scopes.target;
            // Update the current container details to the updated target
            self.container = updated_target.get_container().unwrap().clone();
            self.item_list_selection.cancel_selection();
            self.rebuild_selection();
        }
    }

    pub async fn handle_event(&mut self, event: UIEvent) {
        log::debug!("[container_widget] Handling event: {:?}", event.name());
        match event {
            UIEvent::Termion(termion_event) => {
                match termion_event {
                    termion::event::Event::Key(key) => {
                        match key {
                            // These are key specific as they are not attached to events and thus are purely UI controls for the widget
                            // These may move into some bindings in future to make them dynamic instead of hardcoded
                            Key::Up => {
                                self.item_list_selection.move_up();
                            },
                            Key::Down => {
                                self.item_list_selection.move_down();
                            },
                            Key::PageUp => {
                                self.item_list_selection.page_up();
                            },
                            Key::PageDown => {
                                self.item_list_selection.page_down();
                            },
                            Key::Backspace | Key::Char('\n') => {
                                self.item_list_selection.toggle_select();
                            },
                            // Check commands tied to character keys
                            Key::Char(c) => {
                                let matching_command = self.usage_commands.iter().find(|uc| uc.key == Key::Char(c));
                                if let Some(uc) = matching_command {
                                    self.handle_usage_command(uc.clone()).await
                                }
                            }
                            // Check commands tied to non-character keys
                            k => {
                                let matching_command = self.usage_commands.iter().find(|uc| uc.key == k);
                                if let Some(uc) = matching_command {
                                    self.handle_usage_command(uc.clone()).await
                                }
                            }
                        }

                    }
                    _ => {}
                }
            },
            UIEvent::AppEvent(OpenedContainerEvent(OpenedContainerEventType::TakeItemsResult, Some(OpenedContainerEventData::TakeItemsResult(take_items_response)))) => {
                // Only listen to messages relevant to the container we are displaying
                let source_container_id = self.container.get_self_item().get_id();
                if source_container_id == take_items_response.container_id {
                    self.retain_selected_items(take_items_response.untaken);
                }
            },
            UIEvent::AppEvent(OpenedContainerEvent(OpenedContainerEventType::MoveItemsResult, Some(OpenedContainerEventData::MoveItemsWithSourceResult(response)))) => {
                self.handle_move_items_response(response)
            }
            UIEvent::AppEvent(OpenedContainerEvent(OpenedContainerEventType::DropItemsResult, Some(OpenedContainerEventData::DropItemsResult(drop_items_response)))) => {
                // Only listen to messages relevant to the container we are displaying
                let source_container_id = self.container.get_self_item().get_id();
                if source_container_id == drop_items_response.container_id {
                    self.retain_selected_items(drop_items_response.undropped);
                }
            },
            UIEvent::AppEvent(OpenedContainerEvent(OpenedContainerEventType::MoveToContainerChoiceResult, Some(OpenedContainerEventData::MoveItemsToContainerChoiceResult(response)))) => {
                self.handle_move_items_to_choice_response(response)
            }
            _ => {}
        }
    }

    fn clone_selected_container_items(&mut self) -> Vec<Container> {
        let mut items = Vec::new();
        let selected_items = self.item_list_selection.get_selected_items();
        for item in selected_items {
            if let Some(found) = self.container.find(&item) {
                items.push(found.clone());
            }
        }
        items
    }
    
    fn retain_selected_items(&mut self, to_retain: Vec<Item>) {
        let mut droppable_containers = self.clone_selected_container_items();
        if !droppable_containers.is_empty() {
            let view_container = &mut self.container;
            for retainable in to_retain {
                if let Some(pos) = droppable_containers.iter().position(|c| *c.get_self_item() == retainable) {
                    droppable_containers.remove(pos);
                }
            }
            view_container.remove_matching_items(droppable_containers);
            self.rebuild_selection();
        }
    }

    pub fn rebuild_selection(&mut self) {
        self.item_list_selection = ItemListSelection::new(self.container.to_cloned_item_list(), 1);
    }

}

fn build_columns() -> Vec<Column> {
    vec![
        Column {name : "NAME".to_string(), size: 30},
        Column {name : "WEIGHT (Kg)".to_string(), size: 12},
        Column {name : "VALUE".to_string(), size: 12}
    ]
}

fn build_item_column_text(column: &Column, item: &Item) -> String {
    match column.name.as_str() {
        "NAME" => {
            if item.is_equipped() {
              format!("{} ({})", item.get_name(), item.get_equipment_slot().unwrap())
            } else {
              item.get_name()
            }
        },
        "WEIGHT (Kg)" => {
            item.get_weight().to_string()
        },
        "VALUE" => {
             item.get_value().to_string()
        },
        _ => { "".to_string() }
    }
}

fn build_container_column_text(column: &Column, container: &Container) -> String {
    let self_item =  container.get_self_item();
    match column.name.as_str() {
        "NAME" => {
            self_item.get_name()
        },
        "WEIGHT (Kg)" => {
            container.get_weight_total().to_string()
        },
        "VALUE" => {
            container.get_loot_value().to_string()
        },
        _ => { "".to_string() }
    }
}


impl StatefulWidget for ContainerWidget {
    type State = ContainerWidgetData;

    fn render(mut self, _: Rect, buf: &mut Buffer, data: &mut ContainerWidgetData) {
        let main_area = data.ui_area;
        let frame_size = main_area.to_rect();
        
        let container = &mut data.container;
        let item_list_selection = &mut data.item_list_selection;
        //let usage_line = &mut data.usage_line;

        let window_block = Block::default()
            .borders(Borders::ALL)
            .title(container.get_self_item().get_name().clone());
        let window_area = Rect::new(frame_size.x.clone(), frame_size.y.clone(), frame_size.width.clone(), frame_size.height.clone());
        let inventory_item_lines = window_area.height - 3;
        self.row_count = inventory_item_lines as i32;
        item_list_selection.page_line_count = inventory_item_lines as i32;
        window_block.render(window_area, buf);

        let headings = build_headings(self.columns.clone());
        let headings_area = Rect::new(frame_size.x.clone() + 1, frame_size.y.clone() + 1, frame_size.width.clone() - 4, 2);
        headings.render(headings_area, buf);

        let mut line_index = 0;
        let start_index = item_list_selection.get_start_index();
        let end_of_page_representive_index = item_list_selection.get_end_of_page_index();

        if !container.get_contents().is_empty() {
            let view_contents = &container.get_contents()[start_index as usize..=end_of_page_representive_index as usize];
            for c in view_contents {
                let item_index = start_index.clone() + line_index.clone();
                let item = &c.get_self_item();
                // The x offset is the starting x 
                // + 1 to avoid the left-hand border
                let mut x_offset: u16 = frame_size.x.clone() + 1;
                // The y offset is the starting y 
                // + 2 (to avoid top border and the header row) 
                // + line index (to avoid previous lines)
                let y_offset: u16 = frame_size.y.clone() as u16 + 2 + line_index.clone() as u16;

                let current_index = item_list_selection.is_focused(item_index);
                let selected = item_list_selection.is_selected(item_index);

                for column in &self.columns {

                    let column_text = if c.is_true_container() {
                        build_container_column_text(column, c)
                    }  else {
                        build_item_column_text(column, item)
                    };

                    let mut column_text = build_paragraph(column_text);
                    if current_index.clone() && selected.clone() {
                        column_text = column_text.style(Style::default().fg(Color::Green).add_modifier(Modifier::REVERSED));
                    } else if current_index {
                        column_text = column_text.style(Style::default().add_modifier(Modifier::REVERSED));
                    } else if selected {
                        column_text = column_text.style(Style::default().fg(Color::Green));
                    }

                    let column_length = column.size as i8;
                    let text_area = Rect::new(x_offset.clone(), y_offset.clone(), column_length.try_into().unwrap(), 1);
                    column_text.render(text_area, buf);
                    x_offset += column_length as u16;
                }
                line_index += 1;
            }
            
            // TODO do we want the widget to have this?
            //let usage_description = usage_line.describe();
            //let usage_text = build_paragraph(usage_description.clone());
            //let text_area = Rect::new(window_area.x.clone() + 1, window_area.y.clone() + window_area.height.clone() - 1, usage_description.len().try_into().unwrap(), 1);
            //usage_text.render(text_area, buf);

            // From right hand to left hand side draw the info text
            let page_count = build_page_count(&item_list_selection, window_area.clone());
            page_count.0.render(page_count.1, buf);

            let page_count_text_length = page_count.2;
            let weight_limit = build_weight_limit(&data.container, window_area.clone(), page_count_text_length);
            weight_limit.0.render(weight_limit.1, buf);
        }
    }
}