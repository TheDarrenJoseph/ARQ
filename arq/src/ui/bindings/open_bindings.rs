use crate::map::position::Side;
use crate::ui::bindings::input_bindings::KeyBindings;
use crate::ui::bindings::open_bindings::OpenInput::{OpenDown, OpenLeft, OpenRight, OpenUp};
use std::collections::HashMap;
use termion::event::Key;

#[derive(Debug, Clone)]
pub enum OpenInput {
    OpenUp,
    OpenDown,
    OpenLeft,
    OpenRight
}

#[derive(Debug, Clone)]
pub struct OpenKeyBindings {
    bindings : HashMap<Key, OpenInput>
}

pub fn build_default_open_keybindings() -> OpenKeyBindings {
    let mut bindings = HashMap::new();
    bindings.insert(Key::Up, OpenUp);
    bindings.insert(Key::Char('w'), OpenUp);

    bindings.insert(Key::Down, OpenDown);
    bindings.insert(Key::Char('a'), OpenDown);

    bindings.insert(Key::Left, OpenLeft);
    bindings.insert(Key::Char('s'), OpenLeft);

    bindings.insert(Key::Right, OpenRight);
    bindings.insert(Key::Char('d'), OpenRight);

    OpenKeyBindings {
        bindings
    }
}

impl KeyBindings<OpenInput> for OpenKeyBindings {
    fn get_bindings(&self) -> &HashMap<Key, OpenInput> {
       &self.bindings
    }

    fn get_input(&self, key: Key) -> Option<&OpenInput> {
        self.get_bindings().get(&key)
    }
}

pub fn map_open_input_to_side(look_input: Option<OpenInput>) -> Option<Side> {
    return if let Some(input) = look_input {
        match input {
            OpenInput::OpenUp => Option::from(Side::TOP),
            OpenInput::OpenDown => Option::from(Side::BOTTOM),
            OpenInput::OpenLeft => Option::from(Side::LEFT),
            OpenInput::OpenRight => Option::from(Side::RIGHT),
            _ =>  None
        }
    } else {
        None
    }
}


