use termion::event::Key;

use crate::ui::bindings::action_bindings::Action;
use crate::ui::bindings::input_bindings::{AllKeyBindings, KeyBindings};

pub struct InputHandler {
  current_action: Option<Action>,
  keybindings: AllKeyBindings,  
}

impl InputHandler {
    pub fn new(keybindings: AllKeyBindings) -> InputHandler {
        InputHandler {
            current_action: None,
            keybindings
        }
    }
    pub async fn handle_input(&mut self, key : Key) -> Option<Action> {
        let action_bindings = &self.keybindings.action_key_bindings;
        let action_input = action_bindings.get_input(key);

        if let Some(action) = action_input {
            return Some(action.clone())
        }
        None
    }
}

#[cfg(test)]
mod tests {
    use std::collections::HashMap;

    use termion::event::Key;

    use crate::character::builder::character_builder::{CharacterBuilder, CharacterPattern};

    use crate::engine::game_engine::*;
    use crate::engine::level::Levels;
    use crate::map::position::Position;
    use crate::map::position::{build_square_area, Area};
    use crate::map::tile::{build_library, TileDetails, TileType};
    use crate::map::{Map, Tiles};
    use crate::terminal::terminal_manager;
    use crate::test::utils::test_utils::build_test_levels;
    use crate::view::MIN_RESOLUTION;

    fn build_tiles(map_area: Area, tile : TileType) -> Vec<Vec<TileDetails>> {
        let tile_library = build_library();
        let mut map_tiles = Vec::new();
        let mut row;
        for _y in map_area.start_position.y..=map_area.end_position.y {
            row = Vec::new();
            for _x in map_area.start_position.x..=map_area.end_position.x {
                row.push( tile_library[&tile].clone());
            }
            map_tiles.push(row);
        }
        map_tiles
    }

    async fn test_movement_input(levels: Levels, start_position: Position, input: Vec<Key>, expected_end_position : Position) {
        // GIVEN a game engine with a 3x3 grid of tiles
        let _tile_library = build_library();
        let terminal_manager = terminal_manager::init_test(MIN_RESOLUTION).unwrap();
        let game_engine = build_test_game_engine(levels, terminal_manager);

        match game_engine {
            Ok(mut engine) => {
                let levels = engine.levels.get_level_mut();
                let player = levels.characters.get_player_mut();
                assert_eq!(start_position, player.unwrap().get_global_position());
                
                // Feed each input into the engine handling
                for key in input {
                    engine.handle_input(key).await.unwrap();
                }
                
                assert_eq!(expected_end_position, engine.levels.get_level_mut().characters.get_player().unwrap().get_global_position());
            },
            _ => {
                panic!("Expected a valid Game Engine instance!")
            }
        }
    }
    
    #[test]
    fn test_build_game_engine() {
        let terminal_manager = terminal_manager::init_test(MIN_RESOLUTION).unwrap();
        let _game_engine = build_game_engine(terminal_manager);
    }

    #[tokio::test]
    async fn test_move_down_traversable() {
        // GIVEN a game engine with a 3x3 grid of tiles
        let tile_library = build_library();
        let map_area = build_square_area(Position { x: 0, y: 0}, 3);
        let mut map_tiles = build_tiles(map_area.clone(), TileType::NoTile);

        // AND the middle / bottom middle tile is a corridor
        map_tiles[1] [1] = tile_library[&TileType::Corridor].clone();
        map_tiles[2] [1] = tile_library[&TileType::Corridor].clone();
        let map = Map {
            area: map_area,
            tiles : Tiles { tiles: map_tiles },
            rooms : Vec::new(),
            containers: HashMap::new()
        };

        // AND the player start position is the middle of the map
        let start_position = Position{x:1, y:1};

        let player_pattern_result = CharacterPattern::new_player();
        assert!(player_pattern_result.is_ok(), "Failed to build player CharacterPattern!");

        let player =  CharacterBuilder::new(player_pattern_result.unwrap())
            .position(start_position)
            .build(String::from("Test Player"));
        let levels = build_test_levels(map, player);

        // WHEN we attempt to move down
        let input = vec![Key::Down];
        // THEN we expect the player's position to be updated to the other corridor tile
        let expected_end_position = Position{x:1, y: 2};
        test_movement_input(levels, start_position, input, expected_end_position).await;
    }

    #[tokio::test]
    async fn test_move_down_non_traversable() {
        // GIVEN a game engine with a 3x3 grid of tiles
        let tile_library = build_library();
        let map_area = build_square_area(Position { x: 0, y: 0}, 3);
        let mut map_tiles = build_tiles(map_area.clone(), TileType::NoTile);

        // AND only the middle tile is a corridor
        map_tiles[1] [1] = tile_library[&TileType::Corridor].clone();
        let map = Map {
            area: map_area,
            tiles : Tiles { tiles: map_tiles },
            rooms : Vec::new(),
            containers: HashMap::new()
        };

        // AND the player start position is the middle of the map
        let start_position = Position{x:1, y:1};
        let player_pattern_result = CharacterPattern::new_player();
        assert!(player_pattern_result.is_ok(), "Failed to build player CharacterPattern!");

        let player =  CharacterBuilder::new(player_pattern_result.unwrap())
            .position(start_position)
            .build(String::from("Test Player"));
        let levels = build_test_levels(map, player);
        // WHEN we attempt to move down
        let input = vec![Key::Down];
        // THEN we expect the player's position to remain unchanged
        let expected_end_position = Position{x:1, y: 1};
        test_movement_input(levels, start_position, input, expected_end_position).await;
    }

    #[tokio::test]
    async fn test_move_up_traversable() {
        // GIVEN a game engine with a 3x3 grid of tiles
        let tile_library = build_library();
        let map_area = build_square_area(Position { x: 0, y: 0}, 3);
        let mut map_tiles = build_tiles(map_area.clone(), TileType::NoTile);

        // AND the middle / bottom middle tile is a corridor
        map_tiles[1] [1] = tile_library[&TileType::Corridor].clone();
        map_tiles[2] [1] = tile_library[&TileType::Corridor].clone();
        let map = Map {
            area: map_area,
            tiles : Tiles { tiles: map_tiles },
            rooms : Vec::new(),
            containers: HashMap::new()
        };

        // AND the player start position is the bottom middle of the map
        let start_position = Position{x:1, y:2};
        let player_pattern_result = CharacterPattern::new_player();
        assert!(player_pattern_result.is_ok(), "Failed to build player CharacterPattern!");
        let player =  CharacterBuilder::new(player_pattern_result.unwrap())
            .position(start_position)
            .build(String::from("Test Player"));
        let levels = build_test_levels(map, player);

        // WHEN we attempt to move up
        let input = vec![Key::Up];
        // THEN we expect the player's position to be updated to the other corridor tile
        let expected_end_position = Position{x:1, y: 1};
        test_movement_input(levels, start_position, input, expected_end_position).await;
    }

    #[tokio::test]
    async fn test_move_up_non_traversable() {
        // GIVEN a game engine with a 3x3 grid of tiles
        let tile_library = build_library();
        let map_area = build_square_area(Position { x: 0, y: 0}, 3);
        let mut map_tiles = build_tiles(map_area.clone(), TileType::NoTile);

        // AND only the middle end tile is a corridor
        map_tiles[2] [1] = tile_library[&TileType::Corridor].clone();
        let map = Map {
            area: map_area,
            tiles : Tiles { tiles: map_tiles},
            rooms : Vec::new(),
            containers: HashMap::new()
        };

        // AND the player start position is the middle end of the map
        let start_position = Position{x:1, y:2};
        let player_pattern_result = CharacterPattern::new_player();
        assert!(player_pattern_result.is_ok(), "Failed to build player CharacterPattern!");
        let player =  CharacterBuilder::new(player_pattern_result.unwrap())
            .position(start_position)
            .build(String::from("Test Player"));
        let levels = build_test_levels(map, player);

        // WHEN we attempt to move up
        let input = vec![Key::Up];
        // THEN we expect the player's position to remain unchanged
        let expected_end_position = Position{x:1, y: 2};
        test_movement_input(levels, start_position, input, expected_end_position).await;
    }

    #[tokio::test]
    async fn test_move_left_traversable() {
        // GIVEN a game engine with a 3x3 grid of tiles
        let tile_library = build_library();
        let map_area = build_square_area(Position { x: 0, y: 0}, 3);
        let mut map_tiles = build_tiles(map_area.clone(), TileType::NoTile);

        // AND the middle / middle left tile is a corridor
        map_tiles[1] [0] = tile_library[&TileType::Corridor].clone();
        map_tiles[1] [1] = tile_library[&TileType::Corridor].clone();
        let map = Map {
            area: map_area,
            tiles : Tiles { tiles: map_tiles},
            rooms : Vec::new(),
            containers: HashMap::new()
        };

        // AND the player start position is the middle of the map
        let start_position = Position{x:1, y:1};
        let player_pattern_result = CharacterPattern::new_player();
        assert!(player_pattern_result.is_ok(), "Failed to build player CharacterPattern!");
        let player =  CharacterBuilder::new(player_pattern_result.unwrap())
            .position(start_position)
            .build(String::from("Test Player"));
        let levels = build_test_levels(map, player);

        // WHEN we attempt to move left
        let input = vec![Key::Left];
        // THEN we expect the player's position to be updated to the other corridor tile
        let expected_end_position = Position{x:0, y: 1};
        test_movement_input(levels, start_position, input, expected_end_position).await;
    }

    #[tokio::test]
    async fn test_move_left_non_traversable() {
        // GIVEN a game engine with a 3x3 grid of tiles
        let tile_library = build_library();
        let map_area = build_square_area(Position { x: 0, y: 0}, 3);
        let mut map_tiles = build_tiles(map_area.clone(), TileType::NoTile);

        // AND only the middle end tile is a corridor
        map_tiles[1] [1] = tile_library[&TileType::Corridor].clone();
        let map = Map {
            area: map_area,
            tiles : Tiles { tiles: map_tiles},
            rooms : Vec::new(),
            containers: HashMap::new()
        };

        // AND the player start position is the middle of the map
        let start_position = Position{x:1, y:1};
        let player_pattern_result = CharacterPattern::new_player();
        assert!(player_pattern_result.is_ok(), "Failed to build player CharacterPattern!");
        let player =  CharacterBuilder::new(player_pattern_result.unwrap())
            .position(start_position)
            .build(String::from("Test Player"));
        let levels = build_test_levels(map, player);

        // WHEN we attempt to move left
        let input = vec![Key::Left];
        // THEN we expect the player's position to remain unchanged
        let expected_end_position = Position{x:1, y: 1};
        test_movement_input(levels, start_position, input, expected_end_position).await;
    }

    #[tokio::test]
    async fn test_move_right_traversable() {
        // GIVEN a game engine with a 3x3 grid of tiles
        let tile_library = build_library();
        let map_area = build_square_area(Position { x: 0, y: 0}, 3);
        let mut map_tiles = build_tiles(map_area.clone(), TileType::NoTile);

        // AND the middle / middle right tile is a corridor
        map_tiles[1] [1] = tile_library[&TileType::Corridor].clone();
        map_tiles[1] [2] = tile_library[&TileType::Corridor].clone();
        let map = Map {
            area: map_area,
            tiles : Tiles { tiles: map_tiles},
            rooms : Vec::new(),
            containers: HashMap::new()
        };

        // AND the player start position is the middle of the map
        let start_position = Position{x:1, y:1};
        let player_pattern_result = CharacterPattern::new_player();
        assert!(player_pattern_result.is_ok(), "Failed to build player CharacterPattern!");
        let player =  CharacterBuilder::new(player_pattern_result.unwrap())
            .position(start_position)
            .build(String::from("Test Player"));
        let levels = build_test_levels(map, player);

        // WHEN we attempt to move right
        let input = vec![Key::Right];
        // THEN we expect the player's position to be updated to the other corridor tile
        let expected_end_position = Position{x:2, y: 1};
        test_movement_input(levels, start_position, input, expected_end_position).await;
    }

    #[tokio::test]
    async fn test_move_right_non_traversable() {
        // GIVEN a game engine with a 3x3 grid of tiles
        let tile_library = build_library();
        let map_area = build_square_area(Position { x: 0, y: 0}, 3);
        let mut map_tiles = build_tiles(map_area.clone(), TileType::NoTile);

        // AND only the middle end tile is a corridor
        map_tiles[1] [1] = tile_library[&TileType::Corridor].clone();
        let map = Map {
            area: map_area,
            tiles : Tiles { tiles: map_tiles},
            rooms : Vec::new(),
            containers: HashMap::new()
        };

        // AND the player start position is the middle of the map
        let start_position = Position{x:1, y:1};
        let player_pattern_result = CharacterPattern::new_player();
        assert!(player_pattern_result.is_ok(), "Failed to build player CharacterPattern!");
        let player =  CharacterBuilder::new(player_pattern_result.unwrap())
            .position(start_position)
            .build(String::from("Test Player"));
        let levels = build_test_levels(map, player);

        // WHEN we attempt to move right
        let input = vec![Key::Right];
        // THEN we expect the player's position to remain unchanged
        let expected_end_position = Position{x:1, y: 1};
        test_movement_input(levels, start_position, input, expected_end_position).await;
    }

}