use uuid::Uuid;

use crate::character::equipment::EquipmentSlot;
use crate::map::objects::weapon_builder::BladedWeaponType;
use crate::map::tile::{Colour, Symbol};

const DEFAULT_SYMBOL: Symbol = Symbol { character: 'X',  colour: Colour::White};

#[derive(Clone, PartialEq, Debug, Eq)]
pub enum ItemType {
    ITEM,
    CONTAINER,
    WEAPON(Weapon),
    HEADGEAR,
    TORSO,
    LEGS
}

#[derive(Clone, PartialEq, Debug, Eq)]
pub struct Weapon {
    pub damage : i32,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub enum MaterialType {
    CLOTH,
    LEATHER,
    WOOD,
    STONE,
    BRONZE,
    TIN,
    IRON,
    STEEL,
    SILVER,
    GOLD,
    UNKNOWN
}

impl MaterialType {
    pub fn name(&self) -> String {
        return match self {
            MaterialType::CLOTH => { String::from("Cloth") }
            MaterialType::LEATHER => { String::from("Leather") }
            MaterialType::WOOD => { String::from("Wood") }
            MaterialType::STONE => { String::from("Stone") }
            MaterialType::BRONZE => { String::from("Bronze") }
            MaterialType::TIN => { String::from("Tin") }
            MaterialType::IRON => { String::from("Iron") }
            MaterialType::STEEL => { String::from("Steel") }
            MaterialType::SILVER => { String::from("Silver") }
            MaterialType::GOLD => { String::from("Gold") }
            MaterialType::UNKNOWN => { String::from("Unknown")}
        }
    }


    /* Simplified Density in grams per cm^3 (Centimeter cubed)
     * Methodology for calculating these using kg/m3 values e.g
     * https://en.wikipedia.org/wiki/Steel
     * Upper steel density 8050 kg/m3 = 8.05 g/cm3 so dividing general kg/m3 values by 1000
     * 8050/1000 = 8.05, half-up leaves us 8
     */
    pub fn density_grams_cm3(&self) -> i32 {
        return match self {
            MaterialType::CLOTH => { 2 }
            MaterialType::LEATHER => { 2 }
            MaterialType::WOOD => { 1 }
            MaterialType::STONE => {
                3 // Based on Basalt 2.9 g/cm3
            }
            MaterialType::BRONZE => { 9 }
            MaterialType::TIN => { 7 }
            MaterialType::IRON => { 8 }
            MaterialType::STEEL => { 8 }
            MaterialType::SILVER => { 10 }
            MaterialType::GOLD => { 19 }
            MaterialType::UNKNOWN => { 1 }
        }
    }

}

#[derive(Clone, Debug, PartialEq)]
pub enum ItemForm {
    COIN,
    BAR,
    BLADED(BladedWeaponType),
    OTHER(String)
}

impl ItemForm {
    pub fn name(self) -> String {
        return match self {
            ItemForm::COIN => { String::from("Coin") }
            ItemForm::BAR => { String::from("Bar") }
            ItemForm::BLADED(_sword_type) => { String::from("Sword") }
            ItemForm::OTHER(description) => { description }
        }
    }
}

pub struct Dimensions {
    pub(crate) height: f32,
    pub(crate) width: f32,
    pub(crate) length: f32
}

impl Dimensions {
    pub fn area(&self) -> f32 {
        self.height * self.width * self.length
    }
}


#[derive(Clone, Debug, PartialEq)]
pub struct Item {
    id : Uuid,
    pub item_type: ItemType,
    item_form: ItemForm,
    material_type: MaterialType,
    // Future TODO add Dimensions once we have standardised item building
    name : String,
    pub symbol : Symbol,
    pub weight : f32, // weight in Kilograms
    pub value : i32,
    equipment_slot: Option<EquipmentSlot>
}

impl Item {
    pub fn get_id(&self) -> Uuid {
        self.id
    }
    pub fn get_name(&self) -> String {
        // In some cases / simple items such as "Gold Bar" or "Steel Sword" we can determine a default name purely by item details
        // In these cases, we don't expect a custom item name to be set
        if self.name.is_empty() {
            self.get_default_name()
        } else {
            self.name.clone()
        }
    }
    pub fn get_default_name(&self) -> String {
        format!("{} {}", self.material_type.clone().name(), self.item_form.clone().name())
    }

    pub fn set_name(&mut self, name: String) {
        self.name = name;
    }

    pub fn get_weight(&self) -> f32 {
        self.weight.clone()
    }
    pub fn get_value(&self) -> i32 {
        self.value.clone()
    }
    pub fn is_container(&self) -> bool {
        self.item_type == ItemType::CONTAINER
    }
    pub fn is_equipped(&self) -> bool { self.equipment_slot.is_some() }
    pub fn id_equals(&self, other: &Item) -> bool {
        // Use string for easy debugging
        let self_id = self.id.to_string();
        let other_id = other.id.to_string();
        self_id == other_id
    }

    pub fn set_equipment_slot(&mut self, slot: Option<EquipmentSlot>) {
      self.equipment_slot = slot
    }

    pub fn get_equipment_slot(&self) -> Option<EquipmentSlot> {
        self.equipment_slot.clone()
    }

    pub fn unequip(&mut self) {
        self.equipment_slot = None;
    }
}

impl Item {
    /*
        Builds a true item of the type ItemType::ITEM additional defaults e.g:
         UNKNOWN material type
     */
    pub fn with_defaults(name: String, weight : f32, value : i32) -> Item {
        Item {id: Uuid::new_v4(), item_type: ItemType::ITEM, item_form: ItemForm::OTHER(name.clone()), material_type: MaterialType::UNKNOWN, name, symbol: DEFAULT_SYMBOL, weight, value, equipment_slot: None }
    }

    /*
        Builds a true item of the type ItemType::ITEM
     */
    pub fn new(id: Uuid, name: String, material_type: MaterialType, symbol: char, weight : f32, value : i32) -> Item {
        Item {id, item_type: ItemType::ITEM, item_form: ItemForm::OTHER(name.clone()), material_type, name, symbol: Symbol::new(symbol, Colour::White), weight, value, equipment_slot: None }
    }

    pub fn new_with_form(id: Uuid, name: String, material_type: MaterialType, item_form: ItemForm, symbol: char, weight : f32, value : i32) -> Item {
        Item {id, item_type: ItemType::ITEM, item_form, material_type, name, symbol: Symbol::new(symbol, Colour::White), weight, value, equipment_slot: None }
    }

    /*
      Builds an Item with the type of ItemType::CONTAINER,
     */
    pub fn container_item(id: Uuid, name: String, symbol: char, weight : f32, value : i32) -> Item {
        Item {id, item_type: ItemType::CONTAINER, item_form: ItemForm::OTHER(name.clone()), material_type: MaterialType::UNKNOWN, name, symbol: Symbol::new(symbol, Colour::White), weight, value, equipment_slot: None }
    }

    /*
    TODO Use WeaponBuilder instead
      Builds an Item with the type of ItemType::WEAPON,
     */
    pub fn weapon(id: Uuid, name: String, item_form: ItemForm, material_type: MaterialType, symbol: char, weight : f32, value : i32, weapon: Weapon) -> Item {
        Item {id, item_type: ItemType::WEAPON(weapon), item_form, material_type, name, symbol: Symbol::new(symbol, Colour::White), weight, value, equipment_slot: None }
    }

}


#[cfg(test)]
mod tests {
    use uuid::Uuid;

    use crate::map::objects::items;
    use crate::map::objects::items::{Item, MaterialType};
    use crate::map::tile::Colour;

    #[test]
    fn test_build_item() {
        let id = Uuid::new_v4();
        let item = Item::new(id, "Test Item".to_owned(), MaterialType::UNKNOWN, 'X', 1.0, 1);
        assert_eq!(id, item.get_id());
        assert_eq!(items::ItemType::ITEM, item.item_type);
        assert_eq!("Test Item", item.name);
        assert_eq!('X', item.symbol.character);
        assert_eq!(Colour::White, item.symbol.colour);
        assert_eq!(1.0, item.weight);
        assert_eq!(1, item.value);
    }

    #[test]
    fn test_build_container() {
        let id = Uuid::new_v4();
        let item = Item::container_item(id, "Test Container".to_owned(), 'X', 1.0, 1);

        assert_eq!(id, item.get_id());
        assert_eq!(items::ItemType::CONTAINER, item.item_type);
        assert_eq!("Test Container", item.name);
        assert_eq!('X', item.symbol.character);
        assert_eq!(Colour::White, item.symbol.colour);
        assert_eq!(1.0, item.weight);
        assert_eq!(1, item.value);
    }
}