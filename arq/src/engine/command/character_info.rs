use crate::engine::event::event::UIEventHandler;
use crate::widget::stateful::container_choice_widget::build_container_choice_widget_data;
use crate::engine::command::command::Command;
use crate::engine::command::open_command::OpenCommandChannels;
use crate::engine::command::util::CurrentContainersData;
use crate::engine::container_util;
use crate::engine::container_util::move_items;
use crate::engine::event::container::OpenedContainerEventData;
use crate::engine::event::container::OpenedContainerEventData::MoveItemsToContainerChoiceSelection;
use crate::engine::event::container::OpenedContainerEventType;
use crate::engine::event::container::TargetContainerScope;
use crate::engine::event::container::{MoveItemsRequestV2, PlayerInventoryContainer, PlayerInventoryItemPosition, SourceContainerScope, WorldContainer};
use crate::engine::event::ui::AppEventType::OpenedContainerEvent;
use crate::engine::event::ui::{TerminalEventHandler, UIEvent};
use crate::engine::level::Level;
use crate::error::errors::{ErrorType, ErrorWrapper};
use crate::item_list_selection::{ItemListSelection, ListSelection};
use crate::map::objects::container::Container;
use crate::map::objects::items::Item;
use crate::map::position::{Area, Position};
use crate::terminal::terminal_manager::TerminalManager;
use crate::ui::ui::{UIViewMode, UI};
use crate::ui::ui_areas::{UIArea, UIAreas, UI_AREA_NAME_MAIN};
use crate::ui::ui_layout::LayoutType;
use crate::view::View;
use crate::widget::standard::usage_line::UsageCommand;
use crate::widget::stateful::character_info_widget::{CharacterInfoWidget, CharacterInfoWidgetData, TabChoice};
use crate::widget::stateful::container_choice_widget::{ContainerChoice, ContainerChoiceWidget, ContainerChoiceWidgetData};
use crate::widget::stateful::container_widget::{ContainerWidget, ContainerWidgetData};
use crate::widget::{StandardWidgetType, StatefulWidgetType};
use log::{debug, error, info};
use termion::event::Key;
use tokio::sync::mpsc;
use tokio::sync::mpsc::UnboundedSender;

const UI_USAGE_HINT: &str = "Up/Down - Move, Enter/q - Toggle/clear selection\nTab - Change tab, Esc - Exit";

/*
    This command is responsible for opening and managing the Inventory/Equipment/Character Sheet "Character Info" display
 */
pub struct CharacterInfoCommand<'a, B: 'static + ratatui::backend::Backend> {
    pub level: &'a mut Level,
    pub ui: &'a mut UI,
    pub terminal_manager : &'a mut TerminalManager<B>,
    pub widget_data: Option<CharacterInfoWidgetData>,
    // The commands available for the underlying container widgets
    pub container_widget_commands: Option<Vec<UsageCommand>>
}

impl <B: ratatui::backend::Backend> Command<()> for CharacterInfoCommand<'_, B> {
    async fn start(&mut self) -> Result<(), ErrorWrapper> {
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
                debug!("Waiting for a Character Info UI event");
                if let Some(e) = event_handler.receiver.recv().await {

                    // If we have a container choice data set, it takes priority
                    // Handle any events specific to choosing a container
                    let choice_event = e.clone();
                    if let Some(choice_data) = &mut widget_data.current_containers_data.container_choice_data {
                        debug!("Handling Character Info UI Event (container choice)");
                        choice_data.handle_event(choice_event).await;

                    } else {
                        debug!("Handling Character Info UI Event (container)");
                        widget_data.handle_event(e).await;
                    }
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
                                &mut widget_data.tab_choice,
                                &mut widget_data.current_containers_data,
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

        log::info!("Closing event handling / threading");
        // Make sure to close the event receiver
        event_handler.receiver.close();
        // Make sure to close out the thread
        event_thread_data.cancellation_token.cancel();
        event_thread_data.join_handle.await.unwrap();
        return Ok(())
    }
}

async fn handle_container_event<'a, B: ratatui::backend::Backend>(
    terminal_manager: &mut TerminalManager<B>, // Necessary for building new widgets / widget data
    ui: &'a mut UI, // Necessary for building new widgets / widget data
    level: &'a mut Level, // Necessary to make actual changes to the level / world
    player_position: Position, // Needed for DropItems
    event: UIEvent,
    event_handler: &mut TerminalEventHandler, // This provides terminal IO input (key input)
    tab_choice: &mut TabChoice,
    containers_data: &mut CurrentContainersData, // Tracks the currently open containers / relevant widget data
    child_container_sender: UnboundedSender<UIEvent>,
) -> bool {
    debug!("Handling character_info container event");
    let frame_size = terminal_manager.terminal.get_frame().area();
    let mut ui_layout = ui.ui_layout.clone().unwrap();
    let ui_areas = ui_layout.get_or_build_areas(frame_size, LayoutType::StandardSplit);

    let current_container_id = containers_data.current_container_id.unwrap().clone();
    let widget_data_by_id = &mut containers_data.widget_data_by_id;
    let container_ids = &mut containers_data.container_ids;

    let container_choice_data = &mut containers_data.container_choice_data;

    // If we have a container choice data set, it takes priority
    // Handle any events specific to choosing a container
    if let Some(choice_data) = container_choice_data {
        match event {
            // Clear the data if we're exiting the widget
            UIEvent::AppEvent(OpenedContainerEvent(OpenedContainerEventType::Close, None)) => {
                log::info!("Handling Close event");
                containers_data.container_choice_data = None;
            },
            // If we've picked a selection, we should also close the widget
            UIEvent::AppEvent(OpenedContainerEvent(OpenedContainerEventType::MoveToContainerChoiceSelection, Some(MoveItemsToContainerChoiceSelection(target)))) => {
                log::info!("Handling MoveItemsToContainerChoiceSelection event");

                // Grab all the items selected in the current container widget (before the container choice was presented)
                let current_container_widget_data : ContainerWidgetData = widget_data_by_id.get(&current_container_id).unwrap().clone();

                let source_container = current_container_widget_data.container.clone();
                let items_selected = current_container_widget_data.item_list_selection.get_selected_items();

                let _player_inventory =level.get_player_mut().unwrap().get_inventory();

                let mut to_move: Vec<Item>  = Vec::new();
                items_selected.iter().for_each(|item|to_move.push(item.clone()));

                let data = MoveItemsRequestV2 {
                    source: SourceContainerScope::PlayerInventory(PlayerInventoryContainer { container: source_container.clone() }),
                    target: target,
                    to_move: to_move
                };

                let result = move_items(data, level);
                match result {
                    Ok(response) => {
                        ui.set_console_buffer(response.message.clone());
                        event_handler.sender.send(
                            UIEvent::AppEvent(
                                OpenedContainerEvent(
                                    OpenedContainerEventType::MoveToContainerChoiceResult,
                                    Some(OpenedContainerEventData::MoveItemsToContainerChoiceResult(response))
                                )
                            )
                        ).unwrap();
                    }
                    Err(error_wrapper) => {
                        match error_wrapper.error_type {
                            ErrorType::DISPLAYABLE => {
                                ui.set_console_buffer(error_wrapper.displayable_message.unwrap());
                            },
                            _ => {
                                error!("Error while moving items to another container: {:?}", error_wrapper);
                            }
                        }
                    }
                }
                containers_data.container_choice_data = None;
            },
            _ => {
                // Otherwise, pass anything that might be of value to the widget data
                choice_data.handle_event(event).await;
            }
        }
    } else {
        // Handle any other events for within a container
        match event {
            // TODO can this be refactored to be shared between this and open_command?
            UIEvent::AppEvent(
                OpenedContainerEvent(
                    OpenedContainerEventType::Close,
                    None
                )
            ) => {
                // If there's no container choice window, we're trying to close the container window
                let closing_container_id = current_container_id.clone();
                if widget_data_by_id.len() > 1 {
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
                        // For example
                        // if we had opened a Bag in the parent container
                        // When closing the widget, we need to update the parent's version of the Bag
                        // to match any changes made within it
                        let current_widget_data = widget_data_by_id.get(&current_container_id).unwrap().clone();
                        let parent_container_widget_data = widget_data_by_id.get_mut(parent_container_id).unwrap();
                        // Find the container belonging to the widget being closed
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
                } else if widget_data_by_id.len() == 1 {
                    // If only one container is open, stop the command
                    return false;
                }
            },
            UIEvent::AppEvent(
                OpenedContainerEvent(
                                  OpenedContainerEventType::OpenContainer,
                                  Some(OpenedContainerEventData::OpenContainer(open_container_request)
                                  )
                )
            ) => {
                let target_container = open_container_request.target;
                let target_container_id = target_container.get_self_item().get_id();
                let container_widget = ContainerWidget::new(target_container_id);
                ui.add_stateful_widget(StatefulWidgetType::Container(container_widget));
                
                let container_widget_data = build_container_widget_data(
                    target_container.clone(),
                    ui_areas.clone(),
                    child_container_sender.clone()
                );

                widget_data_by_id.insert(target_container_id, container_widget_data);
                container_ids.push(target_container_id);
                containers_data.current_container_id = Some(target_container_id);
            },
            UIEvent::AppEvent(OpenedContainerEvent(OpenedContainerEventType::DropItems, Some(OpenedContainerEventData::DropItems(mut data)))) => {
                log::info!("[open usage] Received data for DropItems with {} items", data.to_drop.len());
                data.position = Some(player_position.clone());

                let result = container_util::player_drop_items(data, level);
                match result {
                    // If we have a result for this drop handling, send it back via the main event handler
                    // So that the container widget/data can update appropriately
                    Ok(response) => {
                        ui.set_console_buffer(response.message.clone());
                        event_handler.sender.send(
                            UIEvent::AppEvent(
                                OpenedContainerEvent(
                                    OpenedContainerEventType::DropItemsResult,
                                    Some(OpenedContainerEventData::DropItemsResult(response))
                                )
                            )
                        ).unwrap();
                    }
                    Err(error_wrapper) => {
                        match error_wrapper.error_type {
                            ErrorType::DISPLAYABLE => {
                                ui.set_console_buffer(error_wrapper.displayable_message.unwrap());
                            },
                            _ => {
                                error!("Error while dropping items: {:?}", error_wrapper);
                            }
                        }
                    }
                }
            },
            UIEvent::AppEvent(OpenedContainerEvent(OpenedContainerEventType::MoveItems, Some(OpenedContainerEventData::MoveItemsWithinSource(data)))) => {
                // We're either moving items into a container inside the inventory, or a specific item position
                let data = if let Some(target_container) = data.target_container  {
                    MoveItemsRequestV2 {
                        source: SourceContainerScope::PlayerInventory(PlayerInventoryContainer { container: data.source_container.clone() }),
                        target: TargetContainerScope::PlayerInventory(PlayerInventoryContainer { container: target_container.clone() }),
                        to_move: data.to_move.clone()
                    }
                } else {
                    let target_position_item = data.target_position_item.unwrap();
                    MoveItemsRequestV2 {
                        source: SourceContainerScope::PlayerInventory(PlayerInventoryContainer { container: data.source_container.clone() }),
                        target: TargetContainerScope::PlayerInventoryItemPosition(PlayerInventoryItemPosition { target_position_item: target_position_item.clone() }),
                        to_move: data.to_move.clone()
                    }
                };

                let result = container_util::move_items(data, level);
                match result {
                    Ok(response) => {
                        ui.set_console_buffer(response.message.clone());
                        event_handler.sender.send(
                            UIEvent::AppEvent(
                                OpenedContainerEvent(
                                    OpenedContainerEventType::MoveItemsResult,
                                    Some(OpenedContainerEventData::MoveItemsWithSourceResult(response))
                                )
                            )
                        ).unwrap();
                    }
                    Err(error_wrapper) => {
                        match error_wrapper.error_type {
                            ErrorType::DISPLAYABLE => {
                                ui.set_console_buffer(error_wrapper.displayable_message.unwrap());
                            },
                            _ => {
                                error!("Error while moving items: {:?}", error_wrapper);
                            }
                        }
                    }
                }
            },
            UIEvent::AppEvent(
                OpenedContainerEvent(
                    OpenedContainerEventType::ShowMoveContainerChoices,
                    Some(OpenedContainerEventData::MoveItemsToContainerChoice(data))
                )
            ) => {
                let container_choice_widget = ContainerChoiceWidget::new();
                // Register the widget with the UI
                ui.add_stateful_widget(StatefulWidgetType::ContainerChoice(container_choice_widget));

                let player_mut = level.characters.get_player_mut();

                if let Some(player) = player_mut {
                    let inventory = player.get_inventory();

                    let container_choices =
                        container_util::build_container_choices(
                            &data.source,
                            TargetContainerScope::PlayerInventory(
                                PlayerInventoryContainer {
                                    container: inventory.clone()
                                }
                            ),
                            player_position
                        );
                    let mut choices = container_choices.unwrap();

                    let player_pos = player.get_global_position();

                    // Check the player's current position and neighbors
                    // for any containers we might want to move items to
                    if let Some(map) = &level.map {
                        let level_containers = &map.containers;
                        for nearby_pos in player_pos.get_neighbors() {
                            if let Some(ncc) = level_containers.get(&nearby_pos) {
                                let neighbor_container_name = ncc.get_self_item().get_name().clone();

                                choices
                                    .push(
                                        ContainerChoice {
                                            container_scope: TargetContainerScope::WorldContainer(
                                                WorldContainer {
                                                    container: ncc.clone(),
                                                    position: nearby_pos.clone(),
                                                }
                                            ),
                                            location_name: format!("{} ({})", neighbor_container_name, player_pos.describe_neighbor(nearby_pos)),
                                        }
                                    );

                                let child_containers : Vec<Container> = if ncc.is_true_container() {
                                    ncc.get_contents().iter().filter(|c| c.is_true_container()).cloned().collect()
                                } else {
                                    Vec::new()
                                };
                                for child_container in child_containers {
                                    let child_container_name = ncc.get_self_item().get_name().clone();
                                    choices
                                        .push(
                                            ContainerChoice {
                                                container_scope: TargetContainerScope::WorldContainer(
                                                    WorldContainer {
                                                        container: child_container.clone(),
                                                        position: nearby_pos.clone(),
                                                    }
                                                ),
                                                location_name: format!("{} ({})", child_container_name, neighbor_container_name),
                                            }
                                        )
                                }
                            }
                        }
                    }

                    let container_choice_data = build_container_choice_widget_data(
                        choices,
                        ui_areas.clone(),
                        child_container_sender
                    );
                    containers_data.container_choice_data = Some(container_choice_data);
                }


            },
            _ => {}
        }
    }

    // Keep running by default
    true
}

impl<B: ratatui::backend::Backend> CharacterInfoCommand<'_, B> {

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

            let container_widget_data = build_container_widget_data(
                inventory_container.clone(),
                ui_areas.clone(),
                container_event_sender.clone()
            );
            // Take a copy of the commands available for the child container widget so we can display them
            self.container_widget_commands = Some(container_widget_data.usage_commands.clone());

            character_info_widget_data.current_containers_data.add_container_data(
                inventory_container.get_self_item().get_id(),
                container_widget_data
            );

            self.widget_data = Some(character_info_widget_data);
        }

        let inventory_container_id = inventory_container.get_self_item().get_id();
        let character_info_widget = CharacterInfoWidget::new(
            inventory_container_id.clone()
        );

        // Add the character info widget to the UI
        ui.add_stateful_widget(StatefulWidgetType::CharacterInfo(character_info_widget));

        // This is the sender channel that all child containers that get opened will use
        let child_container_sender = container_event_sender.clone();

        // Updates the UI usage line widget to reflect an opened inventory container
        self.update_usage_line();

        OpenCommandChannels {
            container_event_receiver,
            child_container_sender
        }
    }

    // Updates the UI usage line widget to reflect an opened container
    fn update_usage_line(&mut self) {
        let container_usage_commands = self.container_widget_commands.clone().unwrap();
        for widget in self.ui.get_standard_widgets_mut().iter_mut() {
            match widget {
                StandardWidgetType::UsageLine(usage_line_widget) => {
                    usage_line_widget.commands = container_usage_commands.clone();
                }
                _ => {}
            }
        }
    }

    fn reset_usage_line(&mut self) {
        for widget in self.ui.get_standard_widgets_mut().iter_mut() {
            match widget {
                StandardWidgetType::UsageLine(usage_line_widget) => {
                    usage_line_widget.reset_commands();
                }
                _ => {}
            }
        }
    }

}

// Builds a widget to open a sub container within the Character Info view
fn build_container_widget_data(container: Container, ui_areas: UIAreas, container_event_sender: UnboundedSender<UIEvent>) -> ContainerWidgetData {
    let main_area = ui_areas.get_area(UI_AREA_NAME_MAIN).unwrap();
    let widget_ui_area = crate::widget::stateful::container_choice_widget::build_container_widget_area(&main_area);

    // -3 to account for:
    // 1. Title / Border top
    // 2. Table headings
    // 3. Border bottom
    let line_count = widget_ui_area.height - 3;
    let items = container.to_cloned_item_list();
    let item_list_selection =  ItemListSelection::new(items.clone(), line_count.into());

    // These are the usage commands available specifically to the character info command relating to the container widget
    let commands: Vec<UsageCommand> = vec![
        UsageCommand::for_container_event(Key::Char('o'), String::from("open"), OpenedContainerEventType::OpenContainer),
        UsageCommand::for_container_event(Key::Char('d'), String::from("drop"), OpenedContainerEventType::DropItems),
        UsageCommand::for_container_event(Key::Char('m'), String::from("move"), OpenedContainerEventType::MoveItems),
        UsageCommand::for_container_event(Key::Char('c'), String::from("move-to-container"), OpenedContainerEventType::ShowMoveContainerChoices),
        UsageCommand::for_container_event(Key::Char('e'), String::from("equip"), OpenedContainerEventType::EquipItems),
        UsageCommand::for_container_event(Key::Esc, String::from("close"), OpenedContainerEventType::Close),
    ];

    ContainerWidgetData {
        container: container.clone(),
        ui_area: widget_ui_area,
        item_list_selection,
        usage_commands: commands,
        event_sender: container_event_sender
    }
}
