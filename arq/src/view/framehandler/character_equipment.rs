use ratatui::style::Style;
use ratatui::text::{Line, Span};
use ratatui::widgets::Paragraph;

use crate::character::equipment::{all_equipment_slots, Equipment};
use crate::view::framehandler::{FrameData, FrameHandler};

// Deprecated, TODO Move to
pub struct CharacterEquipmentFrameHandler {
}

impl CharacterEquipmentFrameHandler {
    pub const fn new() -> CharacterEquipmentFrameHandler {
        CharacterEquipmentFrameHandler {}
    }
}

impl FrameHandler<Equipment> for CharacterEquipmentFrameHandler {
    fn handle_frame(&mut self, frame: &mut ratatui::Frame, data: FrameData<Equipment>) {

        // TODO potentially use ui_areas in future?
        //let main_area = data.get_ui_areas().get_area(UI_AREA_NAME_MAIN).expect("Main UIArea should be available.");
        let frame_area = data.frame_area;

        let title_lengths : Vec<usize> = all_equipment_slots().iter().map(|s| s.to_string().len()).collect();
        // +1 to ensure good formatting
        let max_title_length : usize = *title_lengths.iter().max().unwrap() + 1;

        // Part one, render the left-hand side titles for each slot
        let mut title_spans_list = Vec::new();
        for slot in all_equipment_slots() {
            let title = Span::from(slot.to_string());
            let spans = Line::from(title);
            title_spans_list.push(spans);
        }

        let paragraph = Paragraph::new(title_spans_list)
            .style(Style::default())
            .alignment(ratatui::layout::Alignment::Left);
        frame.render_widget(paragraph, frame_area.to_rect());

        // Part two, render the equipment names offset to the right
        let equipment = data.data;
        let mut name_spans_list = Vec::new();
        for slot in all_equipment_slots() {
            let name = if let Some(equipped_item) = equipment.get_item(slot) {
                Span::from(equipped_item.get_name())
            } else {
                Span::from("Empty")
            };
            let spans = Line::from(name);
            name_spans_list.push(spans);
        }

        // Render a paragraph for the equipment names
        let mut names_area = frame_area.to_rect();
        names_area.x = names_area.x + max_title_length as u16;
        names_area.width -= max_title_length as u16;
        let paragraph = Paragraph::new(name_spans_list)
            .style(Style::default())
            .alignment(ratatui::layout::Alignment::Left);
        frame.render_widget(paragraph, names_area);
    }
}


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