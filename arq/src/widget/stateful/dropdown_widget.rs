use log::info;
use ratatui::buffer::Buffer;
use ratatui::layout::Rect;
use ratatui::style::{Color, Modifier, Style};
use ratatui::widgets::StatefulWidget;
use crate::character::equipment::{Equipment, EquipmentSlot, EquipmentSlotItem};
use crate::map::objects::items::Item;
use crate::ui::resolution::Resolution;
use crate::ui::ui_areas::UIArea;
use crate::view::MIN_RESOLUTION;
use crate::widget::{StatefulWidgetType};
use crate::widget::stateful::equipment_widget::EquipmentWidgetData;

#[derive(Clone)]
#[derive(Debug)]
pub struct DropdownInputState {
    pub selected: bool,
    pub editable: bool,
    show_options: bool,
    name: String,
    options : Vec<String>,
    selected_index: i8,
    chosen_option : String
}

impl DropdownInputState {
    pub(crate) fn new(option: DropdownOption<Item>) -> DropdownInputState {

        let option_value_name = if let Some(item) = option.value {
             item.get_name()
         } else {
             "Empty".to_string()
         };

         DropdownInputState {
            selected: false,
            editable: false,
            show_options: false,
            name: option.display_name.to_string(),
            // Only 1 option for now
            options: vec!(option_value_name.clone()),
            selected_index: 0,
            chosen_option: option_value_name
        }
    }
}

#[derive(Clone)]
pub struct DropdownOption<T> {
    pub display_name : String,
    pub value: Option<T>
}

impl DropdownOption<Item> {
    pub(crate) fn new(slot_item: EquipmentSlotItem) -> DropdownOption<Item> {
        let slot_name = slot_item.slot.clone().to_string();
        DropdownOption {
            display_name: slot_name,
            value: slot_item.item
        }
    }
}

pub fn get_resolution_dropdown_options() -> Vec<DropdownOption<Resolution>> {
    let min_resolution_dropdown_option: DropdownOption<Resolution> = DropdownOption { display_name: String::from("80x24"), value: Some(MIN_RESOLUTION) };
    let fullscreen_dropdown_option: DropdownOption<Resolution> = DropdownOption { display_name:  String::from("FULLSCREEN"), value: None };
    vec! [
        fullscreen_dropdown_option,
        min_resolution_dropdown_option
    ]
}

pub struct DropdownSetting<T> {
    pub(crate) options : Vec<T>,
    pub(crate) chosen_option : T
}

impl DropdownInputState {
    pub fn select(&mut self, input : String) {
        match self.options.iter().position(|o| *o == input) {
            Some(idx) => {
                self.selected_index = idx as i8;
                self.chosen_option = self.options[idx].clone();
            }, _ => {}
        }
    }

    pub fn select_next(&mut self) {
        if self.selected_index < self.options.len() as i8 - 1 {
            self.selected_index += 1;
            self.chosen_option = self.options[self.selected_index.clone() as usize].clone();
        }
    }

    pub fn select_previous(&mut self) {
        if self.selected_index > 0 {
            self.selected_index -= 1;
            self.chosen_option = self.options[self.selected_index.clone() as usize].clone();
        }
    }

    pub fn get_selection(&self) -> String {
        return self.chosen_option.clone();
    }

    pub fn is_showing_options(&self) -> bool {
        return self.show_options.clone();
    }

    pub fn get_name(&self) -> String {
        self.name.clone()
    }

    pub fn toggle_show(&mut self) {
        self.show_options = !self.show_options.clone();
    }
}

pub fn build_dropdown(name: String, editable: bool, options: Vec<String>) -> StatefulWidgetType {
    let input_state = DropdownInputState {
        selected: false,
        editable,
        show_options: false,
        name,
        selected_index: 0,
        chosen_option: options[0].to_string(),
        options};

    StatefulWidgetType::Dropdown(input_state)
}


impl StatefulWidget for DropdownInputState {
    type State = DropdownInputState;

    fn render(self, area: Rect, buf: &mut Buffer, _state: &mut Self::State) {
        buf.set_string(area.left(), area.top(), self.name.clone(), Style::default());

        let mut index: u16 = 0;
        let input_offset = area.left() + self.name.clone().len() as u16 + 1;
        if self.selected {
            if self.show_options {
                let selected_option = self.chosen_option.clone();
                for opt in self.options {
                    if opt == selected_option {
                        let selected_input_row = Rect::new(input_offset.clone(), area.top() + index.clone(), 12, 1);
                        log::info!("Selecting dropdown {} row {} : {}", self.name, index, self.chosen_option.clone());
                        buf.set_style(selected_input_row.clone(), Style::default().add_modifier(Modifier::UNDERLINED));
                    }
                    let x = input_offset;
                    let y = area.top() + index.clone();
                    info!("Drawing option at: {}, {}", x, y);
                    buf.set_string(x, y, opt.clone(), Style::default());
                    index += 1;
                }
            } else {
                let style = Style::default()
                    .fg(Color::DarkGray)
                    .add_modifier(Modifier::REVERSED | Modifier::UNDERLINED);
                buf.set_string(input_offset, area.top() + index.clone(), self.chosen_option.clone(), style);
            }
        } else {
            let style = Style::default();
            buf.set_string(input_offset, area.top() + index.clone(),self.chosen_option.clone(), style);
        }

    }
}