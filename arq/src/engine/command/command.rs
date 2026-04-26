use crate::error::errors::ErrorWrapper;
use crate::ui::bindings::action_bindings::PlayerAction;

/*
 A command is a way for an action to be executed upon the game state
 */
pub trait Command {
    async fn start(&mut self) -> Result<(), ErrorWrapper>;
}

/*
Some commands take Actions on behalf of the Player, and handle input as part of these
Such as looking around, moving, and opening things
 */
pub trait CommandPlayerAction<Input> {
    fn can_handle_action(&self, action: PlayerAction) -> bool;

    fn handle_input(&mut self, input: Option<&Input>) -> Result<(), ErrorWrapper>;
}