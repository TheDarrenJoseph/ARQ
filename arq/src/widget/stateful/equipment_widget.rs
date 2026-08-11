use std::ptr::eq;
use ratatui::prelude::Widget;
use ratatui::buffer::Buffer;
use ratatui::layout::Rect;
use ratatui::prelude::StatefulWidget;
use ratatui::widgets::{Block, Borders};
use termion::event::Key;
use crate::character::equipment::{Equipment, EquipmentSlotItem};
use crate::engine::event::event::UIEventHandler;
use crate::engine::event::ui::UIEvent;
use crate::map::position::Area;
use crate::ui::ui_areas::UIArea;
use crate::widget::stateful::container_choice_widget::{ContainerChoiceWidget, ContainerChoiceWidgetData};
use crate::widget::stateful::container_widget::ContainerWidgetData;
use crate::widget::stateful::dropdown_widget::{DropdownInputState, DropdownOption};

#[derive(Debug, Clone)]
pub struct EquipmentWidget {
}

impl EquipmentWidget {
    pub(crate) fn new() -> EquipmentWidget {
        EquipmentWidget {
        }
    }
}

#[derive(Debug, Clone)]
pub struct EquipmentWidgetData {
    pub main_ui_area: UIArea,
    pub selected_index: usize,
    pub equipment: Equipment
}

impl EquipmentWidgetData {
    pub(crate) fn new(main_ui_area: UIArea, equipment: Equipment) -> EquipmentWidgetData {
        EquipmentWidgetData {
            main_ui_area: main_ui_area,
            selected_index: 0,
            equipment
        }
    }

    pub fn move_selection_up(&mut self) {
        let slots_size = self.equipment.get_slots().len();
        if self.selected_index < slots_size - 1 {
            self.selected_index += 1;
        }
    }

    pub fn move_selection_down(&mut self) {
        if self.selected_index > 0 {
            self.selected_index -= 1;
        }
    }
}


impl UIEventHandler for EquipmentWidgetData {
    async fn handle_event(&mut self, event: UIEvent) {
        log::debug!("[container_widget] Handling event: {:?}", event.name());
        match event {
            UIEvent::Termion(termion_event) => {
                match termion_event {
                    termion::event::Event::Key(key) => {
                        match key {
                            // These are key specific as they are not attached to events and thus are purely UI controls for the widget
                            // These may move into some bindings in future to make them dynamic instead of hardcoded
                            Key::Up => {
                                self.move_selection_up();
                            },
                            Key::Down => {
                                self.move_selection_down();
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


impl StatefulWidget for EquipmentWidget {
    type State = EquipmentWidgetData;

    fn render(self, area: Rect, buf: &mut Buffer, data: &mut Self::State) {
        let main_area = data.main_ui_area.get_bordered_area();

        let window_block = Block::default()
            .borders(Borders::ALL)
            .title("Equipment");

        // Adjust the inner main window area to account for the tabs
        let window_start_x = main_area.inner.start_position.x;
        let window_start_y = main_area.inner.start_position.y + 1;
        let window_width = main_area.inner.width;
        let window_height = main_area.inner.height - 1;
        let window_area = Rect::new(window_start_x, window_start_y, window_width, window_height);

        window_block.render(window_area, buf);

        let mut dropdown_index = 0;
        for equipment in data.equipment.get_slots() {
            let slot_item = EquipmentSlotItem {
                slot: equipment.0.clone(),
                item: equipment.1.clone()
            };
            let dropdown_option = DropdownOption::new(slot_item);
            let dropdown = DropdownInputState::new(dropdown_option);
            let mut dropdown_state = dropdown.clone();

            let dropdown_start_x = main_area.inner.start_position.x + 1;
            let dropdown_start_y = main_area.inner.start_position.y + 2 + dropdown_index;
            let dropdown_width = main_area.inner.width - 1;
            let dropdown_height = main_area.inner.height - 2 + dropdown_index;
            let dropdown_area = Rect::new(dropdown_start_x, dropdown_start_y, dropdown_width, dropdown_height);
            dropdown.render(dropdown_area, buf, &mut dropdown_state);

            dropdown_index += 1;
        }

    }
}


// TODO convert old framehandler tests???
// #[cfg(test)]
// mod character_equipment_frame_handler_tests {
//     use std::collections::HashMap;
//
//     use ratatui::buffer::Buffer;
//     use uuid::Uuid;
//
//     use crate::character::equipment::Equipment;
//     use crate::character::equipment::EquipmentSlot::PRIMARY;
//     use crate::map::objects::container::Container;
//     use crate::map::objects::items::{Item, ItemForm, MaterialType, Weapon};
//     use crate::map::objects::weapon_builder::BladedWeaponType;
//     use crate::map::position::Area;
//     use crate::terminal::terminal_manager::init_test;
//     use crate::ui::ui_areas::UIAreas;
//     use crate::view::framehandler::character_equipment::CharacterEquipmentFrameHandler;
//     use crate::view::framehandler::{FrameData, FrameHandler};
//     use crate::view::MIN_RESOLUTION;
//
//     fn build_arming_sword_primary() -> Item {
//         Item::weapon(Uuid::new_v4(), "".to_owned(), ItemForm::BLADED(BladedWeaponType::ARMING), MaterialType::STEEL, 'X', 3.0, 50, Weapon { damage: 20 })
//     }
//
//     #[test]
//     fn test_handle_frame() {
//         // GIVEN a frame handler
//         let mut frame_handler = CharacterEquipmentFrameHandler::new();
//
//         // AND we have a test terminal manager using the minimum 80x24 resolution
//         let mut terminal_manager = init_test(MIN_RESOLUTION).unwrap();
//
//         // AND we have equipment with some items equipped
//         let mut equipment = Equipment::new();
//         let primary = build_arming_sword_primary();
//         let container = Container::wrap(primary);
//         equipment.equip(container, PRIMARY).expect("Primary weapon should be equipped successfully");
//
//         let ui_areas = UIAreas::new(HashMap::new());
//
//         // WHEN we call to draw the framehandler
//         terminal_manager.terminal.draw(|frame| {
//             let frame_data = FrameData { data: equipment.clone(), ui_areas, frame_area: Area::from_rect(frame.size()) };
//             frame_handler.handle_frame(frame, frame_data);
//         }).expect("Test Terminal should draw the frame successfully");
//
//         // THEN we expect the framehandler to draw the equipment details to the framebuffer
//         let expected = Buffer::with_lines(vec![
//             "HEAD      Empty                                                                 ",
//             "TORSO     Empty                                                                 ",
//             "LEGS      Empty                                                                 ",
//             "FEET      Empty                                                                 ",
//             "PRIMARY   Steel Sword                                                           ",
//             "SECONDARY Empty                                                                 ",
//             "                                                                                ",
//             "                                                                                ",
//             "                                                                                ",
//             "                                                                                ",
//             "                                                                                ",
//             "                                                                                ",
//             "                                                                                ",
//             "                                                                                ",
//             "                                                                                ",
//             "                                                                                ",
//             "                                                                                ",
//             "                                                                                ",
//             "                                                                                ",
//             "                                                                                ",
//             "                                                                                ",
//             "                                                                                ",
//             "                                                                                ",
//             "                                                                                ",
//         ]);
//         terminal_manager.terminal.backend().assert_buffer(&expected)
//
//     }
// }