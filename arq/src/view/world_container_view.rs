use crate::error::errors::ErrorWrapper;
use crate::input::KeyInputResolver;
use termion::event::Key;
use ratatui::CompletedFrame;

use crate::map::objects::container::Container;
use crate::map::position::{Area, Position};
use crate::terminal::terminal_manager::TerminalManager;
use crate::ui::ui::UI;
use crate::ui::ui_areas::{UIAreas, UI_AREA_NAME_MAIN};
use crate::ui::ui_layout::LayoutType;
use crate::view::framehandler::container::{ContainerFrameHandler, ContainerFrameHandlerInputResult, TakeItemsRequest};
use crate::view::framehandler::container_choice::{ContainerChoiceFrameHandler, ContainerChoiceFrameHandlerInputResult};
use crate::view::framehandler::{FrameData, FrameHandler};
use crate::view::util::callback::Callback;
use crate::view::{InputHandler};
use crate::view::{GenericInputResult, InputResult, View};
use crate::view::util::try_build_container_choice_frame_handler;
/*
    This View is responsible for displaying/interacting with containers in the world (i.e chests, dropped items, dead bodies)
 */
pub struct WorldContainerView<'a, B : ratatui::backend::Backend> {
    pub ui : &'a mut UI,
    pub terminal_manager : &'a mut TerminalManager<B>,
    pub frame_handlers: WorldContainerViewFrameHandlers,
    pub container : Container,
    pub callback : Box<dyn FnMut(ContainerFrameHandlerInputResult) -> Option<ContainerFrameHandlerInputResult> + 'a>,
    pub input_resolver: Box<dyn KeyInputResolver>,
    // The sender and receiver are used for communicating with the command in control
    // Moving, dropping, opening containers etc
    pub result_sender: tokio::sync::mpsc::Sender<ContainerFrameHandlerInputResult>,
    pub result_response_receiver: tokio::sync::mpsc::Receiver<ContainerFrameHandlerInputResult>
}

pub struct WorldContainerViewFrameData {
}

pub struct WorldContainerViewFrameHandlers {
    pub container_frame_handlers: Vec<ContainerFrameHandler>,
    pub choice_frame_handler: Option<ContainerChoiceFrameHandler>
}

impl <B : ratatui::backend::Backend> WorldContainerView<'_, B> {

    /*
    * TODO (DUPLICATION) make choice and generic input flows generic / shared between views i.e world_container and character_info?
    * Handles passthrough to the relevant container choice view (if one is present)
    * Triggering callbacks if needed
    * Returns the optional input result of the container choice view, and a boolean to indicate success
    */
    fn handle_container_choice_input(&mut self, key: Key) -> Result<(Option<GenericInputResult>, bool), ErrorWrapper> {
        if let Some(cfh) = &mut self.frame_handlers.choice_frame_handler {
            let result = cfh.handle_input(Some(key))?;
            if let Some(view_specific_result) = result.view_specific_result {
                match view_specific_result {
                    ContainerChoiceFrameHandlerInputResult::Select(selected_container) => {
                        let container_views = &mut self.frame_handlers.container_frame_handlers;
                        if let Some(topmost_view) = container_views.last_mut() {
                            // Extract the data on from the view
                            let view_specific_result = topmost_view.build_move_items_result().unwrap().view_specific_result.unwrap();
                            match view_specific_result {
                                ContainerFrameHandlerInputResult::MoveToContainerChoice(mut data) => {
                                    // Add the target selected to the callback data
                                    data.target_container = Some(selected_container.clone());
                                    self.trigger_callback(ContainerFrameHandlerInputResult::MoveToContainerChoice(data));
                                    // Clear the frame handler now we're done
                                    self.frame_handlers.choice_frame_handler = None;
                                    return Ok((None, true));
                                },
                                _ => {}
                            }
                        }
                    },
                    _ => {}
                }
            }
            return Ok((Some(result.generic_input_result), true))
        }
        Ok((None, false))
    }

    async fn send_message(&mut self, data: ContainerFrameHandlerInputResult) {
        self.result_sender.send(data).await.expect("Failed to send ContainerFrameHandlerInputResult");
        let response = self.result_response_receiver.recv().await.expect("Failed to receive response ContainerFrameHandlerInputResult");
        self.handle_callback_result(Some(response));
    }
}

impl <B: ratatui::backend::Backend> View<bool> for WorldContainerView<'_, B>  {
    fn begin(&mut self) -> Result<InputResult<bool>, ErrorWrapper> {
        self.terminal_manager.terminal.clear()?;
        self.draw(None)?;
        
        while !self.handle_input(None).unwrap().generic_input_result.done {
            self.draw(None)?;
        }
        return Ok(InputResult { generic_input_result: GenericInputResult { done: true, requires_view_refresh: true }, view_specific_result: None});
    }

    fn draw(&mut self, _area: Option<Area>) -> Result<CompletedFrame, ErrorWrapper> {
        let frame_handler = &mut self.frame_handlers;
        let ui = &mut self.ui;

        let ui_layout = ui.ui_layout.as_mut().unwrap();
        let frame_size = self.terminal_manager.terminal.get_frame().size();
        let ui_areas: UIAreas = ui_layout.get_or_build_areas(frame_size, LayoutType::StandardSplit).clone();

        if let Some(main) = ui_areas.get_area(UI_AREA_NAME_MAIN) {
            let main_area = main.area;
            return Ok(self.terminal_manager.terminal.draw(|frame| {
                ui.render(None, None, frame);
                let frame_area = Area::new(
                    Position::new(main_area.start_position.x + 1, main_area.start_position.y + 1),
                    main_area.width.clone() - 1,
                    main_area.height.clone() - 2
                );
                let specific_frame_data = WorldContainerViewFrameData {};
                frame_handler.handle_frame(frame, FrameData { frame_area: frame_area, data: specific_frame_data, ui_areas: ui_areas.clone() });
            })?);
        }
        ErrorWrapper::internal_result(String::from("Failed to draw world container view"))
    }
}

impl <COM: ratatui::backend::Backend> InputHandler<bool> for WorldContainerView<'_, COM> {
    fn handle_input(&mut self, input: Option<Key>) -> Result<InputResult<bool>, ErrorWrapper> {
        let key = self.input_resolver.get_or_return_input_key(input)?;
        match key {
            Key::Char('t') => {
                if let Some(parent_view) = self.frame_handlers.container_frame_handlers.last_mut() {
                    let selected_container_items = parent_view.get_selected_items();
                    let data = TakeItemsRequest { source: self.container.clone(), to_take: selected_container_items, position: None };
                    let result = ContainerFrameHandlerInputResult::TakeItems(data);

                    // TODO for now we do both messaging and callback
                    let _ = self.send_message(result.clone());
                    self.trigger_callback(result);
                }
            },
            Key::Esc => {
                // Drop the last container view and keep going
                let container_views = &self.frame_handlers.container_frame_handlers;
                if container_views.len() > 1 {
                    if let Some(closing_view) = self.frame_handlers.container_frame_handlers.pop() {
                        let closing_container = closing_view.container;
                        if let Some(parent_view) = self.frame_handlers.container_frame_handlers.last_mut() {
                            let parent_container = &mut parent_view.container;
                            if let Some(position) = parent_container.position(&closing_container) {
                                parent_container.replace(position, closing_container);
                            }
                        }
                    }
                    return Ok(InputResult { generic_input_result: GenericInputResult { done: false, requires_view_refresh: false }, view_specific_result: None});
                } else if container_views.len() == 1 {
                    let last_view = &mut self.frame_handlers.container_frame_handlers[0];
                    self.container = last_view.container.clone();
                }
                return Ok(InputResult { generic_input_result: GenericInputResult { done: true, requires_view_refresh: true }, view_specific_result: None});
            },
            // Passthrough anything not handled here into the sub framehandler
            _ => {
                let mut generic_input_result: Option<GenericInputResult> = None;

                // TODO (DUPLICATE) make generic between world/inventory view?
                // Container choice view takes precedence as it's basically a dialog
                // Container choice handlers take priority as they're essentially a dialog
                if let Ok((gir_result, success)) = self.handle_container_choice_input(key) {
                    // Rewrap this only if something was returned
                    if let Some(gir) = gir_result {
                        generic_input_result = Some(gir);
                    }
                    // Force a redraw
                    if success {
                        return Ok(InputResult { generic_input_result: GenericInputResult { done: false, requires_view_refresh: true }, view_specific_result: None});
                    }
                }

                let container_views = &mut self.frame_handlers.container_frame_handlers;
                let have_container_views = !container_views.is_empty();
                if have_container_views {
                    if let Some(topmost_view) = container_views.last_mut() {
                        let view_input_result = topmost_view.handle_input(Some(key));
                        let result = view_input_result.unwrap();
                        if let Some(ContainerFrameHandlerInputResult::OpenContainerView(stacked_view)) = result.view_specific_result {
                            container_views.push(stacked_view);
                        } else if let Some(r) = result.view_specific_result {
                            self.trigger_callback(r);
                        }
                        generic_input_result = Some(result.generic_input_result);
                    }
                }

                if let Some(r) = generic_input_result {
                    if r.requires_view_refresh {
                        self.terminal_manager.terminal.clear()?;
                    }
                }
            }
        }

        return Ok(InputResult { generic_input_result: GenericInputResult { done: false, requires_view_refresh: false }, view_specific_result: None});
    }
}

impl <'c, B : ratatui::backend::Backend> Callback<'c, ContainerFrameHandlerInputResult> for WorldContainerView<'c, B> {
    fn set_callback(&mut self, callback: Box<impl FnMut(ContainerFrameHandlerInputResult) -> Option<ContainerFrameHandlerInputResult> + 'c>) {
        self.callback = callback;
    }

    fn trigger_callback(&mut self, data: ContainerFrameHandlerInputResult) {
        let result = (self.callback)(data);
        self.handle_callback_result(result);
    }

    fn handle_callback_result(&mut self, result: Option<ContainerFrameHandlerInputResult>) {
        if let Some(r) = result {
            match r {
                ContainerFrameHandlerInputResult::MoveToContainerChoice(ref data) => {
                    let result = try_build_container_choice_frame_handler(data);
                    if result.is_ok() {
                        self.frame_handlers.choice_frame_handler = result.ok()
                    } else {
                        let error = result.err().unwrap();
                        self.ui.set_console_buffer(error.internal_message.unwrap())
                    }
                },
                ContainerFrameHandlerInputResult::MoveItems(ref data) => {
                    // TODO (Duplicate) make generic between world_contaienr / character_info
                    if data.target_container.is_some() {
                        // if target_container is the root view container
                        let root_container = self.frame_handlers.container_frame_handlers.first().map(|top_cv| top_cv.container.clone()).unwrap();
                        let target_is_root = data.target_container.as_ref().map_or_else(|| false, |t| t.id_equals(&root_container));
                        let target_in_source = data.target_container.as_ref().map_or_else(|| false, |c| data.source.find(c.get_self_item()).is_some());
                        // Target is the topmost container, so we're forced to rebuild everything
                        if target_is_root {
                            // Drain all after the first
                            self.frame_handlers.container_frame_handlers.drain(1..);
                            // Then rebuild the remaining (root container) view
                            if let Some(topmost_view) = self.frame_handlers.container_frame_handlers.last_mut() {
                                topmost_view.rebuild_to_container(data.target_container.as_ref().unwrap().clone())
                            }
                        } else {
                            // i.e moving to a parent container that isn't the root
                            if !target_in_source {
                                // Rebuild that specific container view
                                if let Some(cv) = self.frame_handlers.container_frame_handlers.iter_mut()
                                    .find(|cv| cv.container.id_equals(&data.target_container.as_ref().unwrap())) {
                                    cv.rebuild_to_container(data.target_container.as_ref().unwrap().clone())
                                }
                            }

                            // Ensure the current view updates
                            if let Some(topmost_view) = self.frame_handlers.container_frame_handlers.last_mut() {
                                topmost_view.handle_callback_result(r);
                            }
                        }
                    } else {
                        for fh in &mut self.frame_handlers.container_frame_handlers {
                            if fh.container.id_equals(&data.source) {
                                fh.handle_callback_result(r.clone())
                            }
                        }
                    }
                },
                _ => {
                    self.frame_handlers.container_frame_handlers.last_mut().map(|fh| fh.handle_callback_result(r));

                }
            }
        }
    }
}

impl FrameHandler<WorldContainerViewFrameData> for WorldContainerViewFrameHandlers {
    fn handle_frame(&mut self, frame: &mut ratatui::Frame, data: FrameData<WorldContainerViewFrameData>) {
        let main_area = data.ui_areas.get_area(UI_AREA_NAME_MAIN).unwrap().area;
        if let Some(cfh) = &mut self.choice_frame_handler {
            let inventory_area = Area::new(
                Position::new(main_area.start_position.x + 2, main_area.start_position.y + 2),
                main_area.width / 2,
                main_area.height / 2
            );
            let frame_data = FrameData { data: Vec::new(), frame_area: inventory_area, ui_areas: data.ui_areas };
            cfh.handle_frame(frame, frame_data);
        } else if let Some(topmost_view) = self.container_frame_handlers.last_mut() {
            let mut frame_inventory = topmost_view.container.clone();
            topmost_view.handle_frame(frame, FrameData { data: &mut frame_inventory, frame_area: data.frame_area, ui_areas: data.ui_areas });
        }
    }
}

#[cfg(test)]
mod tests {
    use std::collections::VecDeque;

    use crate::input::{IoKeyInputResolver, MockKeyInputResolver};
    use termion::event::Key;
    
    use ratatui::layout::Rect;
    use ratatui::prelude::{Modifier, Style};
    use ratatui::style::Color::{Black, Green, Reset};
    
    use crate::map::map_generator::build_dev_chest;
    use crate::map::position::Area;
    use crate::terminal;
    use crate::terminal::terminal_manager::TerminalManager;
    use crate::test::utils::test_resource_loader::read_expected_buffer_file;
    use crate::ui::resolution::Resolution;
    use crate::ui::ui::{build_ui, UI};
    use crate::ui::ui_areas::UIAreas;
    use crate::ui::ui_layout::{LayoutType, UILayout};
    use crate::view::framehandler::container;
    use crate::widget::standard::usage_line::{UsageCommand, UsageLineWidget};
    use crate::view::world_container_view::{WorldContainerView, WorldContainerViewFrameHandlers};
    use crate::view::{View, MIN_RESOLUTION};
    
    fn build_test_minimal_ui() -> UI {
        let mut ui = build_ui();
        let resolution = Resolution::new(MIN_RESOLUTION.width, MIN_RESOLUTION.height);
        let ui_layout = UILayout::new(resolution);
        ui.ui_layout = Some(ui_layout);
        ui
    }
    
    fn build_view<'a, B: ratatui::backend::Backend>(ui: &'a mut UI, terminal_manager: &'a mut TerminalManager<B>) -> WorldContainerView<'a, B> {
        let container = build_dev_chest();
        let subview_container = container.clone();
        let view_container = container.clone();
        
        let commands : Vec<UsageCommand> = vec![
            UsageCommand::new('o', String::from("open") ),
            UsageCommand::new('t', String::from("take"))
        ];
        let usage_line = UsageLineWidget::for_commands(commands);
        let container_view = container::build_container_frame_handler(subview_container, usage_line);
        
        // AND we've created a WorldContainerView to view a dev testing Chest container
        let frame_handlers = WorldContainerViewFrameHandlers { container_frame_handlers: vec![container_view], choice_frame_handler: None };
        
        let result_channel =  tokio::sync::mpsc::channel(2);
        let world_container_view = WorldContainerView {
            ui,
            terminal_manager,
            frame_handlers,
            container: view_container,
            callback: Box::new(|_data| {None}),
            input_resolver: Box::new(IoKeyInputResolver {}),
            result_sender: result_channel.0,
            result_response_receiver: result_channel.1
        };
        return world_container_view
    }

    #[test]
    fn test_draw() {
        // GIVEN a UI and terminal manager representing a 80x24 (MIN_RESOLUTION) screen
        let mut ui = build_test_minimal_ui();
        let mut terminal_manager = terminal::terminal_manager::init_test(MIN_RESOLUTION).unwrap();
        let mut world_container_view = build_view(&mut ui, &mut terminal_manager);

        // WHEN we call to draw the world container view, it should complete successfully
        world_container_view.draw(None).expect("World container view should have been drawn");
    }

    #[test]
    fn test_begin_selecting_items() {
        // GIVEN a UI and terminal manager representing a 80x24 (MIN_RESOLUTION) screen
        let mut ui = build_test_minimal_ui();
        let mut terminal_manager = terminal::terminal_manager::init_test(MIN_RESOLUTION).unwrap();
        let mut world_container_view = build_view(&mut ui, &mut terminal_manager);
        
        let key_results: VecDeque<Key> = VecDeque::from([
            Key::Down, // Move down past the Bag to Item 1
            crate::global_flags::ENTER_KEY, // Start selection
            Key::Down, // Selecting Test Item 1, 2, and 3
            Key::Down,
            Key::Esc // Quit the view
        ]);
        // AND we've mocked out input to select the first 3 Test Item x's in the list (skipped past the first item, the bag)
        let input_resolver = MockKeyInputResolver { key_results };
        
        world_container_view.input_resolver = Box::new(input_resolver);
        
        // WHEN we call to begin the view draw / IO loop
        let result = world_container_view.begin().unwrap();
        
        // THEN we expect to be returned here after selecting items
        // AND as the view will not have redrawn yet, we should be able to see this in the buffer
        let frame_area = world_container_view.terminal_manager.terminal.get_frame().area();
        let mut ui_layout = world_container_view.ui.ui_layout.clone().unwrap();
        let ui_areas: UIAreas = ui_layout.get_or_build_areas(frame_area, LayoutType::StandardSplit).clone();
        
        let mut expected_buffer= read_expected_buffer_file(String::from("resources/test/world_container_selection.txt"), Area::from_rect(frame_area));

        let reset_style = Style::default()
            .fg(Reset)
            .bg(Reset)
            .underline_color(Reset);
        
        let background_style = Style::default()
            .fg(Reset)
            .bg(Black);

        let selected_style = Style::default()
            .fg(Green)
            .bg(Black);
        
        let highlighted_style = Style::default()
            .fg(Green)
            .bg(Black)
            .add_modifier(Modifier::REVERSED);
        
        expected_buffer.set_style(Rect::new(0,0,frame_area.width, frame_area.height), background_style);
        
        // Row 2 - For Test Item 1 (second row)
        // AND the first 2 rows of the selection should be green text on black background, with no reverse (selected_style)
        // x == 2 (to avoid the 2 left hand borders)
        // y == 4 (to avoid the 2 top borders, and the headers row) + 1 to skip the "Bag" container row
        expected_buffer.set_style(Rect::new(2,4,54, 1), selected_style);
        
        // Row 3 - For Test Item 2
        // x == 2 (to avoid the 2 left hand borders)
        // y == 5 (to avoid the 2 top borders, the headers row, and 2 previous lines)
        expected_buffer.set_style(Rect::new(2,5,54, 1), selected_style);

        // Row 4 - For Test Item 3
        // x == 2 (to avoid the 2 left hand borders)
        // y == 6 (to avoid the 2 top borders, the headers row, and 3 previous lines)
        // This line is actually the reversed / highlighted line as it's both selected, and the current selection index
        expected_buffer.set_style(Rect::new(2,6, 54, 1), highlighted_style);

        // And the last few lines should be blank/reset style
        expected_buffer.set_style(Rect::new(0,19, 80, 1), reset_style);
        expected_buffer.set_style(Rect::new(0,20, 80, 1), reset_style);
        expected_buffer.set_style(Rect::new(0,21, 80, 1), reset_style);
        expected_buffer.set_style(Rect::new(0,22, 80, 1), reset_style);
        expected_buffer.set_style(Rect::new(0,23, 80, 1), reset_style);
        
        world_container_view.terminal_manager.terminal.backend().assert_buffer(&expected_buffer);
    }
}