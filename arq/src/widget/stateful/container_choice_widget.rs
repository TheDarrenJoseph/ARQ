use crate::engine::event::event::UIEventHandler;
use crate::engine::event::container::OpenedContainerEventData::MoveItemsToContainerChoiceSelection;
use crate::engine::event::container::OpenedContainerEventType;
use crate::engine::event::container::TargetContainerScope;
use crate::engine::event::ui::AppEventType::OpenedContainerEvent;
use crate::engine::event::ui::UIEvent;
use crate::item_list_selection::{ItemListSelection, ListSelection};
use crate::map::objects::items::Item;
use crate::map::position::Area;
use crate::ui::ui_util::build_paragraph;
use crate::view::util::tabling::build_headings;
use crate::view::util::tabling::Column;
use crate::widget::standard::usage_line::UsageCommand;
use crate::widget::stateful::container_choice_widget::OpenedContainerEventType::Close;
use log::info;
use ratatui::buffer::Buffer;
use ratatui::layout::Rect;
use ratatui::prelude::{Modifier, StatefulWidget, Style};
use ratatui::prelude::Widget;
use ratatui::widgets::{Block, Borders, Clear};
use termion::event::Key;
use tokio::sync::mpsc;
use tokio::sync::mpsc::UnboundedSender;
use crate::map::objects::container::Container;
use crate::ui::ui_areas::{UIArea, UIAreas, UI_AREA_NAME_MAIN};
use crate::widget::stateful::container_widget::ContainerWidgetData;

#[derive(Clone, Debug)]
pub enum ContainerChoiceScope {
    PlayerInventory,
    WorldContainer
}


#[derive(Debug, Clone)]
pub struct ContainerChoice {
    pub container_scope: TargetContainerScope,
    pub location_name: String
}

fn build_columns() -> Vec<Column> {
    vec![
        Column {name : "NAME".to_string(), size: 30},
        Column {name : "STORAGE (Kg)".to_string(), size: 12},
        Column {name : "LOCATION".to_string(), size: 36}
    ]
}

fn build_column_text(column: &Column, choice: &ContainerChoice) -> String {
    match column.name.as_str()  {
        "NAME" => {
            choice.container_scope.build_container_choice_column_text(column)
        },
        "STORAGE (Kg)" => {
            choice.container_scope.build_container_choice_column_text(column)
        },
        "LOCATION" => {
            choice.location_name.clone()
        },
        _ => { "".to_string() }
    }
}

#[derive(Debug, Clone)]
pub struct ContainerChoiceWidget {
    pub(crate) columns : Vec<Column>
}

impl ContainerChoiceWidget {
    pub(crate) fn new() -> ContainerChoiceWidget {
        ContainerChoiceWidget {
            columns: build_columns()
        }
    }
}

#[derive(Debug, Clone)]
pub struct ContainerChoiceWidgetData {
    pub scope: ContainerChoiceScope,
    pub choices: Vec<ContainerChoice>,
    pub item_list_selection : ItemListSelection,
    pub ui_area: Area,
    pub usage_commands: Vec<UsageCommand>,
    pub event_sender: mpsc::UnboundedSender<UIEvent>,
}

impl ContainerChoiceWidgetData {
    pub fn new(
        choices: Vec<ContainerChoice>,
        ui_area: Area,
        line_count: i32,
        sender: UnboundedSender<UIEvent>
    ) -> ContainerChoiceWidgetData {
        let choice_items = convert_to_item_list(choices.clone());
        let item_list_selection =  ItemListSelection::new(choice_items, line_count.into());
        ContainerChoiceWidgetData {
            scope: ContainerChoiceScope::PlayerInventory,
            choices,
            item_list_selection,
            ui_area,
            usage_commands: vec![
                UsageCommand::for_container_event(Key::Char('\n'), String::from("Select"), OpenedContainerEventType::MoveToContainerChoiceSelection),
                UsageCommand::for_container_event(Key::Esc, String::from("Cancel"), Close),
            ],
            event_sender: sender,
        }
    }

    pub async fn handle_usage_command(&mut self, usage_command: UsageCommand) {
        if let Some(container_event_type) = &usage_command.opened_container_event_type {
            match container_event_type {
                OpenedContainerEventType::MoveToContainerChoiceSelection => {
                    let target = match self.scope {
                        _ => {
                            let focused_item = self.item_list_selection.get_focused_item().unwrap();
                            let chosen_choice = self.choices.iter().find(|c| c.container_scope.matches_item(focused_item));
                            chosen_choice.unwrap().container_scope.clone()
                        }
                    };
                    self.event_sender.send(UIEvent::AppEvent(OpenedContainerEvent(OpenedContainerEventType::MoveToContainerChoiceSelection, Some(MoveItemsToContainerChoiceSelection(target)))))
                        .expect("Failed to send event");
                },
                Close => {
                    self.event_sender.send(UIEvent::AppEvent(OpenedContainerEvent(Close, None)))
                        .expect("Failed to send event");
                },
                _ => {
                    info!("Unsupported OpenedContainerEventType {:?}", container_event_type)
                }
            }
        }
    }

}

impl UIEventHandler for ContainerChoiceWidgetData {
    async fn handle_event(&mut self, event: UIEvent) {
        log::debug!("[container_choice_widget] Handling event: {:?}", event.name());
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
                            },
                            _ => {}
                        }
                    },
                    _ => {}
                }
            },
            _ => {}
        }
    }
}

impl StatefulWidget for ContainerChoiceWidget {
    type State = ContainerChoiceWidgetData;

    fn render(self, area: Rect, buf: &mut Buffer, _state: &mut Self::State) {
        let data = _state;

        let mut ui_area : Area = data.ui_area.clone();
        // Move the start y down one to prevent overlapping with the stat bars
        ui_area.start_position.y += 1;

        let window_rect = ui_area.to_rect();
        let _line_count = data.item_list_selection.page_line_count;

        let clear = Clear::default();
        clear.render(area, buf);

        let window_block = Block::default()
            .title("Choose container to move items into")
            .borders(Borders::ALL);

          window_block.render(window_rect, buf);

        let inventory_item_lines = ui_area.height - 3;
        data.item_list_selection.page_line_count = inventory_item_lines as i32;

        let headings_paragraph = build_headings(self.columns.clone());
        let headings_area = Rect::new(window_rect.x.clone() + 1, window_rect.y.clone() + 1, window_rect.width.clone() - 4, 2);
        headings_paragraph.render(headings_area, buf);

        let _items = data.item_list_selection.get_items();

        let mut line_index = 0;
        let _start_index= data.item_list_selection.get_start_index();
        let start_index = 0;
        let _end_of_page_representive_index = data.item_list_selection.get_end_of_page_index();

        for choice in &data.choices {
            let item_index = start_index.clone() + line_index.clone();
            let mut x_offset: u16 = window_rect.x.clone() as u16 + 1;
            let y_offset: u16 = window_rect.y.clone() as u16 + 2 + line_index.clone() as u16;
            for column in &self.columns {
                let text = build_column_text(column, &choice);
                let mut column_paragraph = build_paragraph(text);
                if data.item_list_selection.is_focused(item_index) {
                    column_paragraph = column_paragraph.style(Style::default().add_modifier(Modifier::REVERSED));
                }

                let column_length = column.size as u16;
                let text_area = Rect::new(x_offset.clone(), y_offset.clone(), column_length, 1);
                column_paragraph.render(text_area, buf);

                x_offset += column_length as u16;
            }
            line_index += 1;
        }

    }
}


fn convert_to_item_list(choices : Vec<ContainerChoice>) -> Vec<Item> {
    let mut items = Vec::new();
    for c in choices {
        items.push(c.container_scope.get_self_item().clone());
    }
    items
}

pub fn build_container_widget_area(main_area: &UIArea) -> Area {
    let mut result = main_area.clone();
    let ui_area = &mut result.area;
    // Offset the container start y to allow for tabs
    ui_area.start_position.y += 2;
    ui_area.height -= 2;
    ui_area.end_position.y -= 2;
    result.area
}

pub fn build_container_choice_widget_data(choices: Vec<ContainerChoice>, ui_areas: UIAreas, container_event_sender: UnboundedSender<UIEvent>) -> ContainerChoiceWidgetData {
    let main_area = ui_areas.get_area(UI_AREA_NAME_MAIN).unwrap();
    let widget_ui_area = build_container_widget_area(&main_area);

    // -3 to account for:
    // 1. Title / Border top
    // 2. Table headings
    // 3. Border bottom
    let line_count = (widget_ui_area.height - 3) as i32;

    ContainerChoiceWidgetData::new(
        choices,
        main_area.area,
        line_count,
        container_event_sender
    )
}