use ratatui::buffer::Buffer;
use ratatui::layout::Rect;
use ratatui::style::{Modifier, Style};
use ratatui::widgets::{Block, Borders, StatefulWidget};
use ratatui::prelude::Widget;

use crate::widget::{build_buffer, StatefulWidgetType};

#[derive(Clone)]
#[derive(Debug)]
pub struct ConsoleInputState {
    pub selected: bool,
    length: i8,
    input : String,
    input_padding: i8,
    selected_index: i8,
}

impl ConsoleInputState {
    pub const fn new(
        length: i8,
        input: String,
        input_padding: i8
    ) -> ConsoleInputState {
        ConsoleInputState { selected: false, length, input, input_padding, selected_index: 0 }
    }
}

pub fn build_console_input(length: i8, input: String, input_padding: i8) -> StatefulWidgetType {
    StatefulWidgetType::Console( ConsoleInputState { selected: false, length, input, input_padding,  selected_index: 0 })
}

impl ConsoleInputState {
    pub fn buffer_full(&self) -> bool {
        return self.input.len() >= self.length.clone() as usize;
    }

    pub fn add_char(&mut self, c : char) {
        if !self.buffer_full() {
            self.input.push_str(&String::from(c));
        }
    }

    pub fn delete_char(&mut self) {
        if self.input.len() > 0 {
            self.input.pop();
        }
    }

    pub fn get_input(&self) -> String {
        self.input.clone()
    }

    pub fn set_input(&mut self, input: String)  {
        self.input = input
    }

}

impl StatefulWidget for ConsoleInputState {
    type State = ConsoleInputState;

    fn render(self, area: Rect, buf: &mut Buffer, _state: &mut Self::State) {
        let window_block = Block::default()
            .borders(Borders::ALL);
        window_block.render(area, buf);

        let input_start_index = area.left() + self.input_padding as u16;
        let input = self.input;
        let current_cursor_index = input_start_index + input.len() as u16;
        let max_index = input_start_index + self.length as u16;
        let input_buffer = build_buffer(self.length.clone(), input.clone());

        // Start on the 2nd line to prevent writing on the border
        let mut line_no = self.input_padding as u16;
        for line in input_buffer.lines() {
            if line_no < area.height {
                buf.set_string(input_start_index, area.top() + line_no, line, Style::default());
                line_no += 1;
            } else {
                break;
            }
        }

        if self.selected && current_cursor_index < max_index {
            let selected_cell = buf.get_mut(current_cursor_index as u16, area.top());
            selected_cell.set_style(Style::default().add_modifier(Modifier::UNDERLINED));
        }
    }
}
