extern crate core;

use std::io;

use ratatui::backend::CrosstermBackend;
use termion::input::TermRead;
use termion::raw::RawTerminal;

use crate::engine::engine_helpers::menu::start_menu;
use crate::engine::game_engine::{build_game_engine, GameEngine};
use crate::error::errors::ErrorWrapper;
use crate::ui::ui::StartMenuChoice::Play;
use crate::view::game_over_view::GameOverChoice;

mod global_flags;
mod error;
mod engine;
mod character;
mod terminal;
mod ui;
mod menu;
mod settings;
mod view;
mod widget;
mod test;
mod item_list_selection;
mod option_list_selection;
mod util;
mod progress;
mod sound;
pub mod map;

pub mod input;

async fn begin() -> Result<(), ErrorWrapper> {
    let game_engine: Result<GameEngine<CrosstermBackend<RawTerminal<std::io::Stdout>>>, ErrorWrapper>;
    let terminal_manager = terminal::terminal_manager::init().unwrap();
    game_engine = build_game_engine(terminal_manager);
    let mut engine = game_engine.unwrap();

    log::info!("Displaying start menu..");
    let mut choice = None;
    let mut game_over = false;
    while !game_over {
        engine.init()?;

        let result = start_menu(&mut engine, choice.clone()).await.await;
        match result {
            Ok(Some(goc)) => {
                match goc {
                    GameOverChoice::RESTART => {
                        engine.rebuild();
                        choice = Some(Play);
                    },
                    GameOverChoice::EXIT => {
                      game_over = true;
                    }
                }
            },
            Err(e) => {
                println!("Fatal error: {}", e);
                io::stdin().keys().next().unwrap()?;
                return Ok(())
            },
            Ok(None) => {}
        }
    }
    engine.ui_wrapper.terminal_manager.clear_screen().expect("Failed to clear screen");
    ratatui::restore();
    Ok(())
}

#[tokio::main(worker_threads = 3)]
async fn main<>() {
    log4rs::init_file("log4rs.yaml", Default::default()).unwrap();
    begin().await.expect("Failure in main thread!");
}
