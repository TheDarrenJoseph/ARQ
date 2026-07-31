use ratatui::buffer::Buffer;
use ratatui::layout::Rect;
use ratatui::style::{Modifier, Style};
use ratatui::widgets::StatefulWidget;

use crate::widget::{build_buffer, StatefulWidgetType};

#[derive(Clone)]
#[derive(Debug)]
pub struct TextInputState {
    pub selected: bool,
    length: i8,
    input : String,
    name: String,
    input_padding: i8,
    selected_index: i8,
}

pub fn build_text_input(length: i8, name: String, input: String, input_padding: i8) -> StatefulWidgetType {
    StatefulWidgetType::Text( TextInputState { selected: false, length, input, name, input_padding,  selected_index: 0 })
}

impl TextInputState {
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

    pub fn get_name(&self) -> String {
        self.name.clone()
    }

}

impl StatefulWidget for TextInputState {
    type State = TextInputState;

    fn render(self, area: Rect, buf: &mut Buffer, _state: &mut Self::State) {

        let input_start_index = area.left() + self.name.len() as u16 + self.input_padding as u16;
        let input = self.input;
        let current_cursor_index = input_start_index + input.len() as u16;
        let max_index = input_start_index + self.length as u16;

        buf.set_string(area.left(), area.top(), self.name.clone(), Style::default());
        let input_buffer = build_buffer(self.length.clone(), input.clone());
        buf.set_string(input_start_index, area.top(), input_buffer, Style::default().add_modifier(Modifier::REVERSED));
        if self.selected && current_cursor_index < max_index {
            let selected_cell = buf.get_mut(current_cursor_index as u16, area.top());
            selected_cell.set_style(Style::default().add_modifier(Modifier::REVERSED | Modifier::UNDERLINED));
        } else if current_cursor_index == max_index {
            let selected_input_row =  Rect::new(input_start_index, area.top(), self.length.clone() as u16, 1);
            buf.set_style(selected_input_row, Style::default().add_modifier(Modifier::REVERSED | Modifier::UNDERLINED));
        }
    }
}
