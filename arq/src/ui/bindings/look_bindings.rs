use crate::map::position::Side;
use crate::ui::bindings::input_bindings::KeyBindings;
use std::collections::HashMap;
use termion::event::Key;

#[derive(Debug, Clone)]
pub enum LookInput {
    LookUp,
    LookDown,
    LookLeft,
    LookRight,
    LookCurrent
}

#[derive(Debug, Clone)]
pub struct LookKeyBindings {
    bindings: HashMap<Key, LookInput>
}

pub fn build_default_look_keybindings() -> LookKeyBindings {
    let mut bindings = HashMap::new();
    bindings.insert(Key::Up, LookInput::LookUp);
    bindings.insert(Key::Char('w'), LookInput::LookUp);
    
    bindings.insert(Key::Down, LookInput::LookDown);
    bindings.insert(Key::Char('a'), LookInput::LookDown);
    
    bindings.insert(Key::Left, LookInput::LookLeft);
    bindings.insert(Key::Char('s'), LookInput::LookLeft);
    
    bindings.insert(Key::Right, LookInput::LookRight);
    bindings.insert(Key::Char('d'), LookInput::LookRight);

    LookKeyBindings { 
        bindings
    }
}

impl KeyBindings<LookInput> for LookKeyBindings {
    fn get_bindings(&self) -> &HashMap<Key, LookInput> {
        &self.bindings
    }

    fn get_input(&self, key: Key) -> Option<&LookInput> {
        self.get_bindings().get(&key)
    }
}

pub fn map_look_input_to_side(look_input: Option<&LookInput>) -> Option<Side> {
    return if let Some(input) = look_input {
        match input {
            LookInput::LookUp => Option::from(Side::TOP),
            LookInput::LookDown => Option::from(Side::BOTTOM),
            LookInput::LookLeft => Option::from(Side::LEFT),
            LookInput::LookRight => Option::from(Side::RIGHT),
            _ =>  None
        }
    } else {
        None
    }
}
