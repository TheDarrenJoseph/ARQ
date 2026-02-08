use crate::error::errors::ErrorWrapper;
use ratatui::layout::Rect;
use ratatui::CompletedFrame;
use termion::event::Key;

use crate::map::position::Area;
use crate::terminal::terminal_manager::TerminalManager;
use crate::ui::ui::UI;
use crate::view::util::widget_menu::WidgetMenu;
use crate::view::{resolve_input, GenericInputResult, InputHandler, InputResult, View};
use crate::widget::{Focusable, StatefulWidgetType};

/*
    This view is for allowing you to adjust elements of the game i.e:
    1. Fog of war
    2. Map seed value
    2. Music volume
 */
pub struct SettingsMenuView<'a, B : ratatui::backend::Backend> {
    pub ui : &'a mut UI,
    pub terminal_manager : &'a mut TerminalManager<B>,
    pub menu: WidgetMenu
}

impl <'b, B : ratatui::backend::Backend> View<bool> for SettingsMenuView<'_, B>  {
    fn begin(&mut self)  -> Result<InputResult<bool>, ErrorWrapper> {
        // Select the first widget
        if self.menu.widgets.widgets.len() > 0 {
            self.menu.widgets.widgets[0].state_type.focus();
        }

        self.terminal_manager.terminal.clear()?;
        self.draw(None)?;

        while !InputHandler::handle_input(self, None).unwrap().generic_input_result.done {
            self.draw(None)?;
        }
        return Ok(InputResult { generic_input_result: GenericInputResult { done: true, requires_view_refresh: true }, view_specific_result: None});
    }

    fn draw(&mut self, _area: Option<Area>) -> Result<CompletedFrame<'_>, ErrorWrapper> {
        let menu_view = &mut self.menu;
        let terminal = &mut self.terminal_manager.terminal;
        let widgets = &menu_view.widgets;
        return Ok(terminal.draw(|frame| {
            let frame_size = frame.size();

            let mut offset = 0;
            for widget in widgets.widgets.iter() {
                let widget_area = Rect::new(5, 5 + offset.clone(), frame_size.width.clone() / 2, 1);
                match &widget.state_type {
                    StatefulWidgetType::Text(w) => {
                        frame.render_stateful_widget(w.clone(), widget_area, &mut w.clone());
                    },
                    StatefulWidgetType::Boolean(w) => {
                        frame.render_stateful_widget(w.clone(), widget_area, &mut w.clone());
                    },
                    StatefulWidgetType::Number(number_state) => {
                        frame.render_stateful_widget(number_state.clone(), widget_area, &mut number_state.clone());
                    },
                    StatefulWidgetType::Dropdown(dropdown_state) => {
                        frame.render_stateful_widget(dropdown_state.clone(), widget_area, &mut dropdown_state.clone());
                    },
                    _ => {}
                }
                offset += 1;
            }
        })?);
    }
}

impl <COM: ratatui::backend::Backend> InputHandler<bool> for SettingsMenuView<'_, COM> {
    fn handle_input(&mut self, input: Option<Key>) -> Result<InputResult<bool>, ErrorWrapper> {
        let menu_view = &mut self.menu;
        let key = resolve_input(input)?;
        let mut target_widget = None;
        match menu_view.widgets.widget_index {
            Some(idx) => {
                target_widget = Some(&mut menu_view.widgets.widgets[idx as usize]);
            },
            None => {}
        }

        match key {
            Key::Down => {
                // Check for anything currently busy with focus
                match target_widget {
                    Some(widget) => {
                        match &mut widget.state_type {
                            StatefulWidgetType::Dropdown(state) => {
                                if state.is_showing_options() {
                                    state.select_next();
                                    return Ok(InputResult { generic_input_result: GenericInputResult { done: false, requires_view_refresh: false }, view_specific_result: None});
                                }
                            }
                            _ => {}
                        }
                    },
                    None => {}
                }
                menu_view.widgets.next_widget();
            },
            Key::Up => {
                // Check for anything currently busy with focus
                match target_widget {
                    Some(widget) => {
                        match &mut widget.state_type {
                            StatefulWidgetType::Dropdown(state) => {
                                if state.is_showing_options() {
                                    state.select_previous();
                                    return Ok(InputResult { generic_input_result: GenericInputResult { done: false, requires_view_refresh: false }, view_specific_result: None});
                                }
                            }
                            _ => {}
                        }
                    },
                    None => {}
                }
                menu_view.widgets.previous_widget();
            },
            crate::global_flags::ENTER_KEY => {
                match target_widget {
                    Some(widget) => {
                        match &mut widget.state_type {
                            StatefulWidgetType::Boolean(state) => {
                                state.value = !state.value;
                            },
                            StatefulWidgetType::Dropdown(state) => {
                                state.toggle_show();
                            }
                            _ => {}
                        }
                    },
                    None => {}
                }
            },
            Key::Backspace => {
                match target_widget {
                    Some(widget) => {
                        match &mut widget.state_type {
                            StatefulWidgetType::Text(state) => {
                                state.delete_char();
                            }
                            _ => {}
                        }
                    },
                    None => {}
                }
            }
            Key::Esc => {
                return Ok(InputResult { generic_input_result: GenericInputResult { done: true, requires_view_refresh: false }, view_specific_result: None});
            },
            Key::Char(c) => {
                match target_widget {
                    Some(widget) => {
                        log::info!("Input: {}", c.to_string());
                        match &mut widget.state_type {
                            StatefulWidgetType::Text(state) => {
                                state.add_char(c);
                                log::info!("Widget state input is: {}", state.get_input());
                            },
                            _ => {}
                        }
                    },
                    None => {}
                }
            },
            Key::Left => {
                match target_widget {
                    Some(widget) => {
                        match &mut widget.state_type {
                            StatefulWidgetType::Number(state) => {
                                if state.editable {
                                    state.decrement()
                                }
                            },
                            _ => {}
                        }
                    },
                    None => {}
                }
            },
            Key::Right => {
                match target_widget {
                    Some(widget) => {
                        match &mut widget.state_type {
                            StatefulWidgetType::Number(state) => {
                                if state.editable {
                                    state.increment()
                                }
                            },
                            _ => {}
                        }
                    },
                    None => {}
                }
            }
            _ => {
            }
        }
        return Ok(InputResult { generic_input_result: GenericInputResult { done: false, requires_view_refresh: false }, view_specific_result: None});
    }
}