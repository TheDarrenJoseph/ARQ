use std::io;
use std::time::Instant;

use log::debug;
use termion::event::Key;
use ratatui::backend::Backend;
use ratatui::CompletedFrame;

use crate::character::Character;
use crate::engine::level::{Level, LevelChange};
use crate::error::errors::ErrorWrapper;
use crate::map::map_view_areas::{calculate_map_display_area, MapViewAreas};
use crate::map::position::{build_rectangular_area, Area, Position};
use crate::map::room::Room;
use crate::menu;
use crate::terminal::terminal_manager::TerminalManager;
use crate::ui::ui::{get_input_key, Draw, StartMenuChoice, UI};
use crate::ui::ui::UIViewMode::Map;
use crate::ui::ui_areas::{UIAreas, UI_AREA_NAME_MAIN};
use crate::ui::ui_layout::LayoutType;
use crate::view::framehandler::character_stats::CharacterFrameHandlerInputResult::VALIDATION;
use crate::view::framehandler::character_stats::{CharacterFrameHandlerInputResult, CharacterStatsFrameHandler, ViewMode};
use crate::view::framehandler::{FrameData, FrameHandler};
use crate::view::map_view::MapView;
use crate::view::menu_view::MenuView;
use crate::view::{verify_display_size, GenericInputResult, InputHandler, InputResult, View};
use crate::widget::widgets::WidgetList;

pub struct UIWrapper<B: 'static + ratatui::backend::Backend> {
    pub(crate) ui : UI,
    pub(crate) terminal_manager : TerminalManager<B>,
}

const UI_USAGE_HINT: &str = "Use the arrow keys/WASD to move.\nEsc - Menu";

impl <B : Backend> UIWrapper<B> {
    // TODO refactor into a singular component shared with commands
    pub(crate) fn re_render(&mut self) -> Result<(), io::Error>  {
        let ui = &mut self.ui;
        self.terminal_manager.terminal.draw(|frame| {
            ui.render(None, Map(), frame);
        })?;
        Ok(())
    }

    pub(crate) fn re_render_all(&mut self, level: Level) -> Result<(), io::Error>  {
        let ui = &mut self.ui;
        self.terminal_manager.terminal.draw(|frame| {
            ui.render(Some(level), Map(), frame);
        })?;
        Ok(())
    }

    /*
      Prints a string to the console and re-renders the most basic UI elements
     */
    pub(crate) fn print_and_re_render(&mut self, message: String) -> Result<(), io::Error> {
        self.ui.set_console_buffer(message);
        self.re_render()
    }

    /*
      Prints a string to the console and re-renders using level info
     */
    pub(crate) fn print_and_re_render_all(&mut self, level: Level, message: String) -> Result<(), io::Error> {
        self.ui.set_console_buffer(message);
        self.re_render_all(level)
    }


    // TODO fix this path rendering
    fn get_prompted_input(&mut self, level: Level, prompt: String) -> Result<Key, io::Error> {
        self.print_and_re_render_all(level, prompt)?;
        get_input_key()
    }

    pub fn yes_or_no(&mut self, level: Level, prompt: String, confirm_message: Option<String>) -> Result<bool, io::Error> {
        let final_prompt = format!("{} (y/n)", prompt);
        loop {
            match self.get_prompted_input(level.clone(), final_prompt.clone())? {
                Key::Char('y') | Key::Char('Y') => {
                    if let Some(message) = confirm_message {
                        let final_message = format!("{} (any key to continue)", message);
                        self.print_and_re_render_all(level, final_message)?;
                        get_input_key()?;
                    }
                    return Ok(true);
                },
                Key::Char('n') | Key::Char('N')  => {
                    return Ok(false);
                }
                _ => {}
            }
        }
    }

    pub(crate) fn draw_start_menu(&mut self, game_running: bool) -> Result<InputResult<StartMenuChoice>, ErrorWrapper>  {
        let ui = &mut self.ui;
        let terminal_manager = &mut self.terminal_manager;

        let menu = menu::build_start_menu(game_running);
        let mut menu_view = MenuView { ui, terminal_manager, menu };

        Ok(menu_view.begin()?)
    }

    pub(crate) fn draw_info(&mut self) -> std::io::Result<CompletedFrame> {
        let ui = &mut self.ui;
        self.terminal_manager.terminal.draw(move |frame| { ui.draw_info(frame) })
    }

    // TODO this should live in it's own view likely
    // Shows character creation screen
    // Returns the finished character once input is confirmed
    fn show_character_creation(&mut self, base_character: Character) -> Result<Character, ErrorWrapper> {
        let mut character_view = CharacterStatsFrameHandler { character: base_character.clone(),  widgets: WidgetList { widgets: Vec::new(), widget_index: None }, view_mode: ViewMode::CREATION, attributes_area: Area::new(Position::zero(), 0, 0)};
        // Begin capture of a new character
        let mut character_creation_result = InputResult { generic_input_result:
        GenericInputResult { done: false, requires_view_refresh: false },
            view_specific_result: None
        };
        while !character_creation_result.generic_input_result.done {
            let ui = &mut self.ui;
            ui.show_console();
            let ui_layout = ui.ui_layout.as_mut().unwrap();
            let frame_size = self.terminal_manager.terminal.get_frame().size();
            let ui_areas: UIAreas = ui_layout.get_or_build_areas(frame_size, LayoutType::StandardSplit).clone();
            if let Some(main) = ui_areas.get_area(UI_AREA_NAME_MAIN) {
                self.terminal_manager.terminal.draw(|frame| {
                    let mut main_area = main.area;
                    main_area.height -= 2;
                    ui.render(None, Map(), frame);
                    character_view.handle_frame(frame, FrameData { data: base_character.clone(), ui_areas: ui_areas.clone(), frame_area: main_area });
                })?;
            }
            ui.hide_console();

            let key = get_input_key()?;
            character_creation_result = character_view.handle_input(Some(key))?;

            match character_creation_result.view_specific_result {
                Some(VALIDATION(message)) => {
                    self.ui.set_console_buffer(message);
                    self.re_render()?;
                },
                Some(CharacterFrameHandlerInputResult::NONE) => {
                    return Ok(character_view.get_character())
                },
                _ => {}
            }
        }
        return Ok(character_view.get_character());
    }

    fn calculate_map_view_area(&self) -> Option<Area> {
        let ui_layout = self.ui.ui_layout.as_ref().unwrap();
        if let Some(main) = ui_layout.get_ui_areas(LayoutType::StandardSplit).get_area(UI_AREA_NAME_MAIN) {
            let main_area = main.area;
            let rect = main_area.to_rect();
            // Main area does not consider borders, so +1 to start inside those
            let map_view_start_pos = Position { x: rect.x + 1, y: rect.y + 1 };
            // Build the view area, -1 for remaining border on the other sides
            let map_view_area = build_rectangular_area(map_view_start_pos, main_area.width - 1, main_area.height - 1);
            return Some(map_view_area);
        }
        return None;
    }

    pub(crate) fn draw_map_view(&mut self, level: &mut Level) -> Result<(), ErrorWrapper> {
        let now = Instant::now();
        verify_display_size(&mut self.terminal_manager);

        // Add the UI usage hint to the console buffer
        self.ui.set_console_buffer(UI_USAGE_HINT.to_string());
        let map_area = level.map.as_ref().unwrap().area;

        // There are 3 map areas to consider:
        // 1. (Map based) Map Area (the position/size of the actual map e.g tiles), this should currently always start at 0,0
        // 2. (Screen based) Map view area (The position/size of the map view relative to the entire terminal frame), this could start at 1,1 for example (accounting for borders)
        // 3. (Screen/Map based) Map display area (The position/size of the map 'viewfinder', the area that you can actually see the map through)
        // 3.1 The map display area is what will move with the character throughout larger maps
        let map_view_area_result = self.calculate_map_view_area();
        if let Some(map_view_area) = map_view_area_result {
            let player_global_position = level.characters.get_player().unwrap().get_global_position();
            let map_display_area = calculate_map_display_area(player_global_position, map_view_area);
            let map_view_areas = MapViewAreas { map_area, map_view_area, map_display_area };
            let mut map_view = MapView {
                level,
                ui: &mut self.ui,
                terminal_manager: &mut self.terminal_manager, 
                map_view_areas
            };

            // Draw the base UI (incl. console) and the map
            map_view.begin()?;
        }

        //let duration = now.elapsed();
        //debug!("Map view draw took: {}ms", duration.as_millis());
        Ok(())
    }

    pub fn check_room_entry_exits(&mut self, level: Level, room: &Room, pos: Position) -> LevelChange {
        if pos.equals_option(room.get_exit()) {
            match self.yes_or_no(
                level.clone(),
                String::from("You've reached the exit! There's a staircase downwards; would you like to leave?"),
                Some(String::from("You move downstairs a level.."))) {
                Ok(true) => {
                    return LevelChange::DOWN;
                },
                _ => {}
            }
        } else if pos.equals_option(room.get_entry()) {
            match self.yes_or_no(
                level.clone(),
                String::from("This is the entrance. There's a staircase upwards; wold you like to leave?"),
                Some(String::from("You move upstairs a level.."))) {
                Ok(true) => {
                    return LevelChange::UP;
                },
                _ => {}
            }
        }

        return LevelChange::NONE;
    }

    pub fn clear_screen(&mut self) -> Result<(), io::Error> {
        self.terminal_manager.terminal.clear()
    }
}