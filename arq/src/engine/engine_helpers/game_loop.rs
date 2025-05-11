use futures::future::err;
use log::error;
use crate::engine::game_engine::GameEngine;
use crate::error::errors::{ErrorType, ErrorWrapper};
use crate::view::game_over_view::GameOverChoice;

pub async fn game_loop<B: ratatui::backend::Backend + Send>(engine: &mut GameEngine<B>) -> Result<Option<GameOverChoice>, ErrorWrapper> {
    let game_over_result = engine.player_turn().await;
    engine.re_render_all().await?;
    match game_over_result {
        Ok(goc) => {
            npc_turns(engine)?;
            return Ok(goc);
        },
        Err(e) => {
            match e.error_type {
                // Handle displayable errors by putting the message into the console
                ErrorType::DISPLAYABLE => {
                    engine.ui_wrapper.ui.set_console_buffer(e.displayable_message.clone().unwrap());
                    engine.ui_wrapper.re_render()?;
                    // TODO use a mockable input handler
                    //self.input_handler.get_input_key()?;
                    return Ok(None)
                }
                // Handle internal errors by logging them 
                ErrorType::INTERNAL => {
                    error!("Internal error: {}", e);
                    return Ok(None)
                }
                ErrorType::IO => {
                    return Err(e)
                }
            }
        }
    }
}

fn npc_turns<B: ratatui::backend::Backend + Send>(_engine: &mut GameEngine<B>)  -> Result<(), ErrorWrapper> {
    // TODO NPC movement
    return Ok(());
}
