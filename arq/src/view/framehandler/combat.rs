use ratatui::layout::{Alignment, Rect};
use ratatui::style::{Modifier, Style};
use ratatui::text::{Line, Span, Text};
use ratatui::widgets::{Block, Borders, Paragraph};
use ratatui::Frame;

use crate::character::battle::Battle;
use crate::character::equipment::{Equipment, EquipmentSlot, WeaponSlot};
use crate::engine::combat::CombatTurnChoice;
use crate::engine::level::Level;
use crate::map::map_view_areas::MapViewAreas;
use crate::map::position::{build_rectangular_area, Area, Position};
use crate::option_list_selection::{MappedOption, OptionListSelection};
use crate::ui::ui_areas::{BorderedArea, UIAreas, UI_AREA_NAME_CONSOLE, UI_AREA_NAME_MAIN, UI_AREA_NAME_MINIMAP};
use crate::view::framehandler::{FrameData, FrameHandler};
use crate::widget::stateful::map_widget::MapWidget;

pub struct CombatFrameHandler {
    pub selection: OptionListSelection<CombatTurnChoice>,
    pub level: Level
}

pub struct ConsoleWidgets<'a> {
    paragraphs : Vec<(Paragraph<'a>, Rect)>,
    window : (Block<'a>, Rect)
}

impl CombatFrameHandler {
    pub fn new(level: Level) -> CombatFrameHandler {
        CombatFrameHandler { selection: OptionListSelection::new(), level }
    }

    fn build_options(&self, equipment: Equipment) -> Vec<MappedOption<CombatTurnChoice>> {
        let mut choices = Vec::new();

        let slots = equipment.get_slots();
        if slots.contains_key(&EquipmentSlot::PRIMARY) {
            choices.push(MappedOption { mapped: CombatTurnChoice::ATTACK(WeaponSlot::PRIMARY), name: String::from("Attack (Primary)"), size: 16});
        }

        if slots.contains_key(&EquipmentSlot::SECONDARY) {
            choices.push(MappedOption {  mapped: CombatTurnChoice::ATTACK(WeaponSlot::SECONDARY), name: String::from("Attack (Secondary)"), size: 18});
        }

        choices.push(MappedOption { mapped: CombatTurnChoice::FLEE, name: String::from("Flee"), size: 4});

        return choices;
    }

    fn build_console_widgets(&self, ui_areas : &UIAreas) -> ConsoleWidgets {
        let console_area = ui_areas.get_area(UI_AREA_NAME_CONSOLE).unwrap();

        let console_area_bordered = BorderedArea::from_area(console_area.area.clone()).unwrap();

        let console_window_block = Block::default()
            .borders(Borders::ALL);

        let highlighted_option = self.selection.index;
        let mut i = 0;

        let mut paragraphs: Vec<(Paragraph, Rect)> = Vec::new();
        let largest_option_length = self.selection.options.iter().max_by_key(|o| o.size).unwrap().size.clone() as u16;
        for option in &self.selection.options {
            let mut style = Style::default();
            if i == highlighted_option {
                style = style.add_modifier(Modifier::REVERSED);
            }

            let paragraph = Paragraph::new(Text::from(option.name.clone()))
                .style(style)
                .alignment(Alignment::Left);

            let offset_y = i + 1;
            let text_area = Rect::new(console_area_bordered.inner.start_position.x.clone() + 1, console_area_bordered.inner.start_position.y.clone() + offset_y, largest_option_length, 1);

            paragraphs.push((paragraph, text_area));
            i += 1;
        }

        return  ConsoleWidgets { window: (console_window_block, console_area_bordered.outer.to_rect()), paragraphs };
    }
}

fn list_equipment(equipment: Equipment) -> Paragraph<'static> {
    let mut spans = vec![];
    for slot in equipment.get_slots() {
        let item = slot.1;
        spans.push(Line::from(item.get_name().clone()))
    }
    Paragraph::new(spans)
}

impl FrameHandler<Battle> for CombatFrameHandler {
    fn handle_frame(&mut self, frame: &mut Frame, data: FrameData<Battle>) {
        let battle = data.data;
        let mut characters = battle.characters;
        let player = characters.get_player_mut().unwrap();
        let player_equipment = player.get_equipment_mut().clone();

        if self.selection.options.len() == 0 {
            self.selection = OptionListSelection { options: self.build_options(player_equipment.clone()), index: 0 };
        }

        let player_name = player.get_name().clone();
        let _player_equipment_slots = player_equipment.get_slots();

        let enemy = characters.get_npcs_mut().first_mut().unwrap();
        let enemy_equipment = enemy.get_equipment_mut().clone();
        let enemy_name = enemy.get_name();

        let ui_areas = data.ui_areas;
        let main_area = ui_areas.get_area(UI_AREA_NAME_MAIN).unwrap().area;
        let bordered_main_area = BorderedArea::from_area(main_area).unwrap();

        let title = String::from(format!("{:─^width$}", "COMBAT───", width = bordered_main_area.outer.width as usize));
        let title_span = Span::from(title);

        // TODO area handling
        let main_window_block = Block::default()
            .title(title_span)
            .borders(Borders::ALL);
        frame.render_widget(main_window_block, bordered_main_area.outer.to_rect());

        // Split the main window into 2 columns / sides
        let side_width = (bordered_main_area.outer.width - 2) / 2;
        let side_height= bordered_main_area.outer.height - 2;

        // Start inside the border (+1)
        let main_area_inner_start_position = bordered_main_area.inner.start_position;
        let left_side_area = build_rectangular_area(main_area_inner_start_position, side_width, side_height);
        let left_side_block = Block::default()
            .title(Span::styled(player_name, Style::default().add_modifier(Modifier::UNDERLINED)))
            .borders(Borders::RIGHT);
        frame.render_widget(left_side_block, left_side_area.to_rect());

        // Player equipment area is the area within the left side window, adjust to fit
        let mut player_equipment_area = left_side_area.to_rect().clone();
        player_equipment_area.y += 1;
        player_equipment_area.height -= 1;
        let player_equipment_list = crate::view::framehandler::combat::list_equipment(player_equipment);
        frame.render_widget(player_equipment_list, player_equipment_area);

        let right_side_start_position = Position { x: left_side_area.end_position.x, y: main_area_inner_start_position.y };
        let right_side_area = build_rectangular_area(right_side_start_position, side_width, side_height);
        let right_side_block = Block::default()
            .title(Span::styled(enemy_name, Style::default().add_modifier(Modifier::UNDERLINED)))
            .borders(Borders::LEFT);
        frame.render_widget(right_side_block, right_side_area.to_rect());

        // Player equipment area is the area within the left side window, adjust to fit
        let mut enemy_equipment_area = right_side_area.to_rect().clone();
        enemy_equipment_area.x += 1;
        enemy_equipment_area.y += 1;
        enemy_equipment_area.height -= 1;
        let enemy_equipment_list = crate::view::framehandler::combat::list_equipment(enemy_equipment);
        frame.render_widget(enemy_equipment_list, enemy_equipment_area);


        // Build widget / area tuples
        let console_widgets = self.build_console_widgets(&ui_areas);
        let console_block = console_widgets.window.0;
        let console_area = console_widgets.window.1;
        frame.render_widget(console_block, console_area);
        // Unpack these tuples and render them
        for paragraph_area in console_widgets.paragraphs {
            frame.render_widget(paragraph_area.0, paragraph_area.1);
        }


        let minimap_area = ui_areas.get_area(UI_AREA_NAME_MINIMAP).unwrap();
        let bordered_minimap_area = BorderedArea::from_area(minimap_area.area).unwrap();

        let mut player_global_pos = self.level.characters.get_player_mut().unwrap().get_global_position();

        // Offset the the player pos by half of the minimap size to center it
        let half_minimap_width = ( bordered_minimap_area.inner.width / 2) as i32;
        let half_minimap_height = ( bordered_minimap_area.inner.height / 2) as i32 ;

        // The entire map area
        let map_area = self.level.map.as_ref().unwrap().area.clone();
        // The view area is the position/area of the minimap on the screen
        let map_view_area = bordered_minimap_area.inner;
        // The display area is the part of the map area we're actually displaying
        let minimap_map_target_pos = player_global_pos.offset(-half_minimap_width, -half_minimap_height);
        let map_display_area = Area::new(minimap_map_target_pos, bordered_minimap_area.inner.width, bordered_minimap_area.inner.height);
        let map_view_areas = MapViewAreas { map_area, map_view_area: map_view_area, map_display_area };

        let minimap_block = Block::default().borders(Borders::ALL);
        frame.render_widget(minimap_block, bordered_minimap_area.outer.to_rect());

        let map_widget = MapWidget::new(map_view_areas);
        let dummy_area = Area::new(Position::new(0,0),0,0);
        frame.render_stateful_widget(map_widget, dummy_area.to_rect(), &mut self.level);
    }
}