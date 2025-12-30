use std::io::Error;

use ratatui::layout::Rect;
use ratatui::style::{Modifier, Style};
use ratatui::symbols::line::VERTICAL;
use ratatui::text::Line;
use ratatui::widgets::{Block, Borders, Tabs};
use ratatui::CompletedFrame;
use termion::event::Key;

use crate::character::Character;
use crate::engine::command::open_command::OpenedContainerEventType;
use crate::error::errors::ErrorWrapper;
use crate::map::position::{Area, Position};
use crate::terminal::terminal_manager::TerminalManager;
use crate::ui::ui::UIViewMode::Map;
use crate::ui::ui::UI;
use crate::ui::ui_areas::{UIAreas, UI_AREA_NAME_MAIN};
use crate::ui::ui_layout::LayoutType;
use crate::view::framehandler::character_equipment::CharacterEquipmentFrameHandler;
use crate::view::framehandler::character_info::CharacterInfoFrameHandler;
use crate::view::framehandler::character_stats::{CharacterStatsFrameHandler, ViewMode};
use crate::view::framehandler::container::ContainerFrameHandlerInputResult;
use crate::view::framehandler::container_choice::ContainerChoiceFrameHandlerInputResult;
use crate::view::framehandler::{container, FrameData, FrameHandler};
use crate::view::util::callback::Callback;
use crate::view::util::try_build_container_choice_frame_handler;
use crate::view::InputHandler;
use crate::view::{resolve_input, verify_display_size, GenericInputResult, InputResult, View};
use crate::widget::standard::usage_line::{UsageCommand, UsageLineWidget};
use crate::widget::widgets::WidgetList;

#[derive(PartialEq, Clone, Debug)]
pub enum TabChoice {
    INVENTORY,
    EQUIPMENT,
    CHARACTER
}

#[derive(Clone)]
pub struct Tab {
    tab_choice: TabChoice,
    pub(crate) title: String
}

impl Tab {
    // Returns the 1st tab
    pub fn first() -> Tab {
        Tab { tab_choice: TabChoice::INVENTORY, title: String::from("Inventory") }
    }

    // Returns all possible tabs in order
    pub fn values() -> Vec<Tab> {
        let inventory_tab = Tab { tab_choice: TabChoice::INVENTORY, title: String::from("Inventory") };
        let equipment_tab = Tab { tab_choice: TabChoice::EQUIPMENT, title: String::from("Equipment") };
        let character_tab = Tab { tab_choice: TabChoice::CHARACTER, title: String::from("Character") };
        vec![inventory_tab, equipment_tab, character_tab]
    }
}

struct CharacterInfoViewFrameData {
    pub character : Character
}

/*
    This View is responsible for the Player's "Character Info" screen i.e Inventory, Character Stats
    Callbacks are used to provide actions (Use, Equip, Drop, etc)
 */
pub struct CharacterInfoView<'a, B : ratatui::backend::Backend> {
    pub character : &'a mut Character,
    pub ui : &'a mut UI,
    pub terminal_manager : &'a mut TerminalManager<B>,
    pub frame_handler: CharacterInfoFrameHandler,
    pub callback : Box<dyn FnMut(ContainerFrameHandlerInputResult) -> Option<ContainerFrameHandlerInputResult> + 'a>
}

impl <B : ratatui::backend::Backend> CharacterInfoView<'_, B> {
    fn initialise(&mut self) {
        let commands: Vec<UsageCommand> = vec![
            UsageCommand::for_container_event(Key::Char('o'), String::from("open"), OpenedContainerEventType::OpenContainer),
            UsageCommand::for_container_event(Key::Char('d'), String::from("drop"), OpenedContainerEventType::DropItems),
            UsageCommand::for_container_event(Key::Char('m'), String::from("move"), OpenedContainerEventType::MoveItems),
            UsageCommand::for_container_event(Key::Char('c'), String::from("move-to-container"), OpenedContainerEventType::MoveItemsToContainerChoice),
            UsageCommand::for_container_event(Key::Char('e'), String::from("equip"), OpenedContainerEventType::EquipItems)
        ];
        let usage_line = UsageLineWidget::for_commands(commands);

        let inventory_view = container::build_container_frame_handler(self.character.get_inventory_mut().clone(), usage_line);
        self.frame_handler.container_frame_handlers = vec!(inventory_view);

        let character_view = CharacterStatsFrameHandler { character: self.character.clone(), widgets: WidgetList { widgets: Vec::new(), widget_index: None }, view_mode: ViewMode::VIEW, attributes_area: Area::new(Position::zero(), 0, 0) };
        self.frame_handler.character_view = Some(character_view);
    }

    // TODO refactor alongside other commands / engine func
    fn re_render(&mut self) -> Result<(), std::io::Error>  {
        let ui = &mut self.ui;
        self.terminal_manager.terminal.draw(|frame| {
            ui.render(None, Map(), frame);
        })?;
        Ok(())
    }

    fn next_tab(&mut self)  {
        let tab_iter = Tab::values().into_iter();
        if let Some(max_index) = tab_iter.size_hint().1 {
            let mut index = 0;
            let mut target_index = None;
            for tab in tab_iter {
                let current_choice = self.frame_handler.tab_choice.clone();
                if tab.tab_choice == current_choice && index == max_index - 1 {
                    // Swap back to the first option
                    self.frame_handler.tab_choice = Tab::first().tab_choice.clone();
                } else if tab.tab_choice == current_choice {
                    target_index = Some(index.clone() + 1);
                }
                index += 1;
            }

            // Select the target tab choice otherwise
            if let Some(idx) = target_index {
                if let Some(tab) = Tab::values().iter().nth(idx) {
                    self.frame_handler.tab_choice = tab.tab_choice.clone();
                }
            }
        }
    }

    /*
      * Handles passthrough to the relevant container choice view (if one is present)
      * Triggering callbacks if needed
      * Returns the optional input result of the container choice view, and a boolean to indicate success
      */
    fn handle_container_choice_input(&mut self, key: Key) -> Result<(Option<GenericInputResult>, bool), ErrorWrapper> {
        if let Some(cfh) = &mut self.frame_handler.choice_frame_handler {
            let result = cfh.handle_input(Some(key))?;
            if let Some(view_specific_result) = result.view_specific_result {
                match view_specific_result {
                    ContainerChoiceFrameHandlerInputResult::Select(selected_container) => {
                        let container_views = &mut self.frame_handler.container_frame_handlers;
                        if let Some(topmost_view) = container_views.last_mut() {
                            let view_specific_result = topmost_view.build_move_items_result().unwrap().view_specific_result.unwrap();
                            match view_specific_result {
                                ContainerFrameHandlerInputResult::MoveToContainerChoice(mut data) => {
                                    // Add the target selected to the callback data
                                    data.target_container = Some(selected_container.clone());
                                    self.trigger_callback(ContainerFrameHandlerInputResult::MoveToContainerChoice(data));
                                    // Clear the frame handler now we're done
                                    self.frame_handler.choice_frame_handler = None;
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

    /*
     * Handles passthrough to the relevant container view (the latest opened)
     * Triggering callbacks if needed
     * Returns the optional input result of the container view, and a boolean to indicate success
     */
    fn handle_container_view_input(&mut self, key: Key) -> Result<(Option<GenericInputResult>, bool), ErrorWrapper> {
        let container_views = &mut self.frame_handler.container_frame_handlers;
        let have_container_views = !container_views.is_empty();
        if have_container_views {
            if let Some(topmost_view) = container_views.last_mut() {
                let result = topmost_view.handle_input(Some(key))?;
                if let Some(view_specific_result) = result.view_specific_result {
                    match view_specific_result {
                        ContainerFrameHandlerInputResult::OpenContainerView(stacked_view) => {
                            container_views.push(stacked_view);
                        },
                        ContainerFrameHandlerInputResult::DropItems(_) => {
                            self.trigger_callback(view_specific_result);
                        },
                        ContainerFrameHandlerInputResult::EquipItems(_) => {
                            self.trigger_callback(view_specific_result);
                        },
                        ContainerFrameHandlerInputResult::MoveItems(_) => {
                            self.trigger_callback(view_specific_result);
                        },
                        ContainerFrameHandlerInputResult::MoveToContainerChoice(_) => {
                            self.trigger_callback(view_specific_result);
                        },
                        _ => {}
                    }
                }
                return Ok((Some(result.generic_input_result), true));
            }
        }
        Ok((None, false))
    }

    fn quit_container_view(&mut self) -> Result<bool, Error> {
        let container_views = &mut self.frame_handler.container_frame_handlers;
        if container_views.len() > 1 {
            if let Some(closing_view) = self.frame_handler.container_frame_handlers.pop() {
                let closing_container = closing_view.container;
                if let Some(parent_view) = self.frame_handler.container_frame_handlers.last_mut() {
                    let parent_container = &mut parent_view.container;
                    if let Some(position) = parent_container.position(&closing_container) {
                        parent_container.replace(position, closing_container);
                    }
                }
            }
            return Ok(false)
        } else if container_views.len() == 1 {
            let last_view = &mut self.frame_handler.container_frame_handlers[0];
            self.character.set_inventory(last_view.container.clone());
        }
        return Ok(true)
    }
    
    /*
        Passes the ContainerFrameHandlerInputResult to the most recent container frame handler
     */
    fn pass_result_to_latest_choice_handler(&mut self, result: ContainerFrameHandlerInputResult) {
        // Find most recent container choice view and update it
        if let Some(topmost_view) = self.frame_handler.container_frame_handlers.last_mut() {
            topmost_view.handle_callback_result(result);
        }
    }
}

impl <'c, B : ratatui::backend::Backend> Callback<'c, ContainerFrameHandlerInputResult> for CharacterInfoView<'c, B> {
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
                        self.frame_handler.choice_frame_handler = result.ok()
                    } else {
                        let error = result.err().unwrap();
                        self.ui.set_console_buffer(error.internal_message.unwrap())
                    }
                },
                ContainerFrameHandlerInputResult::MoveItems(ref data) => {
                    if data.target_container.is_some() {
                        // if target_container is the root view container
                        let root_container = self.frame_handler.container_frame_handlers.first().map(|top_cv| top_cv.container.clone()).unwrap();
                        let target_is_root = data.target_container.as_ref().map_or_else(|| false, |t| t.id_equals(&root_container));
                        let target_in_source = data.target_container.as_ref().map_or_else(|| false, |c| data.source.find(c.get_self_item()).is_some());
                        // Target is the topmost container, so we're forced to rebuild everything
                        if target_is_root {
                            // Drain all after the first
                            self.frame_handler.container_frame_handlers.drain(1..);
                            
                            // Then rebuild the remaining (root container) view
                            if let Some(topmost_view) = self.frame_handler.container_frame_handlers.last_mut() {
                                topmost_view.rebuild_to_container(data.target_container.as_ref().unwrap().clone())
                            }
                        } else {
                            // i.e moving to a parent container that isn't the root
                            if !target_in_source {
                                // Rebuild that specific container view
                                if let Some(cv) = self.frame_handler.container_frame_handlers.iter_mut()
                                    .find(|cv| cv.container.id_equals(&data.target_container.as_ref().unwrap())) {
                                    cv.rebuild_to_container(data.target_container.as_ref().unwrap().clone())
                                }
                            }

                            self.pass_result_to_latest_choice_handler(r);
                        }
                    } else {
                        for fh in &mut self.frame_handler.container_frame_handlers {
                            if fh.container.id_equals(&data.source) {
                                fh.handle_callback_result(r.clone())
                            }
                        }
                    }
                }
                _ => {
                    self.pass_result_to_latest_choice_handler(r);
                }
            }
        }
    }
}

impl <'b, B : ratatui::backend::Backend> View<bool> for CharacterInfoView<'_, B>  {
    fn begin(&mut self) -> Result<InputResult<bool>, ErrorWrapper> {
        self.initialise();
        self.terminal_manager.terminal.clear()?;
        self.draw(None)?;
        while !self.handle_input(None).unwrap().generic_input_result.done {
            self.draw(None)?;
        }
        return Ok(InputResult { generic_input_result: GenericInputResult { done: true, requires_view_refresh: true }, view_specific_result: None});
    }

    fn draw(&mut self, _area: Option<Area>) -> Result<CompletedFrame, ErrorWrapper> {
        let frame_handler = &mut self.frame_handler;
        let character = self.character.clone();
        let ui = &mut self.ui;

        verify_display_size::<B>(self.terminal_manager);

        let ui_layout = ui.ui_layout.as_mut().unwrap();
        let frame_size = self.terminal_manager.terminal.get_frame().size();
        let ui_areas: UIAreas = ui_layout.get_or_build_areas(frame_size, LayoutType::StandardSplit).clone();

        if let Some(main) = ui_areas.get_area(UI_AREA_NAME_MAIN) {
            let main_area = main.area;
            return Ok(self.terminal_manager.terminal.draw(|frame| {
                ui.render(None, Map(), frame);
                // Sizes for the entire 'Character Info' frame area

                let rect = Rect { x: main_area.start_position.x, y: main_area.start_position.y + 1, width: main_area.width.clone(), height: main_area.height.clone() - 1 };
                let area = Area::from_rect(rect);
                let specific_frame_data = CharacterInfoViewFrameData { character };
                frame_handler.handle_frame(frame, FrameData { frame_area: area, ui_areas: ui_areas.clone(), data: specific_frame_data });
            })?);
        }
        
        ErrorWrapper::internal_result(String::from("Failed to draw character info view"))
    }
}

impl <COM: ratatui::backend::Backend> InputHandler<bool> for CharacterInfoView<'_, COM> {
    fn handle_input(&mut self, input: Option<Key>) -> Result<InputResult<bool>, ErrorWrapper> {
        let key = resolve_input(input)?;
        match key {
            Key::Esc => {
                let done = self.quit_container_view()?;
                return Ok(InputResult { generic_input_result: GenericInputResult { done, requires_view_refresh: false }, view_specific_result: None});
            },
            // Horizontal tab
            Key::Char('\t') => {
                self.next_tab();
            }
            // Passthrough anything not handled here into the sub views
            _ => {
                let mut generic_input_result: Option<GenericInputResult> = None;
                match self.frame_handler.tab_choice {
                    TabChoice::INVENTORY => {
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

                        // Finally trigger passthrough to standard container views
                        if let Ok((gir_result, _success)) = self.handle_container_view_input(key) {
                            // Rewrap this only if something was returned
                            if let Some(gir) = gir_result {
                                generic_input_result = Some(gir);
                            }
                        }
                    }
                    TabChoice::CHARACTER => {
                        // Future TODO pass-through to character details view??
                    },
                    TabChoice::EQUIPMENT => {
                        // TODO input pass-through to equipment view?
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

impl FrameHandler<CharacterInfoViewFrameData> for CharacterInfoFrameHandler {
    fn handle_frame(&mut self, frame: &mut ratatui::Frame, data: FrameData<CharacterInfoViewFrameData>) {
        let tabs = Tab::values();
        let titles: Vec<_> = tabs.iter().map(|t| t.title.clone()).map(Line::from).collect();
        let selection_index = self.tab_choice.clone() as i32;
        let tabs = Tabs::new(titles)
            .block(Block::default().title("Character Info").borders(Borders::NONE))
            .style(Style::default())
            .highlight_style(Style::default().add_modifier(Modifier::REVERSED))
            .divider(VERTICAL)
            .select(selection_index as usize);

        let frame_size = data.frame_area;
        let heading_area = Area::new(
            Position::new(frame_size.start_position.x + 1, frame_size.start_position.y),
            frame_size.width - 2,
            3
        );
        frame.render_widget(tabs, heading_area.to_rect());

        let ui_areas  = data.ui_areas;
        let mut character = data.data.character;

        // TODO we shouldn't define this area ourselves and instead use ui_areas ??
        // This specifically needs to be a portion of the inner window avoiding the tabs
        let inner_window_area =  Area::new(
            Position::new(frame_size.start_position.x + 1, frame_size.start_position.y + 2),
            frame_size.width - 2,
            frame_size.height - 3
        );

        match self.tab_choice {
            TabChoice::INVENTORY => {
                // Check for container choice frame handler, favor that as a dialog if it exists
                if let Some(cfh) = &mut self.choice_frame_handler {
                    let frame_data = FrameData { data: Vec::new(), ui_areas, frame_area: inner_window_area};
                    cfh.handle_frame(frame, frame_data);
                } else if let Some(topmost_view) = self.container_frame_handlers.last_mut() {
                    let mut frame_inventory = topmost_view.container.clone();
                    topmost_view.handle_frame(frame, FrameData { data: &mut frame_inventory, ui_areas, frame_area: inner_window_area });
                }
            },
            TabChoice::CHARACTER => {
                match &mut self.character_view {
                    Some(char_view) => {
                        char_view.handle_frame(frame,  FrameData { data: character.clone(), ui_areas, frame_area: inner_window_area } );
                    },
                    _ => {}
                }
            },
            TabChoice::EQUIPMENT => {
                let frame_data = FrameData { data: character.get_equipment().clone(), ui_areas, frame_area: inner_window_area};
                let mut equipment_frame_handler = CharacterEquipmentFrameHandler::new();
                equipment_frame_handler.handle_frame(frame, frame_data);
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::terminal::terminal_manager;
    use crate::test::utils::test_utils::build_test_level;
    use crate::ui::ui::build_ui;
    use crate::view::character_info_view::{CharacterInfoFrameHandler, CharacterInfoView, TabChoice};
    use crate::view::MIN_RESOLUTION;

    #[test]
    fn test_initialise() {
        // GIVEN a valid character info view for a player's inventory
        let mut level = build_test_level(None, None);

        let mut ui = build_ui();
        let mut terminal_manager = terminal_manager::init_test(MIN_RESOLUTION).unwrap();
        let frame_handler = CharacterInfoFrameHandler { tab_choice: TabChoice::INVENTORY, container_frame_handlers: Vec::new(), choice_frame_handler: None, character_view: None };
        let mut character_info_view = CharacterInfoView { character: level.characters.get_player_mut().unwrap(), ui: &mut ui, terminal_manager: &mut terminal_manager, frame_handler, callback: Box::new(|_data| {None}) };

        // WHEN we call to initialise
        // THEN we expect it to complete successfully
        character_info_view.initialise();
    }
}