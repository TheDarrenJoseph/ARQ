use std::io;

use crate::error::errors::ErrorWrapper;
use ratatui::layout::Rect;
use ratatui::CompletedFrame;
use ratatui::Frame;
use termion::event::Key;
use termion::input::TermRead;

use crate::map::position::Area;
use crate::terminal::terminal_manager::TerminalManager;
pub use crate::ui::resolution::MIN_RESOLUTION;
use crate::ui::ui::get_input_key;
use crate::ui::ui_util::{build_paragraph_multi, check_display_size};

pub mod framehandler;
pub mod util;
pub mod character_info_view;
pub mod map_view;
pub mod settings_menu_view;
pub mod game_over_view;
pub mod combat_view;
pub mod model;
pub mod dialog_view;
pub mod menu_view;

/*
    A "View" is:
     * Responsible for managing a particular screen of the UI
     * Something that usually contains a series of FrameHandler(s) that own widgets / state that is then rendered to each UI frame (the entire window per render)
     * Something that often behaves as a proxy between the engine and underlying framehandlers
     * Able to take control of the UI and I/O (rendering, keyboard input, etc) until it exits
        * In this manner a View can take control / begin an I/O loop (upon calling begin()) while rendering itself via framhandlers

    From a data perspective, a View has direct access to both:
     * UI (ARQ's base UI, window areas, and it's elements (widgets))
     * The terminal manager (from ratatui) which is what allows us access to each frame via draw() which is then passed to each framehandler, this also allows direct access to the terminal display (character printing etc)
 */
pub trait View<T> {
    fn begin(&mut self) -> Result<InputResult<T>, ErrorWrapper>;
    fn draw(&mut self, area: Option<Area>) -> Result<CompletedFrame, ErrorWrapper>;
}

pub trait InputHandler<T> {
    fn handle_input(&mut self, input : Option<Key>) -> Result<InputResult<T>, ErrorWrapper>;
}

pub struct GenericInputResult {
    pub(crate) done: bool,
    pub(crate) requires_view_refresh: bool
}

pub struct InputResult<T> {
    pub(crate) generic_input_result : GenericInputResult,
    pub(crate) view_specific_result : Option<T>
}

pub fn resolve_area<B : ratatui::backend::Backend>(area: Option<Rect>, frame: &Frame) -> Rect {
    return match area {
        Some(a) => { a },
        _ => { frame.size() }
    }
}

pub fn resolve_input(input : Option<Key>) -> Result<Key, io::Error> {
    match input {
        Some(input_key) => {
            Ok(input_key)
        },
        _ => {
            Ok(get_input_key()?)
        }
    }

}

pub fn verify_display_size<B : ratatui::backend::Backend>(terminal_manager : &mut TerminalManager<B>) {
    loop {
        let frame_size = terminal_manager.terminal.size().unwrap();
        let result = check_display_size(Some(frame_size));
        match result {
            Err(_e) => {
                terminal_manager.terminal.clear().expect("Terminal failed to clear!");
                let error_paragraph = build_paragraph_multi(
                    vec![String::from(
                        "Window too small."),
                         format!("Minimum is {}x{}", MIN_RESOLUTION.width, MIN_RESOLUTION.height),
                         String::from("Please resize and hit any key to check again.") ]);
                terminal_manager.terminal.draw(|frame|{
                    frame.render_widget(error_paragraph, frame.area());
                }).expect("Failed to draw the frame!");
                io::stdin().keys().next().unwrap().unwrap();
            },
            _ => {
                return;
            }
        }
    }
}

