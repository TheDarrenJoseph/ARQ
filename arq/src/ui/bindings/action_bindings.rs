use crate::map::position::Side;
use crate::map::position::Side::{BOTTOM, LEFT, RIGHT, TOP};
use crate::ui::bindings::action_bindings::Action::{DevBeginCombat, Escape, LookAround, MovePlayer, OpenNearby, ShowCharacterInfo};
use crate::ui::bindings::input_bindings::KeyBindings;
use std::collections::HashMap;
use termion::event::Key;
/*
  An Action that the Player can take
 */
#[derive(Debug, Clone)]
pub enum Action {
    ShowCharacterInfo, // Displays inventory and other character info
    DevBeginCombat, // For development of combat view
    LookAround,
    OpenNearby,
    MovePlayer(Side),
    Escape // This can open the pause menu, close a container view, etc
}

#[derive(Debug, Clone)]
pub struct ActionKeyBindings {
   pub bindings : HashMap<Key, Action>
}

pub fn build_default_action_keybindings() -> ActionKeyBindings {
    let mut bindings = HashMap::new();
    bindings.insert(Key::Esc, Escape);
    bindings.insert(Key::Char('c'), DevBeginCombat);
    bindings.insert(Key::Char('i'), ShowCharacterInfo);
    bindings.insert(Key::Char('k'), LookAround);
    bindings.insert(Key::Char('o'), OpenNearby);
    
    
    // Player movement bindings (arrows)
    bindings.insert(Key::Up, MovePlayer(TOP));
    bindings.insert(Key::Down, MovePlayer(BOTTOM));
    bindings.insert(Key::Left, MovePlayer(LEFT));
    bindings.insert(Key::Right, MovePlayer(RIGHT));
    
    // Player movement bindings (WASD)
    bindings.insert(Key::Char('w'), MovePlayer(TOP));
    bindings.insert(Key::Char('s'), MovePlayer(BOTTOM));
    bindings.insert(Key::Char('a'), MovePlayer(LEFT));
    bindings.insert(Key::Char('d'), MovePlayer(RIGHT));

    ActionKeyBindings {
        bindings
    }
}

impl KeyBindings<Action> for ActionKeyBindings {
    fn get_bindings(&self) -> &HashMap<Key, Action> {
       &self.bindings
    }

    fn get_input(&self, key: Key) -> Option<&Action> {
        self.get_bindings().get(&key)
    }
}


