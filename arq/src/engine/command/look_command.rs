use std::io;


use crate::engine::command::command::{Command, CommandPlayerAction};
use crate::engine::level::Level;
use crate::error::errors::ErrorWrapper;
use crate::map::objects::container::Container;
use crate::map::objects::container::ContainerType::AREA;
use crate::map::position::Position;
use crate::map::room::Room;
use crate::map::tile::TileType;
use crate::map::tile::TileType::{NoTile, Wall, Window};
use crate::terminal::terminal_manager::TerminalManager;
use crate::ui::bindings::action_bindings::PlayerAction;
use crate::ui::bindings::input_bindings::KeyBindings;
use crate::ui::bindings::look_bindings::{map_look_input_to_side, LookInput, LookKeyBindings};
use crate::ui::ui::UIViewMode::Map;
use crate::ui::ui::{get_input_key, UI};

pub struct LookCommand<'a, B: 'static + ratatui::backend::Backend> {
    pub level: &'a mut Level,
    pub ui: &'a mut UI,
    pub terminal_manager : &'a mut TerminalManager<B>,
    pub bindings : LookKeyBindings
}

impl <B: ratatui::backend::Backend> Command<()> for LookCommand<'_, B> {
    async fn start(&mut self) -> Result<(), ErrorWrapper> {
        self.ui.set_console_buffer("Where do you want to look?. Arrow keys to choose. Repeat usage to choose current location.".to_string());
        self.re_render().unwrap();
        return Ok(())
    }
}

fn describe_position_in_room(pos: Position, room: &Room) -> Option<String> {
    if let Some(door) = &room.get_doors().iter().find(|d| d.position == pos) {
        log::info!("Position is a door.");
        return Some(format!("There's a {} here.", &door.tile_details.name));
    }
    None
}

fn describe_position_container(c: &Container) -> Result<String, ErrorWrapper> {
    let item_count = c.get_top_level_count();
    let container_type = c.get_container_type();

    if container_type != AREA  {
        return ErrorWrapper::internal_result( format!("Unexpected input! Cannot describe position with container of type {}.", container_type));
    }


    let c_item_name = c.get_self_item().get_name();
    // Only detail the floor if there's items
    if c_item_name == "Floor" {
        if item_count > 1 {
            Ok(format!("There's {} items on the Floor here.", item_count))
        } else if item_count == 1 {
            let top_item_name = c.get_contents()[0].get_self_item().get_name();
            Ok(format!("There's a {} on the {} here.", top_item_name, c_item_name))
        } else {
            Ok(format!("The Floor is empty here."))
        }
    } else {
        Ok(format!("There's a {} here.", c_item_name))
    }
}

fn describe_position(pos: Position, level : &mut Level) -> Result<String, ErrorWrapper> {
    let nothing_found: String = "There's nothing here.".to_string();
    if let Some(map) = &level.map {
        if let Some(room) = map.get_rooms().iter().find(|r| r.get_area().contains_position(pos)) {
            log::info!("Position is in a room.");
            let prompt = describe_position_in_room(pos, room);
            if prompt.is_some() {
                return Ok(prompt.unwrap());
            }
        }

        return if let Some(tile) = map.tiles.get_tile(pos.clone()) {
           if tile.tile_type == NoTile {
               Ok(nothing_found)
           } else if tile.tile_type == Wall || tile.tile_type == Window {
               // We want to describe the tiles for these always, should never have containers
               return Ok(format!("There's a {} here.", &tile.name));
            } else {
               let first_description = if tile.tile_type == TileType::Room {
                   format!("You're in a room.")
               } else {
                   format!("There's a {} here.", &tile.name)
               };

               if let Some(c) = map.containers.get(&pos) {
                   Ok(format!("{} {}", first_description, describe_position_container(c)?))
               } else {
                   return Ok(first_description);
               }
            }
        } else {
            Ok(nothing_found)
        }
    } else {
        log::error!("Look usage failure, no map on level!");
        return ErrorWrapper::internal_result(String::from("Look usage failure, no map on level!"))
    }
}

impl <B: ratatui::backend::Backend> LookCommand<'_, B> {

    fn re_render(&mut self) -> Result<(), io::Error> {
        let ui = &mut self.ui;
        let level = self.level.clone();
        self.terminal_manager.terminal.draw(|frame| {
            ui.render(Some(level), Map(), frame);
        })?;
        Ok(())
    }

    fn print(&mut self, prompt: String) -> Result<(), io::Error> {
        self.ui.set_console_buffer(prompt);
        return self.re_render();
    }
}

impl <B: ratatui::backend::Backend> CommandPlayerAction<LookInput> for LookCommand<'_, B> {
    fn can_handle_action(&self, action: PlayerAction) -> bool {
        return match action {
            PlayerAction::LookAround => {
                true
            }
            _ => {
                false
            }
        };
    }

    fn handle_input(&mut self, _input: Option<&LookInput>) -> Result<(), ErrorWrapper> {
        self.ui.set_console_buffer("Where do you want to look?. Arrow keys to choose. Repeat usage to choose current location.".to_string());
        self.re_render().unwrap();
        
        let key = get_input_key()?;
        let input  = self.bindings.get_input(key);
        let side = map_look_input_to_side(input);
        
        let position = self.level.find_adjacent_player_position(side);
        if let Some(p) = position {
            log::info!("Player looking at map position: {}, {}", &p.x, &p.y);
            self.re_render()?;
            let prompt =  describe_position(p, &mut self.level)?;
            self.print(prompt)?;
            get_input_key().expect("The next keyboard key should have been captured.");
        }
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use std::collections::HashMap;

    use uuid::Uuid;

    use crate::character::builder::character_builder::{CharacterBuilder, CharacterPattern};
    use crate::character::characters::Characters;
    use crate::engine::command::look_command::{describe_position, describe_position_container, describe_position_in_room};
    use crate::engine::level::Level;
    use crate::map::objects::container::{Container, ContainerType};
    use crate::map::objects::door::build_door;
    use crate::map::position::{build_square_area, Position};
    use crate::map::room::build_room;
    use crate::map::tile::TileType;
    use crate::map::Tiles;

    fn build_test_level(container_position: Position, area_container: Container) -> Level {
        let tile_library = crate::map::tile::build_library();
        let rom = tile_library[&TileType::Room].clone();
        let wall = tile_library[&TileType::Wall].clone();
        let map_pos = Position { x: 0, y: 0 };
        let map_area = build_square_area(map_pos, 3);

        let mut area_containers = HashMap::new();
        area_containers.insert(container_position.clone(), area_container);
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

        let player_pattern_result = CharacterPattern::new_player();
        assert!(player_pattern_result.is_ok(), "Failed to build player CharacterPattern!");
        let player =  CharacterBuilder::new(player_pattern_result.unwrap())
            .build(String::from("Test Player"));
        return  Level { map: Some(map) , characters: Characters::new( Some(player), Vec::new()) };
    }

    #[test]
    fn test_describe_door_position_in_room() {
        // GIVEN a room with a door
        let start_position = Position { x: 0, y: 0};
        let area = build_square_area(start_position, 3);

        let door_position = Position { x: 1, y: 0};
        let door = build_door(door_position);
        let mut doors = Vec::new();
        doors.push(door);
        let room = build_room(area, doors);

        // WHEN we call to describe a door position
        let prompt = describe_position_in_room(door_position, &room);

        // THEN we expect the prompt to reflect this
        assert!(prompt.is_some());
        assert_eq!("There's a Door here.", prompt.unwrap());
    }

    #[test]
    fn test_describe_position_container() {
        // GIVEN a valid a container (AREA) containing 3 OBJECT containers
        let mut container =  Container::new(Uuid::new_v4(), "Floor".to_owned(), 'X', 1.0, 1, ContainerType::AREA, 100);
        let container1 =  Container::new(Uuid::new_v4(), "Test Container 1".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container2 =  Container::new(Uuid::new_v4(), "Test Container 2".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container3 =  Container::new(Uuid::new_v4(), "Test Container 3".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        container.push(vec![container1, container2, container3]);
        assert_eq!(3, container.get_total_count());

        // WHEN we call to describe this
        let prompt = describe_position_container(&container);

        // THEN we expect
        assert!(prompt.is_ok());
        assert_eq!("There's 3 items on the Floor here.", prompt.unwrap());
    }

    #[test]
    fn test_describe_position_container_single_item() {
        // GIVEN a valid a container (AREA) containing 1 ITEM
        let mut container = Container::new(Uuid::new_v4(), "Floor".to_owned(), 'X', 1.0, 1, ContainerType::AREA, 100);
        let item = Container::new(Uuid::new_v4(), "Gold Bar".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 100);
        container.push(vec![item]);
        assert_eq!(1, container.get_total_count());

        // WHEN we call to describe this
        let prompt = describe_position_container(&container);

        // THEN we expect
        assert!(prompt.is_ok());
        assert_eq!("There's a Gold Bar on the Floor here.", prompt.unwrap());
    }

    #[test]
    fn test_describe_position_container_multi_item() {
        // GIVEN a valid a container (AREA) containing 3 ITEMs
        let mut container = Container::new(Uuid::new_v4(), "Floor".to_owned(), 'X', 1.0, 1, ContainerType::AREA, 100);
        let item1 = Container::new(Uuid::new_v4(), "Gold Bar".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 1);
        let item2 = Container::new(Uuid::new_v4(), "Silver Bar".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 1);
        let item3 = Container::new(Uuid::new_v4(), "Bronze Bar".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 1);
        container.push(vec![item1, item2, item3]);
        assert_eq!(3, container.get_total_count());

        // WHEN we call to describe this
        let prompt = describe_position_container(&container);

        // THEN we expect
        assert!(prompt.is_ok());
        assert_eq!("There's 3 items on the Floor here.", prompt.unwrap());
    }

    #[test]
    fn test_describe_position_container_object() {
        // GIVEN a valid a container (AREA) containing an object
        let mut container = Container::new(Uuid::new_v4(), "Floor".to_owned(), 'X', 1.0, 1, ContainerType::AREA, 100);
        let bag = Container::new(Uuid::new_v4(), "Bag".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 1);
        container.push(vec![bag]);
        assert_eq!(1, container.get_total_count());

        // WHEN we call to describe this
        let prompt = describe_position_container(&container);

        // THEN we expect
        assert!(prompt.is_ok());
        assert_eq!("There's a Bag on the Floor here.", prompt.unwrap());
    }

    #[test]
    fn test_describe_position_container_multi_object() {
        // GIVEN a valid a container (AREA) containing 2 OBJECTs
        let mut container = Container::new(Uuid::new_v4(), "Floor".to_owned(), 'X', 1.0, 1, ContainerType::AREA, 100);
        let bag = Container::new(Uuid::new_v4(), "Bag".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 1);
        let box1 = Container::new(Uuid::new_v4(), "Box".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 1);
        container.push(vec![bag, box1]);
        assert_eq!(2, container.get_total_count());

        // WHEN we call to describe this
        let prompt = describe_position_container(&container);

        // THEN we expect
        assert!(prompt.is_ok());
        assert_eq!("There's 2 items on the Floor here.", prompt.unwrap());
    }

    #[test]
    fn test_describe_position_container_mixed() {
        // GIVEN a valid a container (AREA) containing 4 mixed containers
        let mut container = Container::new(Uuid::new_v4(), "Floor".to_owned(), 'X', 1.0, 1, ContainerType::AREA, 100);
        let bag = Container::new(Uuid::new_v4(), "Bag".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 1);
        let gold_bar = Container::new(Uuid::new_v4(), "Gold Bar".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 1);
        let box1 = Container::new(Uuid::new_v4(), "Box".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 1);
        let silver_bar = Container::new(Uuid::new_v4(), "Silver Bar".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 1);

        container.push(vec![bag, gold_bar, box1, silver_bar]);
        assert_eq!(4, container.get_total_count());

        // WHEN we call to describe this
        let prompt = describe_position_container(&container);

        // THEN we expect
        assert!(prompt.is_ok());
        assert_eq!("There's 4 items on the Floor here.", prompt.unwrap());
    }

    #[test]
    fn test_describe_room_position_multiple_items() {
        // GIVEN a valid map
        // that holds a container (AREA) containing 3 OBJECT containers
        let mut source_container = Container::new(Uuid::new_v4(), "Floor".to_owned(), 'X', 1.0, 1, ContainerType::AREA, 100);
        let container1 = Container::new(Uuid::new_v4(), "Test Container 1".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container2 = Container::new(Uuid::new_v4(), "Test Container 2".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container3 = Container::new(Uuid::new_v4(), "Test Container 3".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        source_container.push(vec![container1, container2, container3]);
        assert_eq!(3, source_container.get_total_count());

        let _source = source_container.clone();
        let container_pos =  Position { x: 1, y: 1};
        let mut level = build_test_level(container_pos, source_container);

        // WHEN we call to describe the container position
        let prompt = describe_position(container_pos, &mut level);

        // THEN we expect
        assert!(prompt.is_ok());
        assert_eq!("You're in a room. There's 3 items on the Floor here.", prompt.unwrap());
    }

    #[test]
    fn test_describe_area_multiple_items() {
        // GIVEN a valid map
        // that holds a source container (AREA) containing 3 OBJECT containers
        let mut source_container = Container::new(Uuid::new_v4(), "Floor".to_owned(), 'X', 1.0, 1, ContainerType::AREA, 100);
        let container1 = Container::new(Uuid::new_v4(), "Test Container 1".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container2 = Container::new(Uuid::new_v4(), "Test Container 2".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container3 = Container::new(Uuid::new_v4(), "Test Container 3".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        source_container.push(vec![container1, container2, container3]);
        assert_eq!(3, source_container.get_total_count());

        let _source = source_container.clone();
        let container_pos =  Position { x: 1, y: 1};
        let mut level = build_test_level(container_pos, source_container);

        // WHEN we call to describe the container position
        let prompt = describe_position(container_pos, &mut level);

        // THEN we expect
        assert!(prompt.is_ok());
        assert_eq!("You're in a room. There's 3 items on the Floor here.", prompt.unwrap());
    }

    #[test]
    fn test_describe_area_single_item() {
        // GIVEN a valid map
        // that holds a source container (AREA) containing 1 OBJECT container
        let mut source_container = Container::new(Uuid::new_v4(), "Floor".to_owned(), 'X', 1.0, 1, ContainerType::AREA, 100);
        let bag = Container::new(Uuid::new_v4(), "Bag".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        source_container.push(vec![bag]);
        assert_eq!(1, source_container.get_total_count());
        let _source = source_container.clone();
        let container_pos =  Position { x: 1, y: 1};
        let mut level = build_test_level(container_pos, source_container);

        // WHEN we call to describe the container position
        let prompt = describe_position(container_pos, &mut level);

        // THEN we expect the single item and area to be described
        assert!(prompt.is_ok());
        assert_eq!("You're in a room. There's a Bag on the Floor here.", prompt.unwrap());
    }

    #[test]
    fn test_describe_area_chest() {
        // GIVEN a valid map
        // that holds a source container (AREA) containing 1 OBJECT container with contents of it's own
        let mut source_container = Container::new(Uuid::new_v4(), "Floor".to_owned(), 'X', 1.0, 1, ContainerType::AREA, 100);
        let mut chest = Container::new(Uuid::new_v4(), "Chest".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let bag = Container::new(Uuid::new_v4(), "Bag".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 10);
        let item1 = Container::new(Uuid::new_v4(), "Test Item 1".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 1);
        let item2 = Container::new(Uuid::new_v4(), "Test Item 2".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 1);

        chest.push(vec![bag, item1, item2]);
        source_container.push(vec![chest]);
        assert_eq!(4, source_container.get_total_count());
        let _source = source_container.clone();
        let container_pos =  Position { x: 1, y: 1};
        let mut level = build_test_level(container_pos, source_container);

        // WHEN we call to describe the container position
        let prompt = describe_position(container_pos, &mut level);

        // THEN we expect the single item and area to be described
        assert!(prompt.is_ok());
        assert_eq!("You're in a room. There's a Chest on the Floor here.", prompt.unwrap());
    }

    #[test]
    fn test_describe_area_empty() {
        // GIVEN a valid map
        // that holds a source container (AREA) containing nothing
        let source_container = Container::new(Uuid::new_v4(), "Floor".to_owned(), 'X', 1.0, 1, ContainerType::AREA, 100);
        assert_eq!(0, source_container.get_total_count());
        let _source = source_container.clone();
        let container_pos =  Position { x: 1, y: 1};
        let mut level = build_test_level(container_pos, source_container);

        // WHEN we call to describe the container position
        let prompt = describe_position(container_pos, &mut level);

        // THEN we expect the area to be empty
        assert!(prompt.is_ok());
        assert_eq!("You're in a room. The Floor is empty here.", prompt.unwrap());
    }

    #[test]
    fn test_describe_area_nothing() {
        // GIVEN a valid map
        // that holds a source container (AREA) containing 1 OBJECT container
        let mut source_container = Container::new(Uuid::new_v4(), "Floor".to_owned(), 'X', 1.0, 1, ContainerType::AREA, 100);
        let bag = Container::new(Uuid::new_v4(), "Bag".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        source_container.push(vec![bag]);
        assert_eq!(1, source_container.get_total_count());
        let _source = source_container.clone();
        let container_pos =  Position { x: 1, y: 1};
        let mut level = build_test_level(container_pos, source_container);

        // WHEN we call to describe a different position
        let prompt = describe_position( Position { x: 1, y: 3}, &mut level);

        // THEN we expect nothing to be there
        assert!(prompt.is_ok());
        assert_eq!("There's nothing here.", prompt.unwrap());
    }

    #[test]
    fn test_describe_area_invalid_container_type() {
        // GIVEN a valid map
        // that holds a source container of an unsupported type (OBJECT)
        let source_container = Container::new(Uuid::new_v4(), "Floor".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container_pos =  Position { x: 1, y: 1};
        let mut level = build_test_level(container_pos, source_container);

        // WHEN we call to describe the container position
        let prompt = describe_position( container_pos, &mut level);

        // THEN we expect an error to be returned
        assert!(prompt.is_err());
        let expected =format!("Unexpected input! Cannot describe position with container of type {}.", ContainerType::OBJECT);
        assert_eq!(expected, prompt.err().unwrap().to_string());
    }
}