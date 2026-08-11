use uuid::Uuid;

use crate::character::builder::character_builder::CharacterType::{GoblinWarrior, NewPlayer};
use crate::character::character_details::CharacterDetails;
use crate::character::equipment::Equipment;
use crate::character::equipment::EquipmentSlot::PRIMARY;
use crate::character::stats::attributes::{AttributeScore, AttributeScores};
use crate::character::{Character, Class, Race};
use crate::error::errors::ErrorWrapper;
use crate::map::objects::container::{Container, ContainerType};
use crate::map::objects::items::{Item, ItemForm, MaterialType};
use crate::map::objects::weapon_builder::{BladedWeaponType, WeaponBlueprint, WeaponBuilder};
use crate::map::position::Position;
use crate::map::tile::{Colour, Symbol};

const DEFAULT_POSITION: Position = Position { x: 0, y: 0 };

#[derive(Clone, Debug)]
pub enum CharacterType {
    NewPlayer, // Default human player character, before character building / specialisation
    GoblinWarrior
}

#[derive(Clone, Debug)]
pub struct CharacterPattern {
    character_type: CharacterType,
    blueprint: CharacterBlueprint
}

pub fn build_dev_player_inventory() -> Container {
    let mut container = Container::new(Uuid::new_v4(), "Player's Inventory".to_owned(), '$', 50.0, 1, ContainerType::AREA, 1000000);
    let bronze_bar = Item::new_with_form(Uuid::new_v4(), "Bronze Bar".to_owned(), MaterialType::BRONZE, ItemForm::BAR, 'X', 1.0, 50);
    let mut bag = Container::new(Uuid::new_v4(), "Bag".to_owned(), '$', 5.0, 50, ContainerType::OBJECT, 600);
    let mut carton = Container::new(Uuid::new_v4(), "Carton".to_owned(), '$', 1.0, 50, ContainerType::OBJECT, 5);
    let tin_bar = Item::new_with_form(Uuid::new_v4(), "Tin Bar".to_owned(), MaterialType::TIN, ItemForm::BAR,'X', 1.0, 50);

    // +1 weight
    carton.add_item(tin_bar).expect("The Tin Bar should have been added to the Carton");

    bag.add(carton).expect("The Carton should have been added to the Bag");
    bag.add_item(bronze_bar).expect("The Bronze Bar should have been added to the Bag");

    // +8 weight (bad contains 3 weight)
    container.add(bag).expect("The Bag should have been added to the container");

    // + 60 weight
    for i in 1..=60 {
        let test_item = Item::new(Uuid::new_v4(), format!("Test Item {}", i), MaterialType::UNKNOWN, '$', 1.0, 100);
        container.add_item(test_item).expect(format!("Test Item {} should have been added to the Player's Inventory", i).as_str());
    }
    return container;
}

/*
    Future TODO - Consider storing patterns in a proper data store so we can look them up by CharacterType alone / keep the code lightweight
 */
impl CharacterPattern {
    pub fn new_player() -> Result<CharacterPattern, ErrorWrapper> {
        let attributes: Vec<AttributeScore> = AttributeScores::default().scores;

        // Builds a "Steel Arming Sword" for the player to wield as PRIMARY
        let blueprint = WeaponBlueprint::new(MaterialType::STEEL, ItemForm::BLADED(BladedWeaponType::ARMING)).unwrap();
        let weapon_builder = WeaponBuilder::new(blueprint);
        let sword = weapon_builder.build();
        let equipped_sword = Container::wrap(sword.clone());
        let mut equipment = Equipment::new();
        equipment.replace(equipped_sword, PRIMARY).expect("The sword should have been equipped as PRIMARY");

        let mut inventory = build_dev_player_inventory();
        let add_result = inventory.add_item(sword);
        if add_result.is_err() {
            return Err(add_result.err().unwrap())
        }

        let blueprint = CharacterBlueprint {
            details : CharacterDetails::new(Race::Human, Class::None, 0, 6, 6, attributes),
            position: None,
            symbol: Symbol::new('@', Colour::Green),
            health: 100,
            inventory,
            equipment
        };
        Ok(CharacterPattern { character_type: NewPlayer, blueprint })
    }

    pub fn goblin() -> Result<CharacterPattern, ErrorWrapper> {
        let blueprint = WeaponBlueprint::new(MaterialType::IRON, ItemForm::BLADED(BladedWeaponType::DAGGER)).unwrap();
        let weapon_builder = WeaponBuilder::new(blueprint);
        let dagger = weapon_builder.build();
        let equipped_dagger = Container::wrap(dagger.clone());
        let mut equipment = Equipment::new();
        equipment.replace(equipped_dagger, PRIMARY).expect("The Dagger should have been equipped as PRIMARY");

        let mut inventory = Container::new(Uuid::new_v4(), "A Goblin's dead body".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let add_result = inventory.add_item(dagger);
        if add_result.is_err() {
            return Err(add_result.err().unwrap())
        }

        let attributes: Vec<AttributeScore> = AttributeScores::all_at_value(2).scores;
        let blueprint = CharacterBlueprint {
            details : CharacterDetails::new(Race::Goblin, Class::Warrior, 1, 3, 0, attributes),
            position: None,
            symbol: Symbol::new('g', Colour::Green),
            health: 80,
            inventory,
            equipment
        };

        Ok(CharacterPattern { character_type: GoblinWarrior, blueprint })
    }
}

/*
    These are the more generally re-usable elements of character patterns that can be used to generate individuals
 */
#[derive(Clone, Debug)]
pub struct CharacterBlueprint {
    pub details: CharacterDetails,
    pub position: Option<Position>,
    pub symbol: Symbol,
    pub health: i8,
    pub inventory: Container,
    pub equipment: Equipment
}

pub struct CharacterBuilder {
    pattern: CharacterPattern
}

impl CharacterBuilder {
    pub fn new(character_pattern: CharacterPattern) -> CharacterBuilder {
        CharacterBuilder { pattern: character_pattern }
    }

    pub fn position(&mut self, position: Position) -> &mut CharacterBuilder {
        self.pattern.blueprint.position = Some(position);
        self
    }

    pub fn build(&self, character_name: String) -> Character {
        let character_type = &self.pattern.character_type;
        let blueprint = &self.pattern.blueprint;
        let details = &blueprint.details;
        let symbol = &blueprint.symbol;
        let health = blueprint.health;
        let mut inventory = blueprint.inventory.clone();

        match character_type {
            NewPlayer => {
                let container_name = format!("{}'s Inventory", character_name);
                inventory.get_self_item_mut().set_name(container_name);
            },
            _ => {}
        }

        let position = if let Some(pos) = blueprint.position {
            pos
        } else {
            DEFAULT_POSITION
        };

        let equipment = &blueprint.equipment;
        let character = Character::new_detailed(character_name, position, details.clone(), symbol.clone(), health, inventory.clone(), equipment.clone());
        return character;
    }
}

#[cfg(test)]
mod tests {
    use crate::character::builder::character_builder::{CharacterBuilder, CharacterPattern, DEFAULT_POSITION};
    use crate::character::equipment::EquipmentSlot::PRIMARY;
    use crate::character::stats::attributes::{AttributeScore, AttributeScores};
    use crate::character::{Class, Race};
    use crate::map::tile::Colour;

    fn assert_attribs(expected: Vec<AttributeScore>, actual: Vec<AttributeScore>) {
        assert_eq!(expected.len(), actual.len());
        for e in expected {
            let found = actual.iter().find(|a| a.attribute == e.attribute);
            assert!(found.is_some());
            assert_eq!(e.score, found.unwrap().score)
        }
    }

    #[test]
    pub fn test_build_new_player() {
        // GIVEN a CharacterBuilder with the player pattern
        let player_pattern_result = CharacterPattern::new_player();
        assert!(player_pattern_result.is_ok(), "Failed to build player CharacterPattern!");
        let builder = CharacterBuilder::new(player_pattern_result.unwrap());
        // WHEN we calll to build a character
        let mut character = builder.build(String::from("Player"));

        assert_eq!("Player", character.name);
        assert_eq!('@', character.symbol.character);
        assert_eq!(Colour::Green, character.symbol.colour);

        let details = character.character_details.clone();
        assert_eq!(Race::Human, details.get_race().clone());
        assert_eq!(Class::None, details.get_class().clone());
        let attribs = details.get_attributes();
        let expected_attribs = AttributeScores::all_at_value(0).scores;
        assert_attribs(expected_attribs, attribs);
        assert_eq!(0, details.get_level());
        assert_eq!(6, details.get_free_attribute_points());
        assert_eq!(6, details.get_max_free_attribute_points());

        assert_eq!(100, character.health);
        assert_eq!(DEFAULT_POSITION, character.position);

        let inventory = character.get_inventory_mut();
        assert_eq!(62, inventory.get_contents().len());

        let equipment = character.get_equipment_mut();
        assert_eq!(1, equipment.get_slots().len());

        let primary_weapon = equipment.get_item(PRIMARY).unwrap();
        assert_eq!("Steel Arming Sword", primary_weapon.get_name());
    }

    #[test]
    pub fn test_build_goblin() {
        // GIVEN a CharacterBuilder with the goblin pattern
        let goblin_pattern_result = CharacterPattern::goblin();
        assert!(goblin_pattern_result.is_ok(), "Failed to build Goblin CharacterPattern!");
        let builder = CharacterBuilder::new(goblin_pattern_result.unwrap());
        // WHEN we calll to build a character
        let mut character = builder.build(String::from("Ruggo"));

        assert_eq!("Ruggo", character.name);
        assert_eq!('g', character.symbol.character);
        assert_eq!(Colour::Green, character.symbol.colour);

        let details = character.character_details.clone();
        assert_eq!(Race::Goblin, details.get_race().clone());
        assert_eq!(Class::Warrior, details.get_class().clone());
        let attribs = details.get_attributes();
        let expected_attribs = AttributeScores::all_at_value(2).scores;
        assert_attribs(expected_attribs, attribs);
        assert_eq!(1, details.get_level());
        assert_eq!(0, details.get_free_attribute_points());
        assert_eq!(3, details.get_max_free_attribute_points());

        assert_eq!(80, character.health);
        assert_eq!(DEFAULT_POSITION, character.position);

        let inventory = character.get_inventory_mut();
        assert_eq!(1, inventory.get_contents().len());

        let equipment = character.get_equipment_mut();
        assert_eq!(1, equipment.get_slots().len());

        let primary_weapon = equipment.get_item(PRIMARY).unwrap();
        assert_eq!("Iron Dagger", primary_weapon.get_name());
    }
}
