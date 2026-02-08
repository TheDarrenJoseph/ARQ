use std::fmt::{Debug, Display, Formatter, Result};

use uuid::Uuid;

use crate::character::character_details::{build_default_character_details, CharacterDetails};
use crate::character::equipment::Equipment;
use crate::character::stats::attributes::AttributeScore;
use crate::map::objects::container::Container;
use crate::map::position::Position;
use crate::map::tile::{Colour, Symbol};

pub mod characters;
pub mod character_details;
pub mod stats;
pub mod equipment;
pub mod battle;
pub mod builder;

#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub enum Race {Human,Goblin}

#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub enum Class {None,Warrior}

impl Display for Class {
    fn fmt(&self, f: &mut Formatter) -> Result {
        Debug::fmt(self, f)
    }
}

#[derive(Clone, Debug)]
pub struct Character {
    id : Uuid,
    name : String,
    character_details: CharacterDetails,
    symbol: Symbol,
    health: i8,
    position: Position,
    inventory: Container,
    equipment: Equipment
}

pub fn determine_class(name: String) -> Option<Class> {
    match name.as_str() {
        "None" => {
            Some(Class::None)
        }
        "Warrior" => {
            Some(Class::Warrior)
        }
        _ => {
            None
        }
    }
}

/*
pub fn build_player(name : String, position: Position) -> Character {
    let inventory = build(Uuid::new_v4(), name.clone() + &"'s Inventory".to_owned(), 'X', 1, 1,  ContainerType::OBJECT, 100);
    return Character::new(name, position, Symbol::new('@', Colour::Green), inventory)
}

pub fn build_npc(name : String, position: Position, symbol: char, symbol_colour: Colour) -> Character {
    let inventory = build(Uuid::new_v4(), name.clone() + &"'s dead body".to_owned(), 'X', 1, 1,  ContainerType::OBJECT, 100);
    return Character::new(name, position, Symbol::new('@', Colour::Green), inventory)
}*/

impl Character {
    pub fn new(name : String, position: Position, symbol: Symbol, inventory: Container) -> Character {
        let id = Uuid::new_v4();
        let health = 100;
        let character_details = build_default_character_details();
        let equipment = Equipment::new();

        let player = Character { id, name, character_details, symbol, health, position, inventory, equipment };
        return player;
    }

    pub fn new_detailed(name : String, position: Position, character_details: CharacterDetails, symbol: Symbol, health: i8, inventory: Container, equipment: Equipment) -> Character {
        let id = Uuid::new_v4();
        let player = Character { id, name, character_details, symbol, health, position, inventory, equipment };
        return player;
    }

    pub fn get_name(&self) -> String {
        return self.name.clone();
    }

    pub fn set_name(&mut self, name : String) {
        self.name = name;
    }

    pub fn get_details(&self) -> CharacterDetails {
        self.character_details.clone()
    }

    pub fn get_health(&self) -> i8 {
        return self.health.clone();
    }

    pub fn set_health(&mut self, health: i8) {
        self.health = health;
    }

    pub fn get_symbol(&self) -> char {
        self.symbol.character
    }

    pub fn get_colour(&self) -> Colour {
        return self.symbol.colour.clone();
    }

    pub fn get_global_position(&self) -> Position {
        return self.position.clone();
    }

    pub fn set_position(&mut self, position: Position) {
        self.position = position;
    }

    pub fn get_inventory(&mut self) -> &Container {
        return &self.inventory;
    }
    
    pub fn get_inventory_mut(&mut self) -> &mut Container {
        return &mut self.inventory;
    }

    pub fn set_inventory(&mut self, container: Container) {
       self.inventory = container;
    }

    pub fn set_equipment(&mut self, equipment: Equipment) {
        self.equipment = equipment
    }

    pub fn get_equipment(&mut self) -> &Equipment {
        return &self.equipment;
    }

    pub fn get_equipment_mut(&mut self) -> &mut Equipment {
        return &mut self.equipment;
    }

    pub fn get_race(&mut self) -> Race {
        self.character_details.get_race().clone()
    }

    pub fn set_race(&mut self, race: Race) {
        self.character_details.set_race(race);
    }

    pub fn get_class(&mut self) -> Class {
        self.character_details.get_class()
    }

    pub fn set_class(&mut self, class: Class) {
        self.character_details.set_class(class)
    }

    pub fn get_max_free_attribute_points(&mut self) -> i8 {
        self.character_details.get_max_free_attribute_points()
    }

    pub fn get_free_attribute_points(&mut self) -> i8 {
        self.character_details.get_free_attribute_points()
    }

    pub fn set_free_attribute_points(&mut self, points: i8) {
        self.character_details.set_free_attribute_points(points);
    }

    pub fn get_attribute_scores(&mut self) -> Vec<AttributeScore> {
        self.character_details.get_attributes()
    }

    pub fn set_attribute_scores(&mut self, scores : Vec<AttributeScore> ) {
        self.character_details.set_attributes(scores);
    }
}

#[cfg(test)]
mod tests {
    use uuid::Uuid;

    use crate::character::character_details::build_default_character_details;
    use crate::character::equipment::Equipment;
    use crate::character::Character;
    use crate::map::objects::container::{Container, ContainerType};
    use crate::map::position::Position;
    use crate::map::tile::{Colour, Symbol};

    #[test]
    fn test_character_build() {
        let id = Uuid::new_v4();
        let name = String::from("Test Person");
        let character_details = build_default_character_details();
        let symbol = Symbol { character: '@', colour: Colour::Green };
        let health = 100;
        let position = Position { x: 1, y: 1};
        let inventory = Container::new(Uuid::new_v4(), "Test Person's Inventory".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let equipment = Equipment::new();
        let mut character = Character { id, name, character_details, symbol, health, position, inventory, equipment };

        assert_eq!("Test Person", character.get_name());
        assert_eq!(100, character.get_health());
        assert_eq!(Colour::Green, character.get_colour());
        assert_eq!(position, character.get_global_position());
        assert_eq!(0, character.get_inventory_mut().get_contents().len());
    }
}
