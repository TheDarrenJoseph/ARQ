use std::io;

use ratatui::CompletedFrame;
use termion::event::Key;

use crate::character::battle::Battle;
use crate::character::equipment::WeaponSlot;
use crate::engine::combat::CombatTurnChoice;
use crate::engine::level::Level;
use crate::error::errors::ErrorWrapper;
use crate::map::position::Area;
use crate::terminal::terminal_manager::TerminalManager;
use crate::ui::ui::UIViewMode::Map;
use crate::ui::ui::UI;
use crate::ui::ui_layout::LayoutType;
use crate::view::framehandler::combat::CombatFrameHandler;
use crate::view::framehandler::{FrameData, FrameHandler};
use crate::view::util::callback::Callback;
use crate::view::{resolve_input, verify_display_size, GenericInputResult, InputHandler, InputResult, View};

pub struct CombatView<'a, B : ratatui::backend::Backend>  {
    ui : &'a mut UI,
    terminal_manager : &'a mut TerminalManager<B>,
    level: Level,
    battle : Battle,
    frame_handler : CombatFrameHandler,
    callback : Box<dyn FnMut(CombatCallbackData) -> Option<CombatCallbackData> + 'a>
}

impl  <B: ratatui::backend::Backend> CombatView<'_, B> {
    pub fn new<'a>(ui: &'a mut UI, terminal_manager: &'a mut TerminalManager<B>, level: Level, battle: Battle) -> CombatView<'a, B> {
        let frame_handler = CombatFrameHandler::new(level.clone());
        let callback = Box::new(|_data| {None});
        CombatView { ui, terminal_manager, level: level, battle, frame_handler, callback }
    }

    fn re_render(&mut self) -> Result<(), io::Error>  {
        let ui = &mut self.ui;
        self.terminal_manager.terminal.draw(|frame| {
            ui.render(None, Map(), frame);
        })?;
        Ok(())
    }
}

impl <B : ratatui::backend::Backend> CombatView<'_, B> {
    fn build_done_result(&self) -> InputResult<Battle> {
        InputResult { generic_input_result: GenericInputResult { done: true, requires_view_refresh: false }, view_specific_result: Some(self.battle.clone())}
    }

    fn build_input_done_result(&self) -> InputResult<bool> {
        InputResult { generic_input_result: GenericInputResult { done: true, requires_view_refresh: false }, view_specific_result: None}
    }

    fn build_input_not_done_result(&self) -> InputResult<bool> {
        InputResult { generic_input_result: GenericInputResult { done: false, requires_view_refresh: false }, view_specific_result: None}
    }
}

impl <B : ratatui::backend::Backend> View<Battle> for CombatView<'_, B>  {
    fn begin(&mut self) -> Result<InputResult<Battle>, ErrorWrapper> {
        // Input / Output loop
        while self.battle.in_progress {
            self.draw(None).expect("Combat view should have been drawn.");

            let input_result = self.handle_input(None).unwrap();
            if input_result.generic_input_result.done {
                return Ok(self.build_done_result());
            }

            // TODO get battle action
            // callback w/ action to trigger processing
            // Show action results
        }
        return Ok(self.build_done_result());
    }

    fn draw(&mut self, _area: Option<Area>) -> Result<CompletedFrame, ErrorWrapper> {
        let battle = &mut self.battle;
        let _player = battle.characters.get_player_mut();
        let _npcs = battle.characters.get_npcs();

        let ui = &mut self.ui;
        ui.show_console();
        self.terminal_manager.clear_screen().expect("Screen should have been cleared");
        verify_display_size::<B>(self.terminal_manager);
        let fh = &mut self.frame_handler;

        let frame_area = Area::from_rect(self.terminal_manager.terminal.get_frame().size());
        let ui_layout = ui.ui_layout.as_mut().unwrap();
        let ui_areas = ui_layout.get_or_build_areas(frame_area.to_rect(), LayoutType::CombatView);

        // TODO get the view areas and pass them to the FrameHandler
        return Ok(self.terminal_manager.terminal.draw(|frame| {
            // TODO using frame_size here is risky and doesn't respect UILayout
            let frame_data = FrameData { frame_area: frame_area, data: battle.clone(), ui_areas: ui_areas.clone() };
            fh.handle_frame(frame, frame_data);
        }).expect("Frame should have been drawn."));
    }
}

impl <COM: ratatui::backend::Backend> InputHandler<bool> for CombatView<'_, COM> {
    fn handle_input(&mut self, input: Option<Key>) -> Result<InputResult<bool>, ErrorWrapper> {
        let key = resolve_input(input)?;
        match key {
            Key::Up => {
                let index = self.frame_handler.selection.index;
                let option_count = self.frame_handler.selection.options.len();
                if option_count > 0 && index > 0 {
                    self.frame_handler.selection.index -= 1;
                }
                return Ok(self.build_input_not_done_result());
            },
            Key::Down => {
                let index = self.frame_handler.selection.index;
                let option_count = self.frame_handler.selection.options.len();
                if option_count > 0 && index < option_count as u16 - 1 {
                    self.frame_handler.selection.index += 1;
                }
                return Ok(self.build_input_not_done_result());
            },
            // Enter key
            crate::global_flags::ENTER_KEY => {
                let selection = &self.frame_handler.selection;
                let _option_chosen = selection.options.get(selection.index as usize).unwrap();

                let data = CombatCallbackData { choice: CombatTurnChoice::ATTACK(WeaponSlot::PRIMARY), result: None };
                //self.trigger_callback(data);

                // TODO send-recieve battle turn option
                return Ok(self.build_input_not_done_result());
            },
            Key::Esc => {
                return Ok(self.build_input_done_result());
            },
            _ => {
                return Ok(self.build_input_not_done_result());
            }
        }
    }
}

#[derive(Clone)]
pub struct CombatCallbackData {
    pub choice: CombatTurnChoice,
    pub result: Option<CombatResult>
}

#[derive(Clone)]
pub struct CombatResult {
    pub(crate) messages: Vec<String>
}

impl <'a, B : ratatui::backend::Backend> Callback <'a, CombatCallbackData> for CombatView<'a, B>  {
    fn set_callback(&mut self, callback: Box<impl FnMut(CombatCallbackData) -> Option<CombatCallbackData> + 'a>) {
        self.callback = callback;
    }

    /*
        Triggers a callback to handle the battle logic behind a given combat turn choice
     */
    fn trigger_callback(&mut self, data: CombatCallbackData) {
        let result = (self.callback)(data);
        self.handle_callback_result(result);
    }

    /*
        Any information about the result of a battle action callback will be handled here
     */
    fn handle_callback_result(&mut self, data: Option<CombatCallbackData>) {
        if let Some(_data) = data.clone() {
            // TODO pass messages into the framehandler
        }
    }
}
