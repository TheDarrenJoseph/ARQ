use crate::error::errors::ErrorWrapper;
use log::error;
use ratatui::style::{Color, Style};
use ratatui::text::Span;
use ratatui::widgets::{Block, Borders, Paragraph};
use ratatui::CompletedFrame;

use crate::map::position::{Area, Position};
use crate::terminal::terminal_manager::TerminalManager;
use crate::ui::ui::{get_input_key, UI};
use crate::ui::ui_util::{center_area, MIN_RESOLUTION};
use crate::view::{GenericInputResult, InputResult, View};

pub struct DialogView<'a, B : ratatui::backend::Backend> {
    message: String,
    ui : &'a mut UI,
    terminal_manager : &'a mut TerminalManager<B>,
}

impl <B : ratatui::backend::Backend> DialogView<'_, B> {
    pub fn new<'a>(ui: &'a mut UI, terminal_manager: &'a mut TerminalManager<B>, message: String) -> DialogView<'a, B> {
        DialogView { ui, terminal_manager, message }
    }
}

impl <'b, B : ratatui::backend::Backend> View<()> for DialogView<'_, B>  {
    fn begin(&mut self) -> Result<InputResult<()>, ErrorWrapper> {
        self.draw(None).expect("The dialog view should have been drawn.");
        get_input_key().expect("Keyboard input key should have been captured");
        Ok(InputResult {
            generic_input_result: GenericInputResult { done: false, requires_view_refresh: false },
            view_specific_result: None
        })
    }

    fn draw(&mut self, _area: Option<Area>) -> Result<CompletedFrame, ErrorWrapper> {
        let message = self.message.clone();
        let _ui = &mut self.ui;
        self.terminal_manager.clear_screen().expect("The screen should have been cleared");
        return Ok(self.terminal_manager.terminal.draw(|frame| {

            // First check for the minimum space and center the dialog
            let centered_area_result = center_area(MIN_RESOLUTION.to_rect(), frame.size(), MIN_RESOLUTION);
            if let Ok(area) = centered_area_result {
                let block = Block::default()
                    .borders(Borders::ALL)
                    .style(Style::default().bg(Color::Black));
                frame.render_widget(block, area.to_rect());

                let paragraph = Paragraph::new(Span::from(message.clone()));

                let message_area = Area::new(
                    Position::new(area.start_position.x + 2, area.start_position.y + 2),
                    1,
                    area.width / 2
                );
                frame.render_widget(paragraph, message_area.to_rect());

                let enter_text = String::from("[Enter]");
                let paragraph = Paragraph::new(Span::from(enter_text.clone()));

                let message_start_x = (area.width + area.start_position.x) / 2 - enter_text.len() as u16;
                let message_start_y = area.height + area.start_position.y - 1;
                let message_area = Area::new(
                    Position::new(message_start_x, message_start_y),
                    enter_text.len() as u16,
                    1
                );
                frame.render_widget(paragraph, message_area.to_rect());
            } else {
                let err = centered_area_result.err().unwrap();
                error!("{}", err);
                // TODO update views to be able to return Error
            }
        }).expect("The dialog view should have been drawn!"));
    }
}
