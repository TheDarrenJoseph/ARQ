use ratatui::prelude::Widget;
use crate::engine::event::ui::AppEventType::CombatTurnChoice;
use ratatui::buffer::Buffer;
use ratatui::layout::{Alignment, Rect};
use ratatui::prelude::{Line, Modifier, Span, StatefulWidget, Style, Text};
use ratatui::widgets::{Block, Borders, Paragraph};
use termion::event::Key;
use crate::character::battle::Battle;
use crate::character::equipment::{Equipment, EquipmentSlot, WeaponSlot};
use crate::engine::combat::CombatTurnChoiceEventType;
use crate::engine::command::combat_command::CombatCallbackData;
use crate::engine::event::event::UIEventHandler;
use crate::engine::event::ui::UIEvent;
use crate::engine::level::Level;
use crate::map::map_view_areas::MapViewAreas;
use crate::map::position::{build_rectangular_area, Area, Position};
use crate::option_list_selection::{MappedOption, OptionListSelection};
use crate::ui::ui_areas::{BorderedArea, UIAreas, UI_AREA_NAME_CONSOLE, UI_AREA_NAME_MAIN, UI_AREA_NAME_MINIMAP};
use crate::ui::ui_layout::LayoutType;
use crate::widget::stateful::container_widget::ContainerWidgetData;
use crate::widget::stateful::map_widget::MapWidget;

#[derive(Clone)]
#[derive(Debug)]
pub struct CombatWidget {
}

#[derive(Clone)]
#[derive(Debug)]
pub struct CombatWidgetData {
    pub battle: Battle,
    pub selection: OptionListSelection<CombatTurnChoiceEventType>,
    pub ui_areas: UIAreas,
    pub bordered_main_area : BorderedArea
}

pub struct ConsoleWidgets<'a> {
    paragraphs : Vec<(Paragraph<'a>, Rect)>,
    window : (Block<'a>, Rect)
}

impl CombatWidgetData {
    fn list_equipment(&self, equipment: Equipment) -> Paragraph<'static> {
        let mut spans = vec![];
        for slot in equipment.get_slots() {
            let item_result = slot.1;
            if let Some(item) = item_result {
                spans.push(Line::from(item.get_name().clone()))
            }
        }
        Paragraph::new(spans)
    }

    fn build_console_widgets(&self, ui_areas : &UIAreas) -> ConsoleWidgets<'_> {
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

fn build_options(equipment: Equipment) -> Vec<MappedOption<CombatTurnChoiceEventType>> {
    let mut choices = Vec::new();

    let slots = equipment.get_slots();
    if slots.contains_key(&EquipmentSlot::PRIMARY) {
        choices.push(MappedOption { mapped: CombatTurnChoiceEventType::ATTACK(WeaponSlot::PRIMARY), name: String::from("Attack (Primary)"), size: 16});
    }

    if slots.contains_key(&EquipmentSlot::SECONDARY) {
        choices.push(MappedOption {  mapped: CombatTurnChoiceEventType::ATTACK(WeaponSlot::SECONDARY), name: String::from("Attack (Secondary)"), size: 18});
    }

    choices.push(MappedOption { mapped: CombatTurnChoiceEventType::FLEE, name: String::from("Flee"), size: 4});

    return choices;
}


impl StatefulWidget for CombatWidget {
    type State = CombatWidgetData;

    fn render(mut self, _: Rect, buf: &mut Buffer, data: &mut CombatWidgetData) {
        let mut characters = &mut data.battle.characters;
        let player = characters.get_player_mut().unwrap();
        let player_equipment = player.get_equipment_mut().clone();

        let mut selection = data.selection.clone();
        if selection.options.len() == 0 {
            selection = OptionListSelection { options: build_options(player_equipment.clone()), index: 0 };
        }

        let player_name = player.get_name().clone();
        let _player_equipment_slots = player_equipment.get_slots();

        let enemy = characters.get_npcs_mut().first_mut().unwrap();
        let enemy_equipment = enemy.get_equipment_mut().clone();
        let enemy_name = enemy.get_name();

        let bordered_main_area = data.bordered_main_area.clone();
        let title = String::from(format!("{:─^width$}", "COMBAT───", width = bordered_main_area.outer.width as usize));
        let title_span = Span::from(title);

        // TODO area handling
        let main_window_block = Block::default()
            .title(title_span)
            .borders(Borders::ALL);

        main_window_block.render(bordered_main_area.outer.to_rect(), buf);

        // Split the main window into 2 columns / sides
        let side_width = (bordered_main_area.outer.width - 2) / 2;
        let side_height= bordered_main_area.outer.height - 2;

        // Start inside the border (+1)
        let main_area_inner_start_position = bordered_main_area.inner.start_position;
        let left_side_area = build_rectangular_area(main_area_inner_start_position, side_width, side_height);
        let left_side_block = Block::default()
            .title(Span::styled(player_name, Style::default().add_modifier(Modifier::UNDERLINED)))
            .borders(Borders::RIGHT);
        left_side_block.render(left_side_area.to_rect(), buf);


        // Player equipment area is the area within the left side window, adjust to fit
        let mut player_equipment_area = left_side_area.to_rect().clone();
        player_equipment_area.y += 1;
        player_equipment_area.height -= 1;
        let player_equipment_list = data.list_equipment(player_equipment);
        player_equipment_list.render(player_equipment_area, buf);

        let right_side_start_position = Position { x: left_side_area.end_position.x, y: main_area_inner_start_position.y };
        let right_side_area = build_rectangular_area(right_side_start_position, side_width, side_height);
        let right_side_block = Block::default()
            .title(Span::styled(enemy_name, Style::default().add_modifier(Modifier::UNDERLINED)))
            .borders(Borders::LEFT);
        right_side_block.render(right_side_area.to_rect(), buf);

        // Player equipment area is the area within the left side window, adjust to fit
        let mut enemy_equipment_area = right_side_area.to_rect().clone();
        enemy_equipment_area.x += 1;
        enemy_equipment_area.y += 1;
        enemy_equipment_area.height -= 1;
        let enemy_equipment_list = data.list_equipment(enemy_equipment);
        enemy_equipment_list.render(enemy_equipment_area, buf);

        // Build widget / area tuples
        let console_widgets = data.build_console_widgets(&data.ui_areas);
        let console_block = console_widgets.window.0;
        let console_area = console_widgets.window.1;
        console_block.render(console_area, buf);
        // Unpack these tuples and render them
        for paragraph_area in console_widgets.paragraphs {
            paragraph_area.0.render(paragraph_area.1, buf);
        }

        let minimap_area = data.ui_areas.get_area(UI_AREA_NAME_MINIMAP).unwrap();
        let bordered_minimap_area = BorderedArea::from_area(minimap_area.area).unwrap();

        // TODO pass through current minimap data / render
        // let mut player_global_pos = self.level.characters.get_player_mut().unwrap().get_global_position();
        // Offset the the player pos by half of the minimap size to center it
        // let half_minimap_width = ( bordered_minimap_area.inner.width / 2) as i32;
        // let half_minimap_height = ( bordered_minimap_area.inner.height / 2) as i32 ;
        //
        // // The entire map area
        // let map_area = self.level.map.as_ref().unwrap().area.clone();
        // // The view area is the position/area of the minimap on the screen
        // let map_view_area = bordered_minimap_area.inner;
        // // The display area is the part of the map area we're actually displaying
        // let minimap_map_target_pos = player_global_pos.offset(-half_minimap_width, -half_minimap_height);
        // let map_display_area = Area::new(minimap_map_target_pos, bordered_minimap_area.inner.width, bordered_minimap_area.inner.height);
        // let map_view_areas = MapViewAreas { map_area, map_view_area: map_view_area, map_display_area };
        //
        // let minimap_block = Block::default().borders(Borders::ALL);
        // frame.render_widget(minimap_block, bordered_minimap_area.outer.to_rect());
        //
        // let map_widget = MapWidget::new(map_view_areas);
        // let dummy_area = Area::new(Position::new(0,0),0,0);
        // frame.render_stateful_widget(map_widget, dummy_area.to_rect(), &mut self.level);
    }
}

impl UIEventHandler for CombatWidgetData {
    async fn handle_event(&mut self, event: UIEvent) {
        log::debug!("[container_choice_widget] Handling event: {:?}", event.name());
        let mut messages : Vec<String> = Vec::new();
        match event {
            UIEvent::Termion(termion_event) => {
                match termion_event {
                    termion::event::Event::Key(key) => {
                        match key {
                            // These are key specific as they are not attached to events and thus are purely UI controls for the widget
                            // These may move into some bindings in future to make them dynamic instead of hardcoded
                            Key::Up => {
                                self.selection.move_up();
                            },
                            Key::Down => {
                                self.selection.move_down();
                            },
                            crate::global_flags::ENTER_KEY => {
                                let selection = &self.selection;
                                let chosen_option = selection.get_chosen();

                                // TODO convert
                                //let _data = CombatCallbackData { choice: CombatTurnChoiceEventType::ATTACK(WeaponSlot::PRIMARY), result: None };

                                // TODO send event
                                // return Ok(self.build_input_not_done_result());
                            },
                            Key::Esc => {
                                // TODO escape
                            },
                            _ => {
                                // No-op
                            }
                        }
                    },
                    _ => {}
                }
            },
            UIEvent::AppEvent(CombatTurnChoice(CombatTurnChoiceEventType::ATTACK(weapon_slot))) => {
                messages.push(String::from("You attempt attack..."));
                // TODO
            }
            UIEvent::AppEvent(CombatTurnChoice(CombatTurnChoiceEventType::FLEE)) => {
                messages.push(String::from("You attempt to run away..."));
                // TODO
            },
            _ => {
                // No-op
            }
        }

        // TODO return / set message
        // result_data.result = Some(CombatResult { messages });
        // Some(result_data)
    }
}

