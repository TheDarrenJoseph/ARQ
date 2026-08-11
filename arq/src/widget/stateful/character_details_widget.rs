use ratatui::prelude::Widget;
use ratatui::buffer::Buffer;
use ratatui::layout::Rect;
use ratatui::prelude::StatefulWidget;
use ratatui::widgets::{Block, Borders};
use termion::event::Key;
use crate::character::equipment::Equipment;
use crate::engine::event::event::UIEventHandler;
use crate::engine::event::ui::UIEvent;
use crate::map::position::Area;
use crate::ui::ui_areas::UIArea;
use crate::widget::stateful::container_choice_widget::{ContainerChoiceWidget, ContainerChoiceWidgetData};
use crate::widget::stateful::container_widget::ContainerWidgetData;

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
    pub main_ui_area: UIArea
}

impl CharacterDetailsWidgetData {
    pub(crate) fn new(main_ui_area: UIArea) -> CharacterDetailsWidgetData {
        CharacterDetailsWidgetData {
            main_ui_area: main_ui_area
        }
    }
}


impl UIEventHandler for CharacterDetailsWidgetData {
    async fn handle_event(&mut self, event: UIEvent) {
        log::debug!("[container_widget] Handling event: {:?}", event.name());
        match event {
            UIEvent::Termion(termion_event) => {
                match termion_event {
                    termion::event::Event::Key(key) => {
                        match key {
                            // These are key specific as they are not attached to events and thus are purely UI controls for the widget
                            // These may move into some bindings in future to make them dynamic instead of hardcoded
                            // Key::Up => {
                            //     self.move_selection_up();
                            // }
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
        let main_area = data.main_ui_area.get_bordered_area();

        let window_block = Block::default()
            .borders(Borders::ALL)
            .title("Character Details");

        // Adjust the inner main window area to account for the tabs
        let window_start_x = main_area.inner.start_position.x;
        let window_start_y = main_area.inner.start_position.y + 1;
        let window_width = main_area.inner.width;
        let window_height = main_area.inner.height - 1;
        let window_area = Rect::new(window_start_x, window_start_y, window_width, window_height);

        window_block.render(window_area, buf);
    }
}