use std::future::Future;
use std::io;
use std::io::Error;
use std::pin::Pin;

use log::{error, info};
use termion::input::TermRead;

use crate::engine::game_engine::GameEngine;
use crate::error::errors::ErrorWrapper;
use crate::settings::Settings;
use crate::ui::ui::StartMenuChoice;
use crate::view::game_over_view::GameOverChoice;
use crate::view::settings_menu_view::SettingsMenuView;
use crate::view::View;
use crate::widget::stateful::dropdown_widget::get_resolution_dropdown_options;
use crate::widget::widgets::{build_settings_widgets, WidgetList};
use crate::widget::StatefulWidgetType;

pub async fn start_menu<B: ratatui::backend::Backend + Send>(engine: &mut GameEngine<B>, _choice: Option<StartMenuChoice>) -> Pin<Box<dyn Future< Output = Result<Option<GameOverChoice>, ErrorWrapper> > + '_ >> {
    Box::pin(async move {
        engine.update_from_settings().expect("Failed to update from settings");
        let game_running= engine.is_game_running();
        let ui_wrapper = &mut engine.ui_wrapper;
        ui_wrapper.clear_screen()?;

        // Hide additional widgets when paused
        ui_wrapper.ui.render_additional = false;
        let start_choice = ui_wrapper.draw_start_menu(game_running)?.view_specific_result.unwrap();
        match start_choice {
            StartMenuChoice::Play => {
                ui_wrapper.ui.render_additional = true;
                if !engine.is_game_running() {
                    info!("Starting game..");
                    if let Some(goc) = engine.start_game().await? {
                        return Ok(Some(goc));
                    }
                    return Ok(None);
                } else {
                    return Ok(None);
                }
            },
            StartMenuChoice::Settings => {
                info!("Showing settings..");

                let widgets = build_settings_widgets(&engine.settings);
                let mut settings_menu = SettingsMenuView {
                    ui: &mut ui_wrapper.ui,
                    terminal_manager: &mut ui_wrapper.terminal_manager,
                    menu: crate::view::util::widget_menu::WidgetMenu {
                        selected_widget: Some(0),
                        widgets: WidgetList { widgets, widget_index: Some(0) }
                    }
                };

                settings_menu.begin()?;
                let widgets = settings_menu.menu.widgets;
                handle_settings_menu_selection(&mut engine.settings, widgets)?;
                // Ensure we're using any changes to the settings
                engine.update_from_settings()?;
            },
            StartMenuChoice::Info => {
                info!("Showing info..");
                let _ui = &mut engine.ui_wrapper.ui;
                engine.ui_wrapper.draw_info()?;
                io::stdin().keys().next();
            },
            StartMenuChoice::Quit => {
                if engine.is_game_running() {
                    engine.stop_game();
                }
                return Ok(Some(GameOverChoice::EXIT));
            }
        }

        return Ok(None)
    })
}

pub async fn menu_command<B: ratatui::backend::Backend + Send>(engine: &mut GameEngine<B>) -> Result<Option<GameOverChoice>, ErrorWrapper> {
    engine.ui_wrapper.clear_screen()?;
    engine.ui_wrapper.ui.hide_console();

    if let Some(goc) = start_menu(engine, None).await.await? {
        engine.ui_wrapper.ui.show_console();
        engine.ui_wrapper.clear_screen()?;
        return Ok(Some(goc));
    }

    engine.ui_wrapper.ui.show_console();
    engine.ui_wrapper.clear_screen()?;
    Ok(None)
}



// Saves the widget values into the settings
fn handle_settings_menu_selection(settings: &mut Settings, widgets: WidgetList) -> Result<(), Error> {

    for widget in widgets.widgets {
        match widget {
            StatefulWidgetType::Boolean(b) => {
                let setting = settings.bool_settings.iter_mut().find(|x| x.name == b.get_name());
                if let Some(s) = setting {
                    s.value = b.value;
                }
            },
            StatefulWidgetType::Text(t) => {
                let setting = settings.string_settings.iter_mut().find(|x| x.name == t.get_name());
                if let Some(s) = setting {
                    s.value = t.get_input();
                }
            },
            StatefulWidgetType::Number(mut t) => {
                let setting = settings.u32_settings.iter_mut().find(|x| x.name == t.get_name());
                if let Some(s) = setting {
                    s.value = t.get_input() as u32;
                }
            },
            StatefulWidgetType::Dropdown(t) => {
                // Update the setting value from the widget
                let setting = settings.dropdown_settings.iter_mut().find(|x| x.name == t.get_name());
                if let Some(s) = setting {
                    let res_options = get_resolution_dropdown_options();
                    let resolution_option_chosen = res_options.iter().find(|opt| opt.display_name == t.get_selection());
                    if let Some(resolution_option) = resolution_option_chosen {
                        info!("Resolution selected: {:?}", resolution_option.display_name);
                        s.value.chosen_option = resolution_option.clone();
                    } else {
                        error!("No resolution selected!")
                    }
                }
            },
            _ => {}
        }
    }

    Ok(())
}

