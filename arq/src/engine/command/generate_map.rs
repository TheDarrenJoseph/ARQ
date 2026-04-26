use crate::engine::command::command::Command;
use crate::engine::level::Levels;
use crate::map::map_generator::MapGenerator;
use crate::map::Map;
use crate::progress::MultiStepProgress;
use crate::terminal::terminal_manager::TerminalManager;
use crate::ui::ui::{Draw, UI};
use crate::widget::standard::loading_screen::LoadingScreenWidget;
use crate::widget::StandardWidgetType;
use crate::ErrorWrapper;
use futures::FutureExt;
use log::info;
use std::future::Future;
use std::io;
use termion::input::TermRead;
use tokio::join;
use tokio::sync::mpsc::UnboundedReceiver;

// TODO Consider these
// MapGenerationFrameHandler
pub struct GenerateMapCommand<'a, B: 'static + ratatui::backend::Backend> {
    pub levels: &'a mut Levels,
    pub ui: &'a mut UI,
    pub terminal_manager : &'a mut TerminalManager<B>
}

impl<B: ratatui::backend::Backend> GenerateMapCommand<'_, B> {

    pub async fn generate_map(&mut self) -> Result<GenerateMapResult, ErrorWrapper> {
        let seed = self.levels.get_seed();
        let (progress_tx, progress_rx) = tokio::sync::mpsc::unbounded_channel::<MultiStepProgress>();
        let map_generator =self.levels.build_map_generator(progress_tx.clone());

        // Step 1 - Create the map generator
        let size_x = map_generator.map.area.width;
        let size_y = map_generator.map.area.height;

        info!("Generating map using RNG seed: {} and size: {}, {}", seed, size_x, size_y);

        let current_progress = map_generator.progress.clone();
        let loading_screen_widget = LoadingScreenWidget {
            progress: current_progress.clone()
        };
        // Step 2 -- Register the loading screen widget for Map Generation with the UI
        let standard_widgets = self.ui.get_standard_widgets_mut();
        standard_widgets.push(
            StandardWidgetType::LoadingScreen(
                loading_screen_widget
            )
        );

        // Step 2 -- Lock into loop for generation / progress checking
        progress_tx.send(current_progress.clone()).expect("Map generator progress should have been sent to the tx channel!");

        return Ok(
            GenerateMapResult {
                progress_rx,
                map_generator
            }
        )
    }

    pub async fn handle_progress(&mut self, mut progress_rx: UnboundedReceiver<MultiStepProgress>)  {
        let ui = &mut self.ui;
        let mut done = false;
        while !done {
            let widget =ui.get_standard_widgets_mut().iter_mut().find(|w| match w {
                StandardWidgetType::LoadingScreen(_) => true,
                _ => false
            });

            if let Some(widget_type) = widget {
                match widget_type {
                    StandardWidgetType::LoadingScreen(loading_screen_widget) => {
                        // Keep updating the loading screen widget progress
                        let current_progress = progress_rx.recv().await.unwrap();
                        info!("Current map generation progress: {}/{}", current_progress.get_current_step_number(), current_progress.step_count() - 1);
                        loading_screen_widget.progress = current_progress;
                        done = loading_screen_widget.progress.is_done();
                    }
                    _ => {}
                }
            }

            let _ = self.terminal_manager.terminal.draw(|frame| {
                ui.draw_loading_screen(frame);
            });
        }
    }
}

pub struct GenerateMapResult {
    pub progress_rx: UnboundedReceiver<MultiStepProgress>,
    pub map_generator: MapGenerator
}

impl<B: ratatui::backend::Backend> Command<Map> for GenerateMapCommand<'_, B> {

    async fn start(&mut self) -> Result<Map, ErrorWrapper> {
        let map_result_wrapper = self.generate_map().await?;

        let progress_rx = map_result_wrapper.progress_rx;
        let mut map_generator = map_result_wrapper.map_generator.clone();

        let map_task = map_generator.generate();

        let progress_task = self.handle_progress(progress_rx);

        let done = join!(progress_task, map_task);
        let done_generator = done.1;

        // Update the RNG to avoid it remaining in initial state
        self.levels.rng = done_generator.rng.clone();

        // Block for input as the loading screen asks
        io::stdin().keys().next().unwrap().expect("Failed to wait for [any key]");

        // Remove the loading screen
        let standard_widgets = self.ui.get_standard_widgets_mut();
        let loading_screen_widget_index =  standard_widgets.iter_mut().position(|w| match w {
            StandardWidgetType::LoadingScreen(_) => true,
            _ => false
        }).expect("Failed to find loading screen widget to remove it from the UI");
        standard_widgets.remove(loading_screen_widget_index);

        return Ok(done_generator.map.clone());
    }
}