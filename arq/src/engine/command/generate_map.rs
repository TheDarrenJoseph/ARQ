use crate::terminal::terminal_manager::TerminalManager;
use crate::ui::bindings::look_bindings::LookKeyBindings;
use crate::ui::ui::UI;

pub struct GenerateMapCommand<'a, B: 'static + ratatui::backend::Backend> {
    pub ui: &'a mut UI,
    pub terminal_manager : &'a mut TerminalManager<B>,
    pub bindings : LookKeyBindings
}