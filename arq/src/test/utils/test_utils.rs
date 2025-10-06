use crate::character::builder::character_builder::{CharacterBuilder, CharacterPattern};
use crate::character::characters::Characters;
use crate::character::Character;
use crate::engine::level::{init_level_manager, Level, Levels};
use crate::map::objects::container::{Container, ContainerType};
use crate::map::objects::items::Item;
use crate::map::position::{build_square_area, Position};
use crate::map::tile::{Colour, TileType};
use crate::map::{Map, Tiles};
use rand_seeder::Seeder;
use std::collections::HashMap;
use uuid::Uuid;

pub fn build_test_level(area_container: Option<(Position, Container)>, player: Option<Character>) -> Level {
    let tile_library = crate::map::tile::build_library();
    let rom = tile_library[&TileType::Room].clone();
    let wall = tile_library[&TileType::Wall].clone();
    let map_pos = Position { x: 0, y: 0 };
    let map_area = build_square_area(map_pos, 3);

    // Add the custom area container at pos 0,0 if provided
    let mut area_containers = HashMap::new();
    if let Some(c) = area_container {
        area_containers.insert(c.0, c.1);
    }

    let map = crate::map::Map {
        area: map_area,
        tiles : Tiles { tiles : vec![
            vec![ wall.clone(), wall.clone(), wall.clone() ],
            vec![ wall.clone(), rom.clone(), wall.clone() ],
            vec![ wall.clone(), wall.clone(), wall.clone() ],
        ]},
        rooms: Vec::new(),
        containers: area_containers
    };


    let player_choice = if let Some(p) = player {
        p
    } else {
        let player_pattern_result = CharacterPattern::new_player();
        assert!(player_pattern_result.is_ok(), "Failed to build player CharacterPattern!");
        CharacterBuilder::new(player_pattern_result.unwrap())
            .build(String::from("Test Player"))
    };

    return  Level { map: Some(map) , characters: Characters::new( Some(player_choice), Vec::new())  };
}

pub fn build_test_levels(map: Map, player: Character) -> Levels {
    build_test_levels_for_level(Level {
        map: Some(map.clone()),
        characters: Characters::new(Some(player), Vec::new())
    })
}

pub fn build_test_levels_for_level(level: Level) -> Levels {
    let seed = "test".to_string();
    let seed_copy = seed.clone();
    let rng = Seeder::from(seed).into_rng();
    let mut levels = init_level_manager(seed_copy, rng);
    levels.add_level_directly(level);
    levels
}

pub fn build_test_container() -> Container {
    let id = Uuid::new_v4();
    let mut container = Container::new(id, "Test Container".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    let container_self_item = container.get_self_item();
    assert_eq!(id, container_self_item.get_id());
    assert_eq!("Test Container", container_self_item.get_name());
    assert_eq!('X', container_self_item.symbol.character);
    assert_eq!(Colour::White, container_self_item.symbol.colour);
    assert_eq!(1.0, container_self_item.weight);
    assert_eq!(1, container_self_item.value);

    for i in 1..=4 {
        let test_item = Item::with_defaults(format!("Test Item {}", i), 1.0, 100);
        container.add_item(test_item).expect("Failed to add a test item to the test container!");
    }

    assert_eq!(ContainerType::OBJECT, container.container_type);
    assert_eq!(100, container.get_weight_limit());
    let contents = container.get_contents();
    assert_eq!(4, contents.len());
    container
}
