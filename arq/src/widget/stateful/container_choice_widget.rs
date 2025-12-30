use ratatui::prelude::Widget;
use ratatui::buffer::Buffer;
use ratatui::layout::Rect;
use ratatui::prelude::{Modifier, StatefulWidget, Style};
use ratatui::widgets::{Block, Borders};
use termion::event::Key;
use tokio::sync::mpsc;
use tokio::sync::mpsc::UnboundedSender;
use uuid::Uuid;
use crate::engine::command::open_command::OpenedContainerEventData::MoveItemsResult;
use crate::engine::command::open_command::OpenedContainerEventType::MoveItemsToContainerChoice;
use crate::item_list_selection::{ItemListSelection, ListSelection};
use crate::map::objects::container::Container;
use crate::map::objects::items::Item;
use crate::map::position::Area;
use crate::ui::event::AppEventType::OpenedContainerEvent;
use crate::ui::event::Event;
use crate::view::framehandler::container::{ContainerTarget, MoveItemsRequest};
use crate::view::framehandler::util::tabling::Column;
use crate::widget::build_buffer;
use crate::widget::standard::usage_line::UsageCommand;
use crate::widget::stateful::console_input_widget::ConsoleInputState;

#[derive(Debug, Clone)]
pub struct ContainerChoiceWidget {
    pub(crate) columns : Vec<Column>
}

impl ContainerChoiceWidget {
    pub(crate) fn new() -> ContainerChoiceWidget {
        ContainerChoiceWidget {
            columns: vec![
                Column {name : "NAME".to_string(), size: 30}
            ]
        }
    }
}

#[derive(Debug, Clone)]
pub struct ContainerChoiceWidgetData {
    pub item_list_selection : ItemListSelection,
    pub ui_area: Area,
    pub usage_commands: Vec<UsageCommand>,
    pub event_sender: mpsc::UnboundedSender<Event>,
}

impl ContainerChoiceWidgetData {
    pub fn new(
        choices: Vec<Container>,
        ui_area: Area,
        line_count: i32,
        sender: UnboundedSender<Event>
    ) -> ContainerChoiceWidgetData {
        let choice_items = convert_to_item_list(choices.clone());
        let item_list_selection =  ItemListSelection::new(choice_items, line_count.into());
        ContainerChoiceWidgetData {
            item_list_selection,
            ui_area,
            usage_commands: vec![
                UsageCommand::new(Key::Backspace, String::from("Select")),
            ],
            event_sender: sender,
        }
    }

    pub async fn handle_event(&mut self, event: Event) {
        log::debug!("Handling event: {:?}", event);
        match event {
            Event::Termion(termion_event) => {
                match termion_event {
                    termion::event::Event::Key(key) => {
                        match key {
                            // These are key specific as they are not attached to events and thus are purely UI controls for the widget
                            // These may move into some bindings in future to make them dynamic instead of hardcoded
                            Key::Up => {
                                self.item_list_selection.move_up();
                            },
                            Key::Down => {
                                self.item_list_selection.move_down();
                            },
                            Key::PageUp => {
                                self.item_list_selection.page_up();
                            },
                            Key::PageDown => {
                                self.item_list_selection.page_down();
                            },
                            Key::Backspace | Key::Char('\n') => {
                                // This should:
                                // 1. select an item
                                // 2. fire the choice event
                                // 3. close the widget
                                let selected_item = self.item_list_selection.get_focused_item().unwrap();
                                let container_target = ContainerTarget {
                                    target_container_id: selected_item.get_id()
                                };
                                self.event_sender.send(
                                    Event::AppEvent(
                                        OpenedContainerEvent(
                                            MoveItemsToContainerChoice,
                                            Some(crate::engine::command::open_command::OpenedContainerEventData::ContainerChoice(container_target))
                                        )
                                    )
                                ).expect("Failed to send event");
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

impl StatefulWidget for ContainerChoiceWidget {
    type State = ContainerChoiceWidgetData;

    fn render(self, area: Rect, buf: &mut Buffer, _state: &mut Self::State) {
        let data = _state;

        let ui_area = data.ui_area;

        let window_block = Block::default()
            .borders(Borders::ALL);

        let window_rect = ui_area.to_rect();
        let line_count = data.item_list_selection.page_line_count;

        window_block.render(window_rect, buf);

        //let containers = &self.choices;
        //         let frame_size = data.get_frame_area().clone();
        //         let window_area = Rect::new(frame_size.x.clone(), frame_size.y.clone(), frame_size.width.clone(), frame_size.height.clone());
        //         let inventory_item_lines = window_area.height - 3;
        //
        //         self.item_list_selection.page_line_count = inventory_item_lines as i32;
        //         frame.render_widget(window_block, window_area);
        //
        //         let headings = build_headings(self.columns.clone());
        //         let headings_area = Rect::new(frame_size.x.clone() + 1, frame_size.y.clone() + 1, frame_size.width.clone() - 4, 2);
        //         frame.render_widget(headings, headings_area);
        //
        //         // -3 for the heading and 2  borders
        //         let mut line_index = 0;
        //         let _start_index= self.item_list_selection.get_start_index();
        //         let start_index = 0;
        //         let end_of_page_representive_index = self.item_list_selection.get_end_of_page_index();
        //
        //         if !containers.is_empty() {
        //             let view_contents = &containers[start_index as usize..=end_of_page_representive_index as usize];
        //             for c in view_contents {
        //                 let item_index = start_index.clone() + line_index.clone();
        //                 let mut x_offset: u16 = frame_size.x.clone() as u16 + 1;
        //                 let y_offset: u16 = frame_size.y.clone() as u16 + 2 + line_index.clone() as u16;
        //                 let current_index = self.item_list_selection.is_focused(item_index);
        //                 let selected = self.item_list_selection.is_selected(item_index);
        //                 for column in &self.columns {
        //                     let text = build_column_text(column, c);
        //                     let mut column_text = build_paragraph(text);
        //                     if current_index.clone() && selected.clone() {
        //                         column_text = column_text.style(Style::default().fg(Color::Green).add_modifier(Modifier::REVERSED));
        //                     } else if current_index {
        //                         column_text = column_text.style(Style::default().add_modifier(Modifier::REVERSED));
        //                     } else if selected {
        //                         column_text = column_text.style(Style::default().fg(Color::Green));
        //                     }
        //
        //                     let column_length = column.size as i8;
        //                     let text_area = Rect::new(x_offset.clone(), y_offset.clone(), column_length.try_into().unwrap(), 1);
        //                     frame.render_widget(column_text.clone(), text_area);
        //                     x_offset += column_length as u16;
        //                 }
        //                 line_index += 1;
        //             }
        //
        //             //let usage_description = build_command_usage_descriptions(&self.commands);
        //             //let usage_text = build_paragraph(usage_description.clone());
        //             //let text_area = Rect::new(window_area.x.clone() + 1, window_area.y.clone() + window_area.height.clone() - 1, usage_description.len().try_into().unwrap(), 1);
        //             //frame.render_widget(usage_text.clone(), text_area);
        //
        //             // From right hand to left hand side draw the info text
        //             let page_count = build_page_count(&self.item_list_selection, window_area.clone());
        //             frame.render_widget(page_count.0, page_count.1);

    }
}


fn convert_to_item_list(choices : Vec<Container>) -> Vec<Item> {
    let mut items = Vec::new();
    for c in choices {
        items.push(c.get_self_item().clone());
    }
    items
}