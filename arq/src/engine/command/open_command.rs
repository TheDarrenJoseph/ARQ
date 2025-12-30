use crate::engine::command::open_command::OpenedContainerEventType::{Close, TakeItems, TakeItemsResult};
use crate::engine::command::util::CurrentContainersData;
use crate::engine::container_util;
use crate::engine::level::Level;
use crate::error::errors::{ErrorType, ErrorWrapper};
use crate::input::{IoKeyInputResolver, KeyInputResolver, MockKeyInputResolver};
use crate::map::objects::container::Container;
use crate::map::position::Position;
use crate::terminal::terminal_manager::TerminalManager;
use crate::ui::bindings::input_bindings::KeyBindings;
use crate::ui::bindings::open_bindings::{map_open_input_to_side, OpenInput, OpenKeyBindings};
use crate::ui::event::AppEventType::OpenedContainerEvent;
use crate::ui::event::{Event, TerminalEventHandler};
use crate::ui::ui::UIViewMode::Map;
use crate::ui::ui::{UIViewMode, UI};
use crate::ui::ui_areas::UI_AREA_NAME_MAIN;
use crate::ui::ui_layout::LayoutType;
use crate::view::framehandler::container::{ContainerTarget, DropItemsRequest, DropItemsResponse, MoveItemsRequest, MoveItemsResponse, MoveItemsToContainerRequest, OpenContainerRequest, TakeItemsRequest, TakeItemsResponse};
use crate::widget::standard::usage_line::UsageCommand;
use crate::widget::stateful::container_widget::{ContainerWidget, ContainerWidgetData};
use crate::widget::{Named, StandardWidgetType, StatefulWidgetType};
use log::{debug, error, info};
use std::io;
use termion::event::Key;
use termion::event::Key::Esc;
use tokio::sync::mpsc;
use tokio::sync::mpsc::{UnboundedReceiver, UnboundedSender};
use crate::engine::command::open_command::OpenedContainerEventData::OpenContainer;

pub struct OpenCommandNew<'a, B: 'static + ratatui::backend::Backend> {
    pub level: &'a mut Level,
    pub ui: &'a mut UI,
    pub terminal_manager : &'a mut TerminalManager<B>,
    pub input_resolver: Box<dyn KeyInputResolver>,
    pub key_bindings: OpenKeyBindings,
    pub containers_data: CurrentContainersData
}

pub struct OpenCommandChannels {
    pub container_event_receiver: UnboundedReceiver<Event>, // This receives events from all the container widgets/their data handling
    pub child_container_sender: UnboundedSender<Event> // This is the sender channel that all child containers / their data that get opened will use
}

const UI_USAGE_HINT: &str = "Up/Down - Move\nEnter/q - Toggle/clear selection\nEsc - Exit";
const NOTHING_ERROR : &str = "There's nothing here to open.";

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum OpenedContainerEventType {
    // Generic Container Events
    Close,
    OpenContainer,
    // World Container Events
    TakeItems,
    TakeItemsResult,
    // Character Inventory Specific Events
    DropItems,
    DropItemsResult,
    MoveItems,
    MoveItemsResult,
    // Part of the moving items to container event chain
    MoveItemsToContainerChoice,
    MoveItemsToContainerResult,
    EquipItems,
    EquipItemsResult
}

// Specifying the request types used for a specific OpenedContainerEventType
#[derive(Debug)]
pub enum OpenedContainerEventData {
    OpenContainer(OpenContainerRequest),
    TakeItems(TakeItemsRequest),
    TakeItemsResult(TakeItemsResponse),
    DropItems(DropItemsRequest),
    DropItemsResult(DropItemsResponse),
    MoveItems(MoveItemsRequest),
    MoveItemsResult(MoveItemsResponse),
    MoveItemsToContainerChoice(MoveItemsToContainerRequest),
    ContainerChoice(ContainerTarget),
    MoveItemsToContainerChoiceResult(MoveItemsResponse),
}

impl <B: ratatui::backend::Backend> OpenCommandNew<'_, B> {

    fn re_render(&mut self) -> Result<(), io::Error>  {
        let ui = &mut self.ui;
        let level = self.level.clone();
        self.terminal_manager.terminal.draw(|frame| {
            ui.render(Some(level), Map(), frame);
        })?;
        Ok(())
    }
    
    fn get_input_resolver(&mut self) -> Box<dyn KeyInputResolver> {
        let mock_input_resolver = &mut self.input_resolver.as_any_mut().downcast_mut::<MockKeyInputResolver>();
        return if let Some(mock) = mock_input_resolver {
            Box::new(MockKeyInputResolver { key_results: mock.key_results.clone() })
        } else {
            Box::new(IoKeyInputResolver{})
        }
    }
    
    fn initial_prompt(&mut self) -> Result<Option<OpenInput>, ErrorWrapper> {
        self.ui.set_console_buffer("What do you want to open?. Arrow keys to choose. Repeat usage to choose current location.".to_string());
        self.re_render()?;
        
        let mut input_resolver = self.get_input_resolver();
        let key = input_resolver.get_input_key()?;
        Ok(self.key_bindings.get_input(key).cloned())
    }
    
    fn find_container(&mut self, position: Position) -> Option<Container> {
        if let Some(map) = &mut self.level.map {
            if let Some(c) = map.containers.get(&position) {
                let item_count = c.get_top_level_count();
                if item_count > 0 {
                    log::info!("Found map container.");

                    // Automatically open any fixed container if it's the only item in this area container
                    // For example, a single Chest in the Floor container
                    let contains_single_container = item_count == 1 && c.get_contents()[0].is_fixed_container();
                    if contains_single_container && c.get_contents()[0].get_top_level_count() > 0 {
                        return Some(c.get_contents()[0].clone());
                    } else {
                        // Otherwise, show everything in this area container
                        return Some(c.clone());
                    }
                }
            }
        }
        None
    }
    
    pub(crate) async fn begin(&mut self) -> Result<(), ErrorWrapper> {
        let input_result = self.initial_prompt();
        if input_result.is_ok() {
            
            let input_maybe = input_result.unwrap();
            let side = map_open_input_to_side(input_maybe);
            if side.is_some() {
                if let Some(p) = self.level.find_adjacent_player_position(side) {
                    log::info!("Player opening at map position: {}, {}", &p.x, &p.y);
                    self.re_render()?;
    
                    let to_open : Option<Container> = self.find_container(p);
                    if let Some(c) = to_open {
                        self.ui.clear_console_buffer();
                        self.re_render()?;
                        log::info!("Player opening container of type {:?} and length: {}", c.container_type, c.get_total_count());
                        return self.open_container(p.clone(), &c).await;
                    } else {
                        return ErrorWrapper::internal_result(NOTHING_ERROR.to_string())
                    }
                }
            }
        }
        return ErrorWrapper::internal_result(NOTHING_ERROR.to_string())
    }
    
    
    // Updates the UI usage line widget to reflect an opened container
    fn update_usage_line(&mut self) {
        let container_usage_commands = vec![
            UsageCommand::for_container_event(Key::Char('o'), String::from("open"), OpenedContainerEventType::OpenContainer),
            UsageCommand::for_container_event(Key::Char('t'), String::from("take"), TakeItems),
            UsageCommand::for_container_event(Esc, String::from("close"), Close)
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
    
    // This sets up everything needed to open containers, we set:
    // The initial event handling state (which tracks the containers being opened, and their widget data)
    // The initial container widget is sent to the UI
    // UI console and command hints are updated
    async fn open_container_bootstrapping(&mut self, c: &Container) -> OpenCommandChannels {
        self.ui.set_console_buffer(UI_USAGE_HINT.to_string());

        let frame_size = self.terminal_manager.terminal.get_frame().area();
        let mut ui_layout = self.ui.ui_layout.clone().unwrap();
        let ui_areas = ui_layout.get_or_build_areas(frame_size, LayoutType::StandardSplit);
        

        // This is a special channel designed to allow widget data to send events back to this command
        // So that we can properly perform actions like closing the container display, opening a child container or taking items
        let (container_event_sender, container_event_receiver) = mpsc::unbounded_channel();
        // This is the sender channel that all child containers that get opened will use
        let child_container_sender = container_event_sender.clone();

        let container_id = c.get_self_item().get_id();
        let main_area = ui_areas.get_area(UI_AREA_NAME_MAIN).unwrap();
        let ui_area = main_area.area;
        // Total area height - 3 for title, heading, and stat line
        let line_count = main_area.area.height - 3;
        let commands: Vec<UsageCommand> = vec![
            UsageCommand::for_container_event(Key::Char('o'), String::from("open"), OpenedContainerEventType::OpenContainer),
            UsageCommand::for_container_event(Key::Char('t'), String::from("take"), TakeItems),
            UsageCommand::for_container_event(Key::Esc, String::from("close"), Close)
        ];
        let widget_data = ContainerWidgetData::new(c.clone(), ui_area.clone(), line_count as i32, commands, container_event_sender.clone());
        
        self.containers_data.add_container_data(container_id, widget_data.clone());
        self.update_usage_line();

        let ui = &mut self.ui;

        // Add the container widget to the UI
        let container_widget = ContainerWidget::new(self.containers_data.current_container_id.unwrap());
        let stateful_widgets = ui.get_stateful_widgets_mut();
        stateful_widgets.push(StatefulWidgetType::Container(container_widget));

        OpenCommandChannels {
            container_event_receiver,
            child_container_sender
        }
    }

    // Opens the container specified by the position and container args
    async fn open_container(&mut self, position: Position, container: &Container) -> Result<(), ErrorWrapper> {
        log::info!("Player opening container: {} at position: {:?}", container.get_self_item().get_name(), position);

        // Spawn a thread to handle the UI events 
        let mut event_handler = TerminalEventHandler::new();
        let event_thread_data = event_handler.spawn_thread();

        let channels = self.open_container_bootstrapping(container).await;
        let mut container_event_receiver = channels.container_event_receiver;
        let child_container_sender = channels.child_container_sender;

        let terminal_manager = &mut self.terminal_manager;
        let ui = &mut self.ui;

        let mut running = true;
        while running {
            debug!("LOOPING");
            let current_container_id = self.containers_data.current_container_id.unwrap();
            
            let current_container_widget_data = self.containers_data.widget_data_by_id.get_mut(&current_container_id).unwrap();

            terminal_manager.terminal.draw(|frame| {
                ui.render(None, UIViewMode::Container(current_container_widget_data.clone()), frame);
            })?;

            // Whenever there's a UI event, ask the widget data to handle it
            debug!("Waiting for a UI event");
            if let Some(e) = event_handler.receiver.recv().await {
                current_container_widget_data.handle_event(e).await;
            } else {
                info!("Receiver returned None!");
                running = false;
            }
            
            // If the widget data has sent us an event, handle that
            match container_event_receiver.try_recv() {
                Ok(event) => {
                   // Handle the event / break the loop if we need to 
                   running = handle_container_event(
                       terminal_manager,
                       ui,
                       self.level,
                       position.clone(),
                       event,
                       &mut event_handler,
                       &mut self.containers_data,
                       child_container_sender.clone()
                   ).await;
                },
                Err(e) => {
                    error!("Could not receive container event: {:?}", e);
                }
            }
        }

        event_handler.receiver.close();
        event_thread_data.cancellation_token.cancel();
        event_thread_data.join_handle.await.unwrap();
        log::info!("LOOP | Open Command Event Finished");
    
        self.reset_usage_line();
        Ok(())
    }
}

async fn handle_container_event<'a, B: ratatui::backend::Backend>(
    terminal_manager: &mut TerminalManager<B>, // Necessary for building new widgets / widget data
    ui: &'a mut UI, // Necessary for building new widgets / widget data
    level: &'a mut Level, // Necessary to make actual changes to the level / world
    position: Position, // Needed for TakeItems
    event: Event,
    event_handler: &mut TerminalEventHandler, // This provides terminal IO input (key input)
    containers_data: &mut CurrentContainersData, // Tracks the currently open containers / relevant widget data
    child_container_sender: UnboundedSender<Event>,
) -> bool {
    debug!("Handling open_command container event");
    let frame_size = terminal_manager.terminal.get_frame().area();
    let mut ui_layout = ui.ui_layout.clone().unwrap();
    let ui_areas = ui_layout.get_or_build_areas(frame_size, LayoutType::StandardSplit);

    let current_container_id = containers_data.current_container_id.unwrap().clone();
    let widget_data_by_id = &mut containers_data.widget_data_by_id;
    let container_ids = &mut containers_data.container_ids;

    match event {
        Event::AppEvent(OpenedContainerEvent(OpenedContainerEventType::OpenContainer, Some(OpenContainer(open_container_request)))) => {
            let target_container = open_container_request.target;
            let target_container_id = target_container.get_self_item().get_id();
            let container_widget = ContainerWidget::new(target_container_id);
            let stateful_widgets = ui.get_stateful_widgets_mut();
            stateful_widgets.push(StatefulWidgetType::Container(container_widget));


            let main_area = ui_areas.get_area(UI_AREA_NAME_MAIN).unwrap();
            let ui_area = main_area.area;
            // Total area height - 3 for title, heading, and stat line
            let line_count = main_area.area.height - 3;
            let commands: Vec<UsageCommand> = vec![
                UsageCommand::for_container_event(Key::Char('o'), String::from("open"), OpenedContainerEventType::OpenContainer),
                UsageCommand::for_container_event(Key::Char('t'), String::from("take"), TakeItems),
                UsageCommand::for_container_event(Key::Esc, String::from("close"), Close)
            ];
            let container_widget_data = ContainerWidgetData::new(
                target_container,
                ui_area.clone(), 
                line_count as i32,
                commands,
                child_container_sender.clone()
            );
            
            widget_data_by_id.insert(target_container_id, container_widget_data);
            container_ids.push(target_container_id);
            containers_data.current_container_id = Some(target_container_id);
        }
        Event::AppEvent(OpenedContainerEvent(OpenedContainerEventType::Close, None)) => {
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
                    let current_widget_data = widget_data_by_id.get(&current_container_id).unwrap().clone();
                    let parent_container_widget_data = widget_data_by_id.get_mut(parent_container_id).unwrap();
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
        Event::AppEvent(OpenedContainerEvent(TakeItems, Some(OpenedContainerEventData::TakeItems(mut data)))) => {
            log::info!("[open usage] Received data for TakeItems with {} items", data.to_take.len());
            data.position = Some(position.clone());

            let result = container_util::player_take_items(data, level);
            match result {
                // If we have a result for this take handling, send it back via the main event handler
                // So that the container widget/data can update appropriately
                Ok(take_items_response) => {
                    ui.set_console_buffer(take_items_response.message.clone());
                    event_handler.sender.send(
                        Event::AppEvent(
                            OpenedContainerEvent(
                                TakeItemsResult,
                                Some(OpenedContainerEventData::TakeItemsResult(take_items_response))
                            )
                        )
                    ).unwrap();
                },
                Err(error_wrapper) => {
                    match error_wrapper.error_type {
                        ErrorType::DISPLAYABLE => {
                            ui.set_console_buffer(error_wrapper.displayable_message.unwrap());
                        },
                        _ => {
                            error!("Error while taking items: {:?}", error_wrapper);
                        }
                    }
                }
            }
        }
        _ => {}
    }

    // Keep running by default
    true
}

