use ratatui::buffer::{Buffer, Cell};
use ratatui::layout::Rect;
use ratatui::style::Style;
use ratatui::widgets::StatefulWidget;

use crate::engine::level::Level;
use crate::map::map_view_areas::MapViewAreas;
use crate::map::objects::container::Container;
use crate::map::position::Position;
use crate::map::Map;
use crate::view::util::cell_builder::CellBuilder;

#[derive(Clone)]
#[derive(Debug)]
pub struct MapWidget {
    pub map_view_areas : MapViewAreas // Possibly reduced display area
}

impl MapWidget {
    pub(crate) const fn new(map_view_areas: MapViewAreas) -> MapWidget {
        MapWidget { map_view_areas }
    }
    
    fn find_container<'a>(&'a self, map: &'a Map, global_position: Position) -> Option<(Position, &'a Container)> {
        let containers = &map.containers;

        if containers.contains_key(&global_position) {
            let container = containers.get(&global_position).unwrap();
            let correct_type = container.is_true_container();
            let has_content = container.get_total_count() > 0;
            if correct_type && has_content {
                return Some((global_position.clone(), container));
            }
        }
        None
    }

    fn build_cell_for_position(&mut self, level: &Level, global_position: Position, _cell_target: &mut Cell) -> Cell {
        let characters = &level.characters;
        let player_mut = characters.get_player().unwrap();

        if let Some(map) = &level.map {
            let tiles = &map.tiles;
            let tile_result = tiles.get_tile(global_position);

            // Check for the player
            if global_position == player_mut.get_global_position() {
                return CellBuilder::from_character(player_mut);
            }

            let characters = &level.characters;
            if let Some(npc) = characters.get_npcs().iter().find(|npc| npc.get_global_position().equals(global_position)).cloned() {
                return CellBuilder::from_character(&npc);
            }

            if let Some(container_entry) = self.find_container(map, global_position) {
                let container = container_entry.1;
                return CellBuilder::from_container(container);
            }

            // Otherwise, just draw the tile
            if let Some(tile) = tile_result {
                return CellBuilder::from_tile(&tile);
            }
        }

        // Draw out of range cell
        return CellBuilder::for_blank()
    }
}


impl StatefulWidget for MapWidget {
    type State = Level;

    fn render(mut self, _area: Rect, buf: &mut Buffer, level: &mut Self::State) {
        let map_view_area = self.map_view_areas.map_view_area;
        // Local positions should start at 0,0 to size_x-1, size_y-1
        for x in 0..map_view_area.width {
            for y in 0..map_view_area.height {
                let local_position = Position::new(x,y);
                let global_position = self.map_view_areas.local_to_global(local_position).unwrap();
                let position_in_display_area = self.map_view_areas.is_position_in_map_display_area(global_position);
                if position_in_display_area {
                    let screen_position = map_view_area.get_position(x,y);
                    let mut cell = buf.get_mut(screen_position.x, screen_position.y);
                    let new_cell = self.build_cell_for_position(level, global_position,&mut cell);
                    cell.set_symbol(new_cell.symbol());
                    cell.set_bg(new_cell.bg);
                    cell.set_fg(new_cell.fg);
                    cell.set_style(Style::default().add_modifier(new_cell.modifier));
                }
            }
        }
    }
}
