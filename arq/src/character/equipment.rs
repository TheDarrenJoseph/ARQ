use std::collections::HashMap;
use std::fmt::{Debug, Display, Formatter};

use crate::character::equipment::EquipmentSlot::{FEET, HEAD, LEGS, PRIMARY, SECONDARY, TORSO};
use crate::error::errors::ErrorWrapper;
use crate::map::objects::container::{Container, ContainerType};
use crate::map::objects::items::{Item, ItemType};

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum EquipmentSlot {
    HEAD,
    TORSO,
    LEGS,
    FEET,
    PRIMARY,
    SECONDARY,
}

pub fn all_equipment_slots() -> Vec<EquipmentSlot> {
    vec![
        HEAD,
        TORSO,
        LEGS,
        FEET,
        PRIMARY,
        SECONDARY,
    ]
}

#[derive(Debug, Clone)]
pub enum WeaponSlot {
    PRIMARY,
    SECONDARY
}

impl WeaponSlot {
    pub fn to_equipment_slot(&self) -> EquipmentSlot {
        match self {
            WeaponSlot::PRIMARY => { EquipmentSlot::PRIMARY },
            WeaponSlot::SECONDARY => { EquipmentSlot::SECONDARY }
        }
    }
}

impl Display for EquipmentSlot {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        Debug::fmt(self, f)
    }
}

#[derive(Clone, Debug)]
pub struct Equipment {
    slots : HashMap<EquipmentSlot, Item>
}

// Slot mapping
pub fn get_potential_slots(item_type: ItemType) -> Vec<EquipmentSlot> {
    return match item_type {
        ItemType::WEAPON(_w) => {
            vec![PRIMARY, SECONDARY]
        },
        ItemType::HEADGEAR => {
            vec![HEAD]
        },
        ItemType::TORSO => {
            vec![TORSO]
        },
        ItemType::LEGS => {
            vec![LEGS]
        }
        _ => {
            Vec::new()
        }
    }
}

impl Equipment {
    pub fn new() -> Equipment {
        Equipment { slots : HashMap::new() }
    }

    pub fn is_slot_filled(&self, slot: EquipmentSlot) -> bool {
        self.slots.get(&slot).is_some()
    }

    pub fn equip(&mut self, container_item : Container, slot: EquipmentSlot) -> Result<(), ErrorWrapper> {
        return if !self.is_slot_filled(slot.clone()) {
            match container_item.get_container_type() {
                // Only wrapped items can be equipped
                ContainerType::ITEM => {
                    let item = container_item.get_self_item().clone();
                    self.slots.insert(slot, item);
                    Ok(())
                },
                ct => {
                    ErrorWrapper::internal_result(format!("Unsupported container_type: {}", ct))
                }
            }
        } else {
            ErrorWrapper::internal_result(format!("Cannot equip. Equipment slot: {} is already taken.", slot))
        }
    }

    pub fn get_item(&self, slot: EquipmentSlot) -> Option<&Item> {
        self.slots.get(&slot)
    }

    pub fn unequip(&mut self, slot: EquipmentSlot) -> Result<Container, ErrorWrapper> {
        return if self.is_slot_filled(slot.clone()) {
            let item = self.slots.remove(&slot).unwrap();
            let container = Container::wrap(item);
            Ok(container)
        } else {
            ErrorWrapper::internal_result(format!("Cannot un-equip. Equipment slot: {} is empty.", slot))
        }
    }

    pub fn get_slots(&self) -> &HashMap<EquipmentSlot, Item> {
        &self.slots
    }
}


#[cfg(test)]
mod tests {
    use uuid::Uuid;

    use crate::character::equipment::EquipmentSlot::{HEAD, PRIMARY};
    use crate::character::equipment::{all_equipment_slots, Equipment, EquipmentSlot};
    use crate::map::objects::container::{Container, ContainerType};
    use crate::map::objects::items::{Item, ItemForm, MaterialType, Weapon};
    use crate::map::objects::weapon_builder::BladedWeaponType;

    fn build_test_weapon() -> Item {
        Item::weapon(Uuid::new_v4(), "".to_owned(), ItemForm::BLADED(BladedWeaponType::ARMING), MaterialType::STEEL, 'X', 3.0, 50, Weapon { damage: 20 })
    }

    #[test]
    pub fn test_is_slot_filled_defaults() {
        // GIVEN a default Equipment
        let equipment = Equipment::new();
        // AND any given inventory slot
        for es in all_equipment_slots() {
            // WHEN we check is_slot_filled
            // THEN we we expect everything to be false
            assert!(!equipment.is_slot_filled(es));
        }
    }

    #[test]
    pub fn test_is_slot_filled() {
        // GIVEN an equipment
        // AND we've equipped an item into the PRIMARY slot
        let mut equipment = Equipment::new();
        let steel_sword = Container::wrap(Item::weapon(Uuid::new_v4(), "".to_owned(), ItemForm::BLADED(BladedWeaponType::ARMING), MaterialType::STEEL, 'X', 3.0, 50, Weapon { damage: 20 }));

        let equip_result = equipment.equip(steel_sword, PRIMARY);
        assert!(equip_result.is_ok());

        // AND any given inventory slot
        for es in all_equipment_slots(){
            // WHEN we check is_slot_filled
            // THEN we expect only the PRIMARY slot to be filled
            let expected = if es == PRIMARY {
                true
            } else {
                false
            };
            assert_eq!(expected, equipment.is_slot_filled(es));
        }
    }

    #[test]
    pub fn test_equip() {
        // GIVEN an equipment
        let mut equipment = Equipment::new();
        let weapon = build_test_weapon();
        let wrapped = Container::wrap(weapon.clone());

        // AND we've equipped an item into the PRIMARY slot
        let equip_result = equipment.equip(wrapped, PRIMARY);
        // WHEN we check if the slot is taken, peek at the item in the slot
        // THEN we expect a match to our item
        assert!(equip_result.is_ok());
        assert!(equipment.is_slot_filled(EquipmentSlot::PRIMARY));
        assert_eq!(weapon.get_id(), equipment.get_item(EquipmentSlot::PRIMARY).unwrap().get_id());
    }

    #[test]
    pub fn test_equip_bad_container_type() {
        // GIVEN an equipment
        let mut equipment = Equipment::new();

        // AND an OBJECT type container i.e a Bag
        let wrapped = Container::new(Uuid::new_v4(), "Bag".to_owned(), '$', 5.0, 50, ContainerType::OBJECT, 50);

        // WHEN we call to equip this
        let equip_result = equipment.equip(wrapped, PRIMARY);
        // THEN we expect an error to return
        assert!(equip_result.is_err());
        assert_eq!("Unsupported container_type: OBJECT".to_string(), equip_result.err().unwrap().to_string())
    }

    #[test]
    pub fn test_equip_slot_taken() {
        // GIVEN an equipment
        let mut equipment = Equipment::new();
        let weapon1 = build_test_weapon();
        let wrapped = Container::wrap(weapon1.clone());

        // AND we've equipped an item into the PRIMARY slot
        let mut equip_result = equipment.equip(wrapped, PRIMARY);
        assert!(equip_result.is_ok());
        assert!(equipment.is_slot_filled(EquipmentSlot::PRIMARY));
        assert_eq!(weapon1.get_id(), equipment.get_item(EquipmentSlot::PRIMARY).unwrap().get_id());

        // WHEN we call to equip another item to this slot
        let _weapon2 = build_test_weapon();
        let wrapped_other = Container::wrap(weapon1.clone());
        equip_result = equipment.equip(wrapped_other, PRIMARY);
        // THEN we expect an error to return
        assert!(equip_result.is_err());
        assert_eq!("Cannot equip. Equipment slot: PRIMARY is already taken.".to_string(), equip_result.err().unwrap().to_string())
    }

    #[test]
    pub fn test_unequip() {
        // GIVEN an equipment
        let mut equipment = Equipment::new();
        let weapon = build_test_weapon();
        let wrapped = Container::wrap(weapon.clone());

        // AND we've equipped an item into the PRIMARY slot
        let equip_result = equipment.equip(wrapped.clone(), PRIMARY);
        assert!(equip_result.is_ok());
        assert!(equipment.is_slot_filled(EquipmentSlot::PRIMARY));
        assert_eq!(weapon.get_id(), equipment.get_item(EquipmentSlot::PRIMARY).unwrap().get_id());

        // WHEN we call to unequip this
        let unequip_result = equipment.unequip(PRIMARY);
        // THEN we expect it to succeed and return the original item with a new wrapper Container
        assert!(unequip_result.is_ok());
        assert_eq!(wrapped.get_self_item().get_id(), unequip_result.unwrap().get_self_item().get_id());
        // AND the slot should be empty now
        assert_eq!(false, equipment.is_slot_filled(PRIMARY))
    }

    #[test]
    pub fn test_unequip_unfilled_slot() {
        // GIVEN an equipment with no slots filled
        let mut equipment = Equipment::new();

        // WHEN we call to unequip a slot
        let unequip_result = equipment.unequip(HEAD);
        assert!(unequip_result.is_err());
        assert_eq!("Cannot un-equip. Equipment slot: HEAD is empty.".to_string(), unequip_result.err().unwrap().to_string())
    }
}