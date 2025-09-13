use std::collections::HashMap;
use log::{debug, error, info};
use std::io::Error;
use ratatui::prelude::{Line, Modifier, Style};
use ratatui::symbols::line::VERTICAL;
use ratatui::widgets::{Block, Borders, Tabs};
use tokio::sync::mpsc;
use tokio::sync::mpsc::UnboundedSender;
use uuid::Uuid;
use crate::character::Character;
use crate::character::equipment::get_potential_slots;
use crate::engine::command::command::Command;
use crate::engine::command::open_command::{OpenCommandChannels, OpenedContainerEventType};
use crate::engine::command::util::CurrentContainersData;
use crate::engine::container_util;
use crate::engine::level::Level;
use crate::error::errors::ErrorWrapper;
use crate::item_list_selection::ItemListSelection;
use crate::map::objects::container::Container;
use crate::map::objects::items::Item;
use crate::map::position::{Area, Position};
use crate::terminal::terminal_manager::TerminalManager;
use crate::ui::bindings::action_bindings::Action;
use crate::ui::bindings::inventory_bindings::InventoryInput;
use crate::ui::event::{Event, TerminalEventHandler};
use crate::ui::event::AppEventType::OpenedContainerEvent;
use crate::ui::ui::{UIViewMode, UI};
use crate::ui::ui_areas::{UIAreas, UI_AREA_NAME_MAIN};
use crate::ui::ui_layout::LayoutType;
use crate::view::character_info_view::{CharacterInfoView, Tab, TabChoice};
use crate::view::framehandler::character_info::CharacterInfoFrameHandler;
use crate::view::framehandler::container::ContainerFrameHandlerInputResult::{DropItems, EquipItems, MoveItems, MoveToContainerChoice};
use crate::view::framehandler::container::{ContainerFrameHandlerInputResult, MoveItemsData, MoveToContainerChoiceData};
use crate::view::util::callback::Callback;
use crate::view::View;
use crate::widget::standard::usage_line::UsageCommand;
use crate::widget::stateful::character_info_widget::{CharacterInfoWidget, CharacterInfoWidgetData};
use crate::widget::stateful::container_widget::{ContainerWidget, ContainerWidgetData};
use crate::widget::{StandardWidgetType, StatefulWidgetType};

const UI_USAGE_HINT: &str = "Up/Down - Move, Enter/q - Toggle/clear selection\nTab - Change tab, Esc - Exit";

/*
    This command is responsible for opening and managing the Inventory/Equipment/Character Sheet "Character Info" display
 */
pub struct CharacterInfoCommand<'a, B: 'static + ratatui::backend::Backend> {
    pub level: &'a mut Level,
    pub ui: &'a mut UI,
    pub terminal_manager : &'a mut TerminalManager<B>,
    pub widget_data: Option<CharacterInfoWidgetData>
}

async fn handle_container_event<'a, B: ratatui::backend::Backend>(
    terminal_manager: &mut TerminalManager<B>, // Necessary for building new widgets / widget data
    ui: &'a mut UI, // Necessary for building new widgets / widget data
    level: &'a mut Level, // Necessary to make actual changes to the level / world
    player_position: Position, // Needed for DropItems
    event: Event,
    event_handler: &mut TerminalEventHandler, // This provides terminal IO input (key input)
    containers_data: &mut CurrentContainersData, // Tracks the currently open containers / relevant widget data
    child_container_sender: UnboundedSender<Event>,
) -> bool {
    debug!("Handling character_info container event");
    let frame_size = terminal_manager.terminal.get_frame().area();
    let mut ui_layout = ui.ui_layout.clone().unwrap();
    let ui_areas = ui_layout.get_or_build_areas(frame_size, LayoutType::StandardSplit);

    let current_container_id = containers_data.current_container_id.unwrap().clone();
    let widget_data_by_id = &mut containers_data.widget_data_by_id;
    let container_ids = &mut containers_data.container_ids;

    match event {
        Event::AppEvent(OpenedContainerEvent(OpenedContainerEventType::Escape)) => {
            // TODO can this be refactored to be shared between this and open_command?
            let closing_container_id = current_container_id.clone();
            if (widget_data_by_id.len() > 1) {
                // If we have more than one container opened, remove the current one
                let stateful_widgets = ui.get_stateful_widgets_mut();
                let current_widget_index = stateful_widgets.iter().position(|widget| {
                    return match widget {
                        StatefulWidgetType::Container(container_widget) => {
                            container_widget.container_id == closing_container_id
                        },
                        _ => {
                            false
                        }
                    }
                });
                if let Some(idx) = current_widget_index {
                    // The index of the parent container
                    let parent_container_id = container_ids.get(container_ids.len() - 2).unwrap();

                    // Update the parent container with the updated current container contents
                    let current_widget_data = widget_data_by_id.get(&current_container_id).unwrap().clone();
                    let mut parent_container_widget_data = widget_data_by_id.get_mut(parent_container_id).unwrap();
                    let updated_container = current_widget_data.container.clone();
                    parent_container_widget_data.container.replace_container(updated_container);

                    // Remove the widget and it's data
                    stateful_widgets.remove(idx);
                    widget_data_by_id.remove(&current_container_id);

                    // Drop the last container id from the list
                    container_ids.pop();
                    // Set the current container ID to the last one in the list
                    containers_data.current_container_id = Some(container_ids.last().unwrap().clone());
                }
            } else if (widget_data_by_id.len() == 1) {
                // If only one container is open, stop the command
                return false;
            }
        },
        _ => {}
    }

    // Keep running by default
    true
}

impl <B: ratatui::backend::Backend> CharacterInfoCommand<'_, B> {
    pub async fn start(&mut self) -> Result<(), ErrorWrapper> {
        log::info!("Player opening Character Info Screen.");

        // Build the CharacterInfoWidgetData and return the required event channels for communicating
        let channels = self.bootstrap().await;

        let mut container_event_receiver = channels.container_event_receiver;
        let child_container_sender = channels.child_container_sender;

        let terminal_manager = &mut self.terminal_manager;
        let ui = &mut self.ui;

        ui.set_console_buffer(UI_USAGE_HINT.to_string());
        
        // Spawn a thread to handle the UI events 
        let mut event_handler = TerminalEventHandler::new();
        let event_thread_data = event_handler.spawn_thread();

        let mut running = true;
        while running {
            if let Some(widget_data) = &mut self.widget_data {
                terminal_manager.terminal.draw(|frame| {
                    debug!("Rendering Character Info Screen");
                    ui.render(None, UIViewMode::CharacterInfo(widget_data.clone()), frame);
                })?;

                // Whenever there's a UI event, ask the widget data to handle it
                debug!("Waiting for a Character InfoUI event");
                if let Some(e) = event_handler.receiver.recv().await {
                    debug!("Handling Character Info UI Event");
                    widget_data.handle_event(e).await;
                } else {
                    info!("Receiver returned None!");
                    running = false;
                }

                if let Some(widget_data) = &mut self.widget_data {
                    let player_position = self.level.get_player_mut().unwrap().get_global_position();

                    // If the widget data has sent us an event, handle that
                    match container_event_receiver.try_recv() {
                        Ok(event) => {
                            // Handle the event / break the loop if we need to
                            running = handle_container_event(
                                terminal_manager,
                                ui,
                                self.level,
                                player_position.clone(),
                                event,
                                &mut event_handler,
                                &mut widget_data.containers_data,
                                child_container_sender.clone()
                            ).await;
                        },
                        Err(e) => {
                            error!("Could not receive container event: {:?}", e);
                        }
                    }
                }

            }
        }

        return Ok(())
    }
    
    async fn bootstrap(&mut self) -> OpenCommandChannels {
        let ui = &mut self.ui;
        ui.set_console_buffer(UI_USAGE_HINT.to_string());
        
        // This is a special channel designed to allow widget data to send events back to this command
        // So that we can properly perform actions like closing the container display, opening a child container or taking items
        let (container_event_sender, container_event_receiver) = mpsc::unbounded_channel();
        let inventory_container = self.level.get_player_mut().unwrap().get_inventory_mut();
        let ui_areas = ui.ui_layout.as_mut().expect("Failed to get UI Layout").get_ui_areas(LayoutType::StandardSplit);
        
        if self.widget_data.is_none() {
            let mut character_info_widget_data = CharacterInfoWidgetData::new(
                inventory_container.clone(),
                ui_areas.clone(),
                container_event_sender.clone(),
            );
            
            let widget_data = build_container_widget_data(
                inventory_container.clone(),
                ui_areas.clone(),
                container_event_sender.clone()
            );
            
            character_info_widget_data.containers_data.add_container_data(
                inventory_container.get_self_item().get_id(),
                widget_data
            );
            
            self.widget_data = Some(character_info_widget_data);
        }

        let current_container_id = inventory_container.get_self_item().get_id();
        let container_widget = ContainerWidget::new(current_container_id.clone());
        let character_info_widget = CharacterInfoWidget::new(
            container_widget
        );

        // Add the container widget to the UI
        let stateful_widgets = self.ui.get_stateful_widgets_mut();
        stateful_widgets.push(StatefulWidgetType::CharacterInfo(character_info_widget));
        
        // This is the sender channel that all child containers that get opened will use
        let child_container_sender = container_event_sender.clone();
        
        self.update_usage_line();
        
        OpenCommandChannels {
            container_event_receiver,
            child_container_sender
        }
    }

    // Updates the UI usage line widget to reflect an opened container
    fn update_usage_line(&mut self) {
        let mut container_usage_commands = vec![
            UsageCommand::new('o', String::from("open") ),
            UsageCommand::new('t', String::from("take"))
        ];
        for widget in self.ui.get_additional_widgets_mut().iter_mut() {
            match widget {
                StandardWidgetType::UsageLine(usage_line_widget) => {
                    usage_line_widget.commands = container_usage_commands.clone();
                }
                _ => {}
            }
        }
    }

    fn reset_usage_line(&mut self) {
        for widget in self.ui.get_additional_widgets_mut().iter_mut() {
            match widget {
                StandardWidgetType::UsageLine(usage_line_widget) => {
                    usage_line_widget.reset_commands();
                }
                _ => {}
            }
        }
    }

}

fn build_container_widget_data(container: Container, ui_areas: UIAreas, container_event_sender: UnboundedSender<Event>) -> ContainerWidgetData {
    let main_area = ui_areas.get_area(UI_AREA_NAME_MAIN).unwrap();
    // Total area height - 4 for title, tabs row, heading row, and usage line at the bottom 
    let line_count = main_area.area.height - 4;

    let items = container.to_cloned_item_list();
    let item_list_selection =  ItemListSelection::new(items.clone(), line_count.into());
    
    let ui_area = main_area.area;
    
    ContainerWidgetData {
        container: container.clone(),
        ui_area,
        item_list_selection,
        event_sender: container_event_sender,
    }
}
    
