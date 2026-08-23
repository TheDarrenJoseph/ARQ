use crate::ui::ui_util::center_area;
use crate::ui::resolution::Resolution;
use ratatui::prelude::Widget;
use ratatui::buffer::Buffer;
use ratatui::layout::Rect;
use ratatui::prelude::StatefulWidget;
use ratatui::widgets::{Block, Borders};
use termion::event::Key;
use crate::character::{determine_class, Character, Class};
use crate::character::equipment::Equipment;
use crate::character::stats::attributes::all_attributes;
use crate::engine::event::event::UIEventHandler;
use crate::engine::event::ui::UIEvent;
use crate::error::errors::ErrorWrapper;
use crate::map::position::{Area, Position};
use crate::ui::ui_areas::UIArea;
use crate::view::{GenericInputResult, InputResult};
use crate::widget::stateful::button_widget::build_button;
use crate::widget::stateful::container_choice_widget::{ContainerChoiceWidget, ContainerChoiceWidgetData};
use crate::widget::stateful::container_widget::ContainerWidgetData;
use crate::widget::stateful::dropdown_widget::build_dropdown;
use crate::widget::stateful::number_widget::{build_number_input, build_number_input_with_value, NumberInputState};
use crate::widget::stateful::text_widget::build_text_input;
use crate::widget::{Named, StatefulWidgetType};
use crate::widget::widgets::WidgetList;
use crate::widget::Focusable;

#[derive(PartialEq, Clone, Debug)]
pub enum ViewMode {
    CREATION,
    VIEW
}

#[derive(Debug, Clone)]
pub struct CharacterDetailsWidget {
}

impl CharacterDetailsWidget {
    pub(crate) fn new() -> CharacterDetailsWidget {
        CharacterDetailsWidget {
        }
    }
}

#[derive(Debug, Clone)]
pub struct CharacterDetailsWidgetData {
    pub main_ui_area: UIArea,
    pub character : Character,
    pub widget_list: WidgetList,
    pub view_mode : ViewMode,
    pub attributes_area: Area
}

impl CharacterDetailsWidgetData {
    pub(crate) fn new(
        main_ui_area: UIArea,
        character : Character,
        view_mode : ViewMode,
    ) -> CharacterDetailsWidgetData {
        CharacterDetailsWidgetData {
            main_ui_area,
            character,
            widget_list: WidgetList::new(),
            view_mode,
            attributes_area: Area::new(Position::zero(), 0, 0)
        }
    }


    fn build_attribute_inputs(&mut self) {
        let character = &mut self.get_character();

        let mut scores = character.get_attribute_scores();
        for attribute in all_attributes() {
            let score = scores.iter_mut().find(|score| score.attribute == attribute);

            let editable = self.view_mode == ViewMode::CREATION;
            let mut attribute_input = build_number_input(editable,1, attribute.to_string(), 1);
            match attribute_input {
                StatefulWidgetType::Number(ref mut state) => {
                    match score {
                        Some(s) => {
                            state.set_input(s.score.into());
                        },
                        _ => {}
                    }
                },
                _ => {}
            }
            self.widget_list.widgets.push(attribute_input);
        }
        let free_points = build_number_input_with_value(false, character.get_free_attribute_points() as i32, 1, "Free points".to_string(), 1);
        self.widget_list.widgets.push(free_points);
    }

    fn build_widgets(&mut self) {
        let creation_mode = self.view_mode == ViewMode::CREATION;

        let character = &mut self.get_character();

        if creation_mode {
            let name_input = build_text_input(12, String::from("Name"), character.get_name(), 2);
            self.widget_list.widgets.push(name_input);
        }

        let mut class_input = build_dropdown("Class".to_string(), creation_mode,vec!["None".to_string(), "Warrior".to_string()]);
        match class_input {
            StatefulWidgetType::Dropdown(ref mut state) => {
                state.select(character.get_class().to_string())
            }, _ => {}
        }
        self.widget_list.widgets.push(class_input);

        self.build_attribute_inputs();

        if creation_mode {
            let button = build_button("[Enter]".to_string().len() as i8, "[Enter]".to_string());
            self.widget_list.widgets.push(button);
        }

        self.widget_list.widget_index = Some(0);
        self.widget_list.widgets[0].focus();
    }

    pub fn update_free_points(&mut self, free_points: i32) {
        for widget in self.widget_list.widgets.iter_mut() {
            match widget {
                StatefulWidgetType::Number(state) => {
                    if "Free points" == state.get_name() {
                        state.set_input(free_points.clone());
                    }
                },
                _ => {}
            }
        }
    }

    fn validate_character(&mut self) {
        let mut character = self.get_character();
        if character.get_free_attribute_points() > 0 {
            // TODO events / results
           // return VALIDATION(format!("You need to spend the {} remaining point(s).", character.get_free_attribute_points()));
        }
        match character.get_class() {
            Class::None => {
                // TODO events / results
                //return VALIDATION(format!("You must choose a class!"));
            }
            _ => {}
        }
        // TODO events / results
        // return NONE
        return;
    }

    pub fn get_character(&mut self) -> Character {
        let mut character = self.character.clone();
        let mut scores  = character.get_attribute_scores();

        // If we're in creation mode we need to update the base character with all the current inputs
        if self.view_mode == ViewMode::CREATION {
            for widget in self.widget_list.widgets.iter_mut() {
                if String::from("Name") == widget.get_name() {
                    match widget {
                        StatefulWidgetType::Text(state) => {
                            character.set_name(state.get_input());
                        },
                        _ => {}
                    }
                }

                if String::from("Class") == widget.get_name() {
                    match widget {
                        StatefulWidgetType::Dropdown(state) => {
                            let class = determine_class(state.get_selection());
                            match class {
                                Some(c) => {
                                    character.set_class(c);
                                },
                                _ => {}
                            }
                        },
                        _ => {}
                    }
                }
            }

            let mut number_states : Vec<NumberInputState> = Vec::new();
            let ns_options : Vec<Option<NumberInputState>> = self.widget_list.widgets.iter_mut().map(|w| map_number_widget_state(  w)).collect();
            for ns_option in ns_options {
                match ns_option {
                    Some(ns) => {
                        number_states.push(ns)
                    },
                    _ => {}
                }
            }

            for attribute in all_attributes() {
                let number_state = number_states.iter_mut().find(|ns| ns.get_name() == attribute.to_string());
                match number_state {
                    Some(ns) => {
                        let score = scores.iter_mut().find(|score| score.attribute == attribute);
                        match score {
                            Some(s) => {
                                s.score = ns.get_input() as i8;
                            },
                            _ => {}
                        }
                    },
                    _ => {}
                }
            }

            character.set_attribute_scores(scores);
        }

        character
    }
}


impl UIEventHandler for CharacterDetailsWidgetData {
    async fn handle_event(&mut self, event: UIEvent) {
        log::debug!("[container_widget] Handling event: {:?}", event.name());

        let horizontal_tab : char = char::from_u32(0x2409).unwrap();
        let widgets = &mut self.widget_list.widgets;
        let mut selected_widget = None;
        match self.widget_list.widget_index {
            Some(idx) => {
                let widget = &mut widgets[idx as usize];
                widget.focus();
                selected_widget = Some(widget);
            },
            None => {}
        }

        let mut done = false;
        // let default_done_result = Ok(InputResult {
        //     generic_input_result: GenericInputResult { done, requires_view_refresh: true },
        //     view_specific_result: None
        // });

        match event {
            UIEvent::Termion(termion_event) => {
                match termion_event {
                    termion::event::Event::Key(key) => {
                        match key {
                            Key::Esc => {
                                // TODO close event?
                                return;
                            },
                            crate::global_flags::ENTER_KEY => {
                                match selected_widget {
                                    Some(widget) => {
                                        match widget {
                                            StatefulWidgetType::Dropdown(state) => {
                                                state.toggle_show();
                                            },
                                            StatefulWidgetType::Button(state) => {
                                                match state.get_name().as_str() {
                                                    "[Enter]" => {
                                                        // TODO handle events for character validation?
                                                        return;
                                                        // match self.validate_character() {
                                                        //     NONE => {
                                                        //         done = true;
                                                        //         return Ok(InputResult {
                                                        //             generic_input_result: GenericInputResult { done, requires_view_refresh: true },
                                                        //             view_specific_result: Some(NONE)
                                                        //         });
                                                        //     },
                                                        //     other => {
                                                        //         return Ok(InputResult {
                                                        //             generic_input_result: GenericInputResult { done, requires_view_refresh: true },
                                                        //             view_specific_result: Some(other)
                                                        //         });
                                                        //     }
                                                        // }
                                                    },
                                                    _ => {}
                                                }
                                            },
                                            _ => {
                                            }
                                        }
                                    }
                                    None => {}
                                }
                            },
                            Key::Backspace => {
                                match selected_widget {
                                    Some(widget) => {
                                        match widget {
                                            StatefulWidgetType::Text(state) => {
                                                state.delete_char();
                                            }
                                            _ => {}
                                        }

                                    }
                                    None => {}
                                }
                            },
                            Key::Down => {
                                match selected_widget {
                                    Some(widget) => {
                                        match widget {
                                            StatefulWidgetType::Dropdown(state) => {
                                                if state.editable {
                                                    if state.is_showing_options() {
                                                        state.select_next();
                                                    } else {
                                                        self.widget_list.next_widget();
                                                    }
                                                }
                                            },
                                            _ => {
                                                self.widget_list.next_widget();
                                            }
                                        }
                                    }
                                    None => {}
                                }
                            },
                            Key::Up => {
                                match selected_widget {
                                    Some(widget) => {
                                        match widget {
                                            StatefulWidgetType::Dropdown(state) => {
                                                if state.editable {
                                                    if state.is_showing_options() {
                                                        state.select_previous();
                                                    } else {
                                                        self.widget_list.previous_widget();
                                                    }
                                                }
                                            },
                                            _ => {
                                                self.widget_list.previous_widget();
                                            }
                                        }
                                    },
                                    None => {}
                                }
                            },
                            Key::Right => {
                                match selected_widget {
                                    Some(widget) => {
                                        match widget {
                                            StatefulWidgetType::Number(state) => {
                                                if state.editable {
                                                    let free_points = self.character.get_free_attribute_points().clone();
                                                    if free_points > 0 {
                                                        state.increment();
                                                        self.character.set_free_attribute_points(free_points - 1);
                                                        self.update_free_points(free_points.clone() as i32 - 1);
                                                    }
                                                }
                                            },
                                            _ => {}
                                        }
                                    },
                                    None => {}
                                }
                            },
                            Key::Left => {
                                match selected_widget {
                                    Some(widget) => {
                                        match widget {
                                            StatefulWidgetType::Number(state) => {
                                                if state.editable {
                                                    let free_points = self.character.get_free_attribute_points();
                                                    if free_points < self.character.get_max_free_attribute_points() {
                                                        state.decrement();
                                                        self.character.set_free_attribute_points(free_points + 1);
                                                        self.update_free_points(free_points.clone() as i32 + 1);
                                                    }
                                                }
                                            },
                                            _ => {}
                                        }
                                    },
                                    None => {}
                                }
                            },
                            Key::Char(c) => {
                                // For view mode, tab should exit
                                if c == horizontal_tab && self.view_mode == ViewMode::VIEW {
                                    // TODO exit event???
                                    return;
                                }

                                match selected_widget {
                                    Some(widget) => {
                                        log::info!("Input: {}", c.to_string());

                                        match widget {
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
                            _ => {}
                        }
                    },
                    _ => {}
                }
            },
            _ => {}
        }
    }
}


impl StatefulWidget for CharacterDetailsWidget {
    type State = CharacterDetailsWidgetData;

    fn render(self, area: Rect, buf: &mut Buffer, data: &mut Self::State) {
        let title = match data.view_mode {
            ViewMode::CREATION => {
                "Character Creation"
            },
            ViewMode::VIEW => {
                "Character Details"
            }
        };

        let main_area = data.main_ui_area.get_bordered_area();

        let window_block = Block::default()
            .borders(Borders::ALL)
            .title(title);

        // Adjust the inner main window area to account for the tabs
        let window_start_x = main_area.inner.start_position.x;
        let window_start_y = main_area.inner.start_position.y + 1;
        let window_width = main_area.inner.width;
        let window_height = main_area.inner.height - 1;
        let window_area = Rect::new(window_start_x, window_start_y, window_width, window_height);

        data.get_character();
        if data.widget_list.widgets.is_empty() {
            log::info!("Building input widgets...");
            data.build_widgets();
        }

        let attributes_block = Block::default()
            .borders(Borders::ALL)
            .title("Attributes");

        let all_attributes = all_attributes();

        let mut _attribute_start = (data.widget_list.widgets.len() as u16 - 1) - (all_attributes.len() as u16 - 1);

        let attributes_area;
        // To account for the enter button
        if data.view_mode == ViewMode::CREATION {
            _attribute_start -= 1;
            // When creating a character, we have both a main window "Character Creation"
            // And an inner attributes window
            // We also center the attributes window
            let target_area = Rect::new(window_start_x + 1, window_start_y + 1, 50, 10);
            let available_area = Rect::new(window_start_x + 1, window_start_y + 1, window_width - 2, window_height - 2);
            let resolution = Resolution::new(target_area.width, target_area.height);
            let attributes_area_result = center_area(target_area, available_area, resolution);
            // TODO error handling if center_area fails?
            // error!("{}", attributes_area_result.err().unwrap())
            attributes_area = attributes_area_result.unwrap();
            data.attributes_area = attributes_area;
            attributes_block.render(attributes_area.to_rect(), buf);
        } else {
            attributes_area = Area::new(Position::new(window_start_x, window_start_y), window_width, window_height);
        }

        window_block.render(window_area, buf);

        let widgets = data.widget_list.widgets.clone();
        let widget_count = widgets.len();
        if widget_count > 0 {
            let x_offset = 1;
            let mut y_offset = 1;
            for widget in widgets {
                let widget_area = Rect::new(
                    attributes_area.start_position.x + x_offset,
                    attributes_area.start_position.y + y_offset,
                    attributes_area.width.clone() / 2,
                    1
                );

                match widget {
                    StatefulWidgetType::Text(w) => {
                        w.clone().render(widget_area, buf, &mut w.clone());
                    },
                    StatefulWidgetType::Dropdown(w) => {
                        w.clone().render(widget_area, buf, &mut w.clone());
                    },
                    StatefulWidgetType::Button(w) => {
                        w.clone().render(widget_area, buf, &mut w.clone());
                    },
                    StatefulWidgetType::Number(w) => {
                        w.clone().render(widget_area, buf, &mut w.clone());
                    },
                    _ => {}
                }
                y_offset += 1;
            }
        }
    }
}

fn map_number_widget_state(widget : &mut StatefulWidgetType) -> Option<NumberInputState> {
    match &widget {
        StatefulWidgetType::Number(state) => {
            Some(state.clone())
        },
        _ => {
            None
        }
    }
}