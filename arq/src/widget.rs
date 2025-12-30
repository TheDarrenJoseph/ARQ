use crate::widget::standard::character_stat_line::CharacterStatLineWidget;
use crate::widget::standard::usage_line::UsageLineWidget;
use crate::widget::stateful::boolean_widget::BooleanState;
use crate::widget::stateful::button_widget::ButtonState;
use crate::widget::stateful::character_info_widget::CharacterInfoWidget;
use crate::widget::stateful::console_input_widget::ConsoleInputState;
use crate::widget::stateful::container_choice_widget::ContainerChoiceWidget;
use crate::widget::stateful::container_widget::ContainerWidget;
use crate::widget::stateful::dropdown_widget::DropdownInputState;
use crate::widget::stateful::map_widget::MapWidget;
use crate::widget::stateful::number_widget::NumberInputState;
use crate::widget::stateful::text_widget::TextInputState;
pub mod widgets;

pub mod standard;
pub mod stateful;

pub fn build_buffer(length: i8, input: String) -> String {
    let mut buffer = String::from("");
    for i in 0..length {
        let idx = i as usize;
        let input_char =  input.chars().nth(idx);
        match input_char {
            Some(s) => {
                buffer.push(s);
            }, None => {
                buffer.push(' ');
            }
        }
    }
    return buffer;
}


#[derive(Debug)]
pub enum StatefulWidgetType {
    Text(TextInputState),
    Boolean(BooleanState),
    Console(ConsoleInputState),
    Number(NumberInputState),
    Dropdown(DropdownInputState),
    Button(ButtonState),
    Map(MapWidget),
    Container(ContainerWidget),
    CharacterInfo(CharacterInfoWidget),
    ContainerChoice(ContainerChoiceWidget),
}

// Non stateful
#[derive(PartialEq, Eq)]
pub enum StandardWidgetType {
    StatLine(CharacterStatLineWidget),
    UsageLine(UsageLineWidget)
}

pub struct StatefulWidgetState {
    pub state_type: StatefulWidgetType
}

pub trait Named {
    fn get_name(&self) -> String;
}

impl Named for StatefulWidgetType {
    fn get_name(&self) -> String {
        match self {
            StatefulWidgetType::Text(state) => {
                state.get_name()
            },
            StatefulWidgetType::Boolean(state) => {
                state.get_name()
            },
            StatefulWidgetType::Console(_state) => {
                String::from("Console")
            },
            StatefulWidgetType::Number(state) => {
                state.get_name().clone()
            },
            StatefulWidgetType::Dropdown(state) => {
                state.get_name().clone()
            },
            StatefulWidgetType::Button(state) => {
                state.get_name()
            },
            StatefulWidgetType::Map(_state) => {
                String::from("Map")
            },
            StatefulWidgetType::Container(_state) => {
                String::from("Container")
            },
            _ => { "".to_string() }
        }
    }
}

pub trait Focusable {
    fn focus(&mut self);
    fn unfocus(&mut self);
    fn is_focused(&mut self) -> bool;
}

impl Focusable for StatefulWidgetType {
    fn focus(&mut self) {
        match self {
            StatefulWidgetType::Text(state) =>  state.selected = true,
            StatefulWidgetType::Console(state) => state.selected = true,
            StatefulWidgetType::Number(state) => state.selected = true,
            StatefulWidgetType::Dropdown(state) => state.selected = true,
            StatefulWidgetType::Button(state) => state.selected = true,
            StatefulWidgetType::Boolean(state) => state.selected = true,
            _ => {}
        }
    }

    fn unfocus(&mut self) {
        match self {
            StatefulWidgetType::Text(state) =>  state.selected = false,
            StatefulWidgetType::Console(state) => state.selected = false,
            StatefulWidgetType::Number(state) => state.selected = false,
            StatefulWidgetType::Dropdown(state) => state.selected = false,
            StatefulWidgetType::Button(state) => state.selected = false,
            StatefulWidgetType::Boolean(state) => state.selected = false,
            _ => {}
        }
    }

    fn is_focused(&mut self) -> bool {
        match self {
            StatefulWidgetType::Text(state) => state.selected.clone(),
            StatefulWidgetType::Console(state) => state.selected.clone(),
            StatefulWidgetType::Number(state) => state.selected.clone(),
            StatefulWidgetType::Dropdown(state) => state.selected.clone(),
            StatefulWidgetType::Button(state) => state.selected.clone(),
            StatefulWidgetType::Boolean(state) => state.selected.clone(),
            _ => { false }
        }
    }
}
