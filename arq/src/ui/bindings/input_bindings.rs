use crate::map::position::Side;
use crate::ui::bindings::action_bindings::ActionKeyBindings;
use crate::ui::bindings::inventory_bindings::InventoryKeyBindings;
use crate::ui::bindings::look_bindings::LookKeyBindings;
use crate::ui::bindings::open_bindings::OpenKeyBindings;
use std::collections::HashMap;
use termion::event::Key;

pub(crate) fn key_to_side(key : Key) -> Option<Side> {
    return match key {
        Key::Up | Key::Char('w')=> {
            Some(Side::TOP)
        },
        Key::Down | Key::Char('s') => {
            Some(Side::BOTTOM)
        },
        Key::Left | Key::Char('a') => {
            Some(Side::LEFT)
        },
        Key::Right | Key::Char('d') => {
            Some(Side::RIGHT)
        },
        _ => {
            None
        }
    }
}

pub trait KeyBindings<T> {
    fn get_bindings(&self) -> &HashMap<Key, T>;
    fn get_input(&self, key: Key) -> Option<&T>;
}


#[derive(Debug, Clone)]
pub struct CommandSpecificKeyBindings {
    pub inventory_key_bindings: InventoryKeyBindings,
    pub look_key_bindings: LookKeyBindings,
    pub open_key_bindings: OpenKeyBindings
}

#[derive(Debug, Clone)]
pub struct AllKeyBindings {
    pub action_key_bindings: ActionKeyBindings,
    pub command_specific_key_bindings: CommandSpecificKeyBindings,
}