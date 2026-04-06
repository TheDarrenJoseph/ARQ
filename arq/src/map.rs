use std::collections::HashMap;

use room::Room;

use crate::map::objects::container::Container;
use crate::map::position::{Area, Position};
use crate::map::tile::{TileDetails, TileType};

pub mod objects;
pub mod map_generator;
pub mod position;
pub mod room;
pub mod tile;
pub mod map_view_areas;

#[derive(Debug, Clone)]
pub struct Map {
    pub area : Area,
    pub tiles : Tiles,
    pub rooms : Vec<Room>,
    // For containers not belonging to a room (loot containers for example)
    pub containers : HashMap<Position, Container>
}

#[derive(Debug, Clone)]
pub struct Tiles {
    pub tiles : Vec<Vec<TileDetails>>
}

impl Tiles {
    pub fn get_tile(&self, position: Position) -> Option<TileDetails> {
        match self.tiles.get(position.y as usize) {
            Some (row) => {
                match row.get(position.x as usize) {
                    Some(details) => {
                        return Some(details.clone());
                    },
                    None => {
                        return None
                    }
                }
            },
            None => {
                return None
            }
        }
    }

    pub fn set_tile(&mut self, position: Position, tile: TileDetails) {
        let x = position.x as usize;
        let y = position.y as usize;
        self.tiles[y][x] = tile
    }

}

impl Map {

    pub fn in_bounds(&self, x: usize, y: usize) -> bool {
        if let Some(row) = self.tiles.tiles.get(y) {
            if let Some(_) = row.get(x) {
                return true
            }
        }
        false
    }

    pub fn position_in_bounds(&self, position: Position) -> bool {
       self.in_bounds(position.x as usize, position.y as usize)
    }

    pub fn get_rooms(&self) -> Vec<Room> {
        return self.rooms.clone();
    }


    pub fn get_container(&self, position: Position) -> Option<&Container> {
        self.containers.get(&position)
    }

    pub fn get_container_at_position_mut(&mut self, position: Position) -> Option<&mut Container> {
        self.containers.get_mut(&position)
    }

    pub fn replace_container(&mut self, pos: Position, new: Container) {
        if let Some(old) = self.find_containers_mut(pos).iter_mut().find(|c| {
            return c.id_equals(&new);
        }) {
            log::info!("Replacing old container: {} with new: {}", old.get_self_item().get_name(), new.get_self_item().get_name());
            **old = new;
        }
    }

    pub fn find_containers(&self, position: Position) -> Vec<&Container> {
        let mut containers : Vec<&Container> = Vec::new();
        if let Some(map_c) = self.containers.get(&position) {
            containers.push(map_c);
        }
        containers
    }

    pub fn find_containers_mut(&mut self, position: Position) -> Vec<&mut Container> {
        let mut containers : Vec<&mut Container> = Vec::new();
        if let Some(map_c) = self.containers.get_mut(&position) {
            containers.push(map_c);
        }
        containers
    }


    pub fn get_containers_mut(&mut self) -> &mut HashMap<Position, Container> {
        &mut self.containers
    }

    pub fn find_container(&self, target: &Container, pos: Position) -> Option<&Container> {
        for c in self.find_containers(pos) {
            if c.id_equals(&target) {
                return Some(c);
            } else if let Some(subcontainer) = c.find(target.get_self_item()) {
                return Some(subcontainer);
            }
        }

        None
    }

    pub fn find_container_mut(&mut self, target: &Container, pos: Position) -> Option<& mut Container> {
        for c in self.find_containers_mut(pos) {
            if c.id_equals(&target) {
                return Some(c);
            } else if let Some(subcontainer) = c.find_mut(target.get_self_item()) {
               return Some(subcontainer);
            }
        }

        None
    }

    pub fn is_paveable(&self, position: Position) -> bool {
        match self.tiles.get_tile(position) {
            Some(tile) => {
                // All traversable tile types are paveable, including NoTile
                if tile.tile_type == TileType::NoTile {
                    true
                } else {
                    tile.traversable
                }
            }, None => {
                false
            }
        }
    }

    pub fn is_traversable(&self, position: Position) -> bool {
        match self.tiles.get_tile(position) {
            Some(tile) => {
                tile.traversable
            }, None => {
                false
            }
        }
    }

    pub fn get_neighbors(&self, position: Position) -> Vec<Position> {

        let mut results = Vec::new();
        let area = self.area;
        if area.contains_position(position) {
            let neighbors = position.get_neighbors();
            for n in neighbors {
                if area.contains_position(n) {
                    results.push(n);
                }
            }

        }
        results
    }
}

#[cfg(test)]
mod tests {
    use std::collections::HashMap;

    use crate::map::position::{build_square_area, Position};
    use crate::map::room::build_room;
    use crate::map::tile::TileType;
    use crate::map::Tiles;

    #[test]
    fn test_build_map() {
        let tile_library = crate::map::tile::build_library();
        assert_eq!(9, tile_library.len());

        let rom = tile_library[&TileType::Room].clone();
        let wall = tile_library[&TileType::Wall].clone();

        let room_pos = Position { x: 0, y: 0 };
        let room_area = build_square_area(room_pos, 3);
        let doors = Vec::new();

        let room = build_room(room_area, doors);

        let mut rooms = Vec::new();
        rooms.push(room);

        let map_pos = Position { x: 0, y: 0 };
        let map_area = build_square_area(map_pos, 3);
        let map = crate::map::Map {
            area: map_area,
            tiles : Tiles {tiles: vec![
                vec![ wall.clone(), wall.clone(), wall.clone() ],
                vec![ wall.clone(), rom.clone(), wall.clone() ],
                vec![ wall.clone(), wall.clone(), wall.clone() ],
        ]},
            rooms,
            containers: HashMap::new()
        };

        assert_eq!(3, map.tiles.tiles.len());
        assert_eq!(3, map.tiles.tiles[0].len());
        assert_eq!(3, map.tiles.tiles[1].len());
        assert_eq!(3, map.tiles.tiles[2].len());

    }

    #[test]
    fn test_adjust_map() {
        let tile_library = crate::map::tile::build_library();
        assert_eq!(9, tile_library.len());

        let wall = tile_library[&TileType::Wall].clone();

        let room_pos = Position { x: 0, y: 0 };
        let room_area = build_square_area(room_pos, 3);
        let room = build_room(room_area,  Vec::new());

        let mut rooms = Vec::new();
        rooms.push(room);

        let map_pos = Position { x: 0, y: 0 };
        let map_area = build_square_area(map_pos, 3);
        let mut map = crate::map::Map {
            area: map_area,
            tiles : Tiles {tiles: vec![
                vec![ wall.clone(),  wall.clone(),  wall.clone()],
            ]},
            rooms,
            containers: HashMap::new()
        };

        assert_eq!(1, map.tiles.tiles.len());

        // WHEN we push an item to the first row
        map.tiles.tiles[0].push(wall.clone());
        // THEN we expect it to go from 3 to 4 items long
        assert_eq!(4, map.tiles.tiles[0].len());

        // THEN we expect it to be available at 0,1
        assert_eq!(crate::map::tile::TileType::Wall, map.tiles.tiles[0][1].tile_type);

        // AND WHEN we push an new row to the map
        map.tiles.tiles.push(vec![wall.clone()]);
        // THEN we expect the length to increase
        assert_eq!(1, map.tiles.tiles[1].len());
        // AND the new tile to be available at 1,0
        assert_eq!(crate::map::tile::TileType::Wall, map.tiles.tiles[1][0].tile_type);
    }
}
