use std::convert::TryInto;
use std::fmt;
use std::io::{Error, ErrorKind};

use crate::error::errors::ErrorWrapper;
use uuid::Uuid;

use crate::map::objects::items::{Item, ItemType};
use crate::map::position::Position;
use crate::widget::stateful::container_choice_widget::ContainerChoice;

#[derive(Clone, Eq)]
#[derive(PartialEq)]
#[derive(Debug)]
pub enum ContainerType {
    ITEM, // No storage, just a wrapped Item
    OBJECT, // Movable container i.e Bags
    AREA // Fixed container i.e Floor, Chest, or a Player's inventory, these do not count towards weight totals are they are never "part" of another container
}

impl fmt::Display for ContainerType {
    fn fmt(&self, formatter: &mut fmt::Formatter) -> fmt::Result {
        fmt::Debug::fmt(self, formatter)
    }
}

/*
* Container is an item wrapper at it's most basic (ITEM),
* otherwise a container for storing items (i.e: OBJECT, AREA) which are also Container instances
*/
#[derive(Clone, Debug, PartialEq)]
pub struct Container {
    item : Item,
    pub container_type : ContainerType,
    weight_limit : i32,
    contents : Vec<Container>
}

impl Container {
    pub fn new(id: Uuid, name: String, symbol: char, weight : f32, value : i32, container_type : ContainerType, weight_limit : i32) -> Container {
        let container_item = Item::container_item(id, name, symbol, weight, value);
        Container { item: container_item, container_type, weight_limit, contents: Vec::new()}
    }

    pub fn wrap(item: Item) -> Container {
        Container { item, container_type: ContainerType::ITEM, weight_limit: 0, contents: Vec::new() }
    }
    
    pub fn is_moveable_container(&self) -> bool {
        let container_type_valid = self.container_type == ContainerType::OBJECT;
        let item_type_valid = self.item.item_type == ItemType::CONTAINER;
        return container_type_valid && item_type_valid;
    }

    pub fn is_fixed_container(&self) -> bool {
        let container_type_valid = self.container_type == ContainerType::AREA;
        let item_type_valid = self.item.item_type == ItemType::CONTAINER;
        return container_type_valid && item_type_valid;
    }

    pub fn is_true_container(&self) -> bool {
        let container_type_valid = (self.container_type == ContainerType::OBJECT) | (self.container_type == ContainerType::AREA);;
        let item_type_valid = self.item.item_type == ItemType::CONTAINER;
        return container_type_valid && item_type_valid;
    }

    pub fn get_self_item(&self) -> &Item {
        &self.item
    }

    pub fn get_self_item_mut(&mut self) -> &mut Item {
        &mut self.item
    }

    pub fn id_equals(&self, other: &Container) -> bool {
        self.item.get_id() == other.get_self_item().get_id()
    }

    pub fn id_equals_uuid(&self, other_id: Uuid) -> bool {
        self.item.get_id() == other_id
    }

    pub fn get_contents(&self) -> &Vec<Container> {
        &self.contents
    }

    pub fn get_contents_mut(&mut self) -> &mut Vec<Container> {
        &mut self.contents
    }

    pub fn get_container_type(&self) -> ContainerType {
        self.container_type.clone()
    }

    // Finds all subcontainers (OBJECTs) within this container's topmost level
    pub fn find_topmost_subcontainers(&mut self) -> Vec<&mut Container> {
        let mut containers = Vec::new();
        for c in &mut self.contents {
            if c.container_type == ContainerType::OBJECT {
                containers.push(c);
            }
        }
        containers
    }

    /*
    * Returns a ContainerChoice copy of each subcontainer
    */
    pub fn find_subcontainer_choices(&mut self, position: Position) -> Vec<ContainerChoice> {
        let container_name =  self.get_self_item().get_name();
        let mut containers = Vec::new();
        for c in &mut self.contents {
            if c.container_type == ContainerType::OBJECT {
                let sc = c.find_subcontainer_choices(position);
                for s in sc {
                    containers.push(s.clone());
                }
                containers.push(
                    ContainerChoice {
                        container: c.clone(),
                        position: position.clone(),
                        location_name: container_name.clone()
                    }
                );
            }
        }
        containers
    }

    fn count_contents(&self) -> usize {
        let mut item_count = 0;
        if self.is_true_container() {
            for c in self.get_contents() {
                item_count += 1;
                let count = c.get_total_count();
                item_count += count;
            }
        }
        return item_count;
    }

    // Returns the top-level content count if this is a 'true' container, 0 otherwise (i.e a wrapped item)
    pub fn get_top_level_count(&self) -> usize {
        return if self.is_true_container() {
            self.contents.len()
        } else {
            0
        }
    }

    // Returns the total count of all contents (recursively)
    pub fn get_total_count(&self) -> usize {
        let mut item_count = 0;
        if self.is_true_container() {
            for c in self.get_contents() {
                item_count += 1;
                let content_count = c.count_contents();
                item_count += content_count;
            }
        } else {
            return 0;
        }
        return item_count;
    }

    pub fn get_weight_limit(&self) -> i32 {
        self.weight_limit.clone()
    }

    pub fn can_fit_container_item(&self, item: &Container) -> bool {
        let weight_limit = self.weight_limit.clone() as f32;
        let content_weight_total = self.get_contents_weight_total();
        let free_weight : f32 = weight_limit - content_weight_total;
        item.get_weight_total() <= free_weight
    }

    pub fn get(&self, index: i32) -> &Container {
        &self.contents[index as usize]
    }

    pub fn get_optional(&self, index: i32) -> Option<&Container> {
        self.contents.get(index as usize)
    }

    pub fn find(&self, item: &Item) -> Option<&Container> {
        // Initial pass
        let found = self.contents.iter().find(|c| {
            let expected_id = item.get_id();
            let self_item = c.get_self_item();
            self_item.get_id() == expected_id
        });

        if found.is_some() {
            found
        } else {
            // Recurse
            return self.contents.iter().flat_map(|c| {
                return c.find(item);
            }).next();
        }
    }

    pub fn find_by_id(&self, item_id: &Uuid) -> Option<&Container> {
        // Initial pass
        let found = self.contents.iter().find(|c| {
            let self_item = c.get_self_item();
            self_item.get_id() == *item_id
        });

        if found.is_some() {
            found
        } else {
            // Recurse
            return self.contents.iter().flat_map(|c| {
                return c.find_by_id(item_id);
            }).next();
        }
    }

    pub fn push(&mut self, containers : Vec<Container>) {
        self.contents.extend(containers);
    }

    pub fn insert(&mut self, index: usize, containers : Vec<Container>) {
        let mut to_move : Vec<Container> = Vec::new();
        for c in containers {
            to_move.push(c.clone())
        }
        self.contents.splice(index..index, to_move);
    }

    pub fn replace_container(&mut self, new: Container) {
        if let Some(c) = self.contents.iter_mut().find(|c| c.id_equals(&new)) {
            *c = new;
        }
    }

    pub fn replace(&mut self, index: usize, container : Container) {
        if self.contents.len() > 0 && index < self.contents.len() {
            self.contents[index] = container;
        }
    }

    pub fn remove_item(&mut self, item : &Container) -> bool {
        if let Some(position) = self.position(item) {
            self.contents.remove(position);
            return true;
        }
        return false;
    }

    pub fn remove_matching_items(&mut self, items : Vec<Container>) -> bool {
        let mut removed = Vec::new();
        for item in items.iter() {
            removed.push(item.clone());
            if !self.remove_item(item) {
                for c in self.contents.iter_mut() {
                    if c.remove_matching_items(items.clone()) {
                        removed = items.clone();
                    }
                }
            }
        }
        return removed.len() == items.len();
    }

    pub fn remove_items(&mut self, items : Vec<&Container>) {
        for item in items.iter() {
            if let Some(position) = self.position(item) {
                self.contents.remove(position);
            }
        }
    }

    pub fn position(&self, container: &Container) -> Option<usize> {
        self.contents.iter().position(|c| {
            let expected_id = container.get_self_item().get_id();
            let self_item = c.get_self_item();
            self_item.get_id() == expected_id
        })
    }

    pub fn item_position(&self, item: &Item) -> Option<usize> {
        self.contents.iter().position(|c| {
            let expected_id = item.get_id();
            let self_item = c.get_self_item();
            self_item.get_id() == expected_id
        })
    }

    pub fn find_mut(&mut self, target: &Item) -> Option<&mut Container> {
        let target_id = target.get_id();
        for c in self.contents.iter_mut() {
            if c.get_self_item().get_id() == target_id {
                return Some(c)
            } else {
                if let Some(subcontainer) = c.find_mut(target) {
                    return Some(subcontainer);
                }
            }
        }
        None
    }

    pub fn get_mut(&mut self, index: i32) -> Option<&mut Container> {
        let contents_size : i32 = self.contents.len().try_into().unwrap();
        if index >= 0 && index < contents_size {
            return Some(&mut self.contents[index as usize])
        }
        None
    }

    pub(crate) fn get_weight_total(&self) -> f32 {
        let mut weight_total = 0.0;
        match self.container_type {
            ContainerType::OBJECT | ContainerType::AREA => {
                weight_total += self.get_self_item().weight.clone();
                for c in self.get_contents() {
                    weight_total += c.get_weight_total();
                }
                return weight_total;
            },
            _ => {
                weight_total += self.get_self_item().weight.clone();
            }
        }
        weight_total
    }

    pub(crate) fn get_contents_weight_total(&self) -> f32 {
        return  self.get_weight_total() - self.get_self_item().weight.clone();
    }

    pub fn get_loot_value(&self) -> i32 {
        let mut loot_total = 0;
        for c in &self.contents {
            match c.container_type {
                ContainerType::ITEM => {
                    loot_total += c.get_self_item().value;
                },
                ContainerType::OBJECT | ContainerType::AREA => {
                    loot_total += c.get_self_item().value;
                    for c in c.get_contents() {
                        loot_total += c.get_loot_value();
                    }
                }
            }

        }
        loot_total
    }

    // Adds an AREA container to this container without any validation
    // This is only to be used for map generation / bootstrapping of containers
    // TODO potentially rework Floor/Chest usage to only ever have 1 AREA container at a time and swap them out when needed
    pub fn add_area(&mut self, container : Container) -> Result<(), Error> {
        return if self.is_true_container() {
            match container.container_type {
                ContainerType::AREA => {
                    let total_weight = self.get_contents_weight_total();
                    let adding_weight_limit = container.weight_limit as f32;
                    let max_weight_limit = total_weight + adding_weight_limit;

                    let within_potential_weight_limit = max_weight_limit <= self.weight_limit as f32;
                    let potential_weight = total_weight.clone() + container.get_weight_total();
                    return if within_potential_weight_limit && potential_weight <= self.weight_limit as f32 {
                        self.contents.push(container);
                        Ok(())
                    } else {
                        Err(Error::new(ErrorKind::Other, format!("Cannot add the container to this container as above the weight limit. Potential weight: {}, Weight limit: {}", potential_weight, self.weight_limit)))
                    }
                },
                _ => {
                    Err(Error::new(ErrorKind::Other, "Cannot add the container to this container, as the container is not an AREA ContainerType."))
                }
            }
        } else {
            Err(Error::new(ErrorKind::Other, "Cannot add the container to this container, as this container is not a true container."))
        }
    }


    // Adds a container to this one only if this is a true container and the adding Container is a storable/movable type (ITEM or OBJECT)
    pub fn add(&mut self, container : Container) -> Result<(), Error> {
        return if self.is_true_container() {
            match container.container_type {
                ContainerType::ITEM | ContainerType::OBJECT => {
                    let total_weight = self.get_contents_weight_total();
                    let adding_weight_limit = container.weight_limit as f32;
                    let max_weight_limit = total_weight + adding_weight_limit;

                    let within_potential_weight_limit = max_weight_limit <= self.weight_limit as f32;
                    let potential_weight = total_weight.clone() + container.get_weight_total();
                    return if within_potential_weight_limit && potential_weight <= self.weight_limit as f32 {
                        self.contents.push(container);
                        Ok(())
                    } else {
                        Err(Error::new(ErrorKind::Other, format!("Cannot add the container to this container as above the weight limit. Potential weight: {}, Weight limit: {}", potential_weight, self.weight_limit)))
                    }
                },
                _ => {
                    Err(Error::new(ErrorKind::Other, "Cannot add the container to this container, as the container is not an ITEM or OBJECT ContainerType."))
                }
            }
        } else {
            Err(Error::new(ErrorKind::Other, "Cannot add the container to this container, as this container is not a true container."))
        }
    }

    pub fn can_open(&self) -> bool {
        self.is_true_container()
    }

    pub fn add_item(&mut self, item : Item) -> Result<(), ErrorWrapper> {
        match item.item_type {
            // Container items should only ever be the meta item for a container
            ItemType::CONTAINER => {
                ErrorWrapper::internal_result(String::from("Cannot add an item of type CONTAINER to another container."))
            },
            _ => {
                let weight_limit: f32 = self.weight_limit as f32;
                let weight_total = match self.container_type {
                    ContainerType::OBJECT => {
                        let current_weight_total: f32 = self.get_weight_total();
                        let item_weight: f32 = item.weight;
                        current_weight_total + item_weight
                    },
                    ContainerType::AREA => {
                        // Only consider the weight of the item to be added
                        item.weight
                    },
                    ContainerType::ITEM => {
                        return ErrorWrapper::internal_result(String::from("Cannot add item. An item cannot be added to an ITEM ContainerType!"))
                    }
                };

                if weight_total <= weight_limit {
                    let container = Container::wrap(item.clone());
                    self.contents.push(container);
                    return Ok(())
                } else {
                    return Err(ErrorWrapper::new_internal(
                        String::from(
                            format!("Cannot add item. Not enough free weight space in this container. Current weight total: {}, item weight: {}, weight limit: {}",
                                    self.get_weight_total(), item.weight, weight_limit)
                        )
                    ))
                }
            }
        }
    }

    pub fn add_items(&mut self, items : Vec<Item>) {
        for i in items {
            self.add_item(i).expect("Item should have been added!");
        }
    }

    pub fn to_cloned_item_list(&self) -> Vec<Item> {
        let mut items = Vec::new();
        for c in self.get_contents() {
            items.push(c.get_self_item().clone());
        }
        items
    }
}

#[cfg(test)]
mod tests {
    use uuid::Uuid;

    use crate::map::objects::container::{Container, ContainerType};
    use crate::map::objects::items::{Item, MaterialType};
    use crate::map::tile::Colour;

    #[test]
    fn test_container_build() {
        let id = Uuid::new_v4();
        let container = Container::new(id, "Test Container".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);

        assert_eq!(id, container.item.get_id());
        assert_eq!("Test Container", container.item.get_name());
        assert_eq!('X', container.item.symbol.character);
        assert_eq!(Colour::White, container.item.symbol.colour);
        assert_eq!(1.0, container.item.weight);
        assert_eq!(1, container.item.value);

        assert_eq!(ContainerType::OBJECT, container.container_type);
        assert_eq!(100, container.weight_limit);

        let contents = container.get_contents();
        assert_eq!(0, contents.len());
    }

    #[test]
    fn test_container_add_item() {
        // GIVEN we have a valid container
        let mut container = Container::new(Uuid::new_v4(), "Test Container".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        // AND it has no items in it's contents
        assert_eq!(0, container.get_contents().len());

        // WHEN we call to add a new item
        let item = Item::with_defaults("Test Item".to_owned(), 1.0, 1);
        container.add_item(item).expect("Test Item should have been added");

        // THEN we expect it's contents size to increase
        assert_eq!(1, container.get_contents().len());
    }

    #[test]
    fn test_container_add_item_weight_limit() {
        // GIVEN we have a valid container
        let mut container = Container::new(Uuid::new_v4(), "Test Container".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        // AND it has no items in it's contents
        assert_eq!(0, container.get_contents().len());

        // WHEN we try to add more items than the supported weight limit of 100
        // 1 Weight is fine
        let item = Item::with_defaults("Test Item 1".to_owned(), 1.0, 1);
        // 100 weight would give us 101 > 100
        let item2 = Item::with_defaults("Test Item 2".to_owned(),100.0, 1);
        container.add_item(item).expect("Test Item should have been added");
        container.add_item(item2).expect_err("Expected Test Item 2 to fail to add");

        // THEN we expect only the first item to be added
        assert_eq!(1, container.get_contents().len());
    }

    #[test]
    fn test_container_add_item_container_item() {
        // GIVEN we have a valid container
        let mut container = Container::new(Uuid::new_v4(), "Test Container".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        // AND it has no items in it's contents
        assert_eq!(0, container.get_contents().len());

        // WHEN we call to add a new item that's of CONTAINER type
        let item = Item::container_item(Uuid::new_v4(), "Test Item".to_owned(), 'X', 1.0, 1);
        container.add_item(item).expect_err("Expected Test Item not to be added to Test Container as the container has the wrong container type.");

        // THEN we expect nothing to happen as it's an unsupported type
        assert_eq!(0, container.get_contents().len());
    }

    #[test]
    fn test_container_add() {
        // GIVEN we have a valid container
        let mut container = Container::new(Uuid::new_v4(), "Test Container".to_owned(), 'X', 0.0, 1, ContainerType::OBJECT, 40);
        // AND it has no items in it's contents
        assert_eq!(0, container.get_contents().len());

        // WHEN we call to add either an wrapped ITEM or OBJECT container
        let gold_bar = Container::wrap(Item::new(Uuid::new_v4(), "Gold Bar".to_owned(), MaterialType::GOLD, 'X', 10.0, 100));
        let bag_object = Container::new(Uuid::new_v4(), "Bag".to_owned(), 'X', 0.0, 1, ContainerType::OBJECT, 30);
        container.add(gold_bar).expect("Gold bar should have been added to the container!");
        container.add(bag_object).expect("Bag should have been added to the container!");

        // THEN we expect it's contents size to increase
        assert_eq!(2, container.get_contents().len());
    }

    #[test]
    fn test_container_add_weight_limit() {
        // GIVEN we have a valid container with a weight limit of 40
        let mut container = Container::new(Uuid::new_v4(), "Test Container".to_owned(), 'X', 0.0, 1, ContainerType::OBJECT, 40);
        // AND it has no items in it's contents
        assert_eq!(0, container.get_contents().len());

        let gold_bar_1 = Container::wrap(Item::new(Uuid::new_v4(), "Gold Bar".to_owned(), MaterialType::GOLD, 'X', 10.0, 100));
        let mut bag_object = Container::new(Uuid::new_v4(), "Bag".to_owned(), 'X', 0.0, 1, ContainerType::OBJECT, 30);
        let gold_bar_2 = Container::wrap(Item::new(Uuid::new_v4(), "Gold Bar".to_owned(), MaterialType::GOLD, 'X', 10.0, 100));
        let gold_bar_3 = Container::wrap(Item::new(Uuid::new_v4(), "Gold Bar".to_owned(), MaterialType::GOLD, 'X', 10.0, 100));
        let gold_bar_4 = Container::wrap(Item::new(Uuid::new_v4(), "Gold Bar".to_owned(), MaterialType::GOLD, 'X', 10.0, 100));
        let lockpick_1 = Container::wrap(Item::new(Uuid::new_v4(), "Lockpick".to_owned(), MaterialType::GOLD, 'X', 1.0, 5));
        let lockpick_2 = Container::wrap(Item::new(Uuid::new_v4(), "Lockpick".to_owned(), MaterialType::IRON, 'X', 1.0, 5));
        bag_object.add(gold_bar_2).expect("Gold bar 2 should have been added to the container!");
        bag_object.add(gold_bar_3).expect("Gold bar 3 should have been added to the container!");
        bag_object.add(gold_bar_4).expect("Gold bar 4 should have been added to the container!");

        // WHEN we add more items than the container can support (total of 42 weight)
        container.add(gold_bar_1).expect("Gold bar 1 should have been added to the container!");
        container.add(bag_object).expect("Bag should have been added to the container!");
        // THEN we expect only the first 2 objects to be added successfully
        container.add(lockpick_1).expect_err("Lockpick 1 should not have been added to the container (too much weight)!");
        container.add(lockpick_2).expect_err("Lockpick 2 should not have been added to the container (too much weight)!");

        // Along with the bag contents
        assert_eq!(2, container.get_contents().len());
        assert_eq!(3, container.get_contents()[1].get_contents().len());
    }

    #[test]
    fn test_container_add_area_item() {
        // GIVEN we have a valid container
        let mut container = Container::new(Uuid::new_v4(), "Test Container".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        // AND it has no items in it's contents
        assert_eq!(0, container.get_contents().len());

        // WHEN we try to add an AREA container (immovable)
        let floor = Container::new(Uuid::new_v4(), "Floor".to_owned(), 'X', 1.0, 1, ContainerType::AREA, 100);
        container.add(floor).expect_err("Should not be able to add the floor to the container.");

        // THEN we expect nothing to happen as it's an unsupported type
        assert_eq!(0, container.get_contents().len());
    }

    #[test]
    fn test_get_loot_value_empty() {
        // GIVEN we have a valid container with no items
        let container =  Container::new(Uuid::new_v4(), "Test Container".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        assert_eq!(0, container.get_contents().len());
        // WHEN we call to get the total item value
        let total_value = container.get_loot_value();
        // THEN we expect 0 to be returned
        assert_eq!(0, total_value);
    }

    #[test]
    fn test_get_loot_value() {
        // GIVEN we have a valid container
        let mut container =  Container::new(Uuid::new_v4(), "Test Container".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        assert_eq!(0, container.get_contents().len());

        // AND we've added 2 items with different values
        let gold_bar = Item::new(Uuid::new_v4(), "Gold Bar".to_owned(), MaterialType::GOLD,'X', 1.0, 100);
        container.add_item(gold_bar).expect("Gold Bar should have been added to Test Container");

        let silver_bar = Item::new(Uuid::new_v4(), "Silver Bar".to_owned(), MaterialType::GOLD,'X', 1.0, 50);
        container.add_item(silver_bar).expect("Silver Bar should have been added to Test Container");
        assert_eq!(2, container.get_contents().len());

        // WHEN we call to get their total value
        let total_value = container.get_loot_value();
        // THEN we expect the total of all the item values to be returned
        assert_eq!(150, total_value);
    }

    #[test]
    fn test_get_content_count() {
        // GIVEN we have a valid container
        let mut container =  Container::new(Uuid::new_v4(), "Test Container".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        assert_eq!(0, container.get_contents().len());
        // AND we've added 2 items with different values
        let gold_bar = Item::new(Uuid::new_v4(), "Gold Bar".to_owned(), MaterialType::GOLD,'X', 1.0, 100);
        container.add_item(gold_bar).expect("Gold Bar should have been added to Test Container");
        let silver_bar = Item::new(Uuid::new_v4(), "Silver Bar".to_owned(), MaterialType::SILVER,'X', 1.0, 50);
        container.add_item(silver_bar).expect("Silver Bar should have been added to Test Container");
        assert_eq!(2, container.get_contents().len());

        // WHEN we call to get the content count
        let count = container.get_top_level_count();
        // THEN we expect the total of the top-level contents to return
        assert_eq!(2, count);
    }

    #[test]
    fn test_get_content_count_nested() {
        // GIVEN we have a valid container
        let mut container =  Container::new(Uuid::new_v4(), "Test Container".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        assert_eq!(0, container.get_contents().len());
        // AND we've added 2 items with different values
        let gold_bar = Item::new(Uuid::new_v4(), "Gold Bar".to_owned(), MaterialType::GOLD,'X', 1.0, 100);
        container.add_item(gold_bar).expect("Gold Bar should have been added to Test Container");
        let silver_bar = Item::new(Uuid::new_v4(), "Silver Bar".to_owned(), MaterialType::SILVER,'X', 1.0, 50);
        container.add_item(silver_bar).expect("Silver Bar should have been added to Test Container");

        // AND we've added a container with it's own series of items
        let mut bag =  Container::new(Uuid::new_v4(), "Bag".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 20);
        let coin1 = Item::new(Uuid::new_v4(), "Gold Coin 1".to_owned(), MaterialType::GOLD,'X', 1.0, 10);
        let coin2 = Item::new(Uuid::new_v4(), "Gold Coin 2".to_owned(), MaterialType::GOLD,'X', 1.0, 10);
        let coin3 = Item::new(Uuid::new_v4(), "Gold Coin 3".to_owned(), MaterialType::GOLD,'X', 1.0, 10);
        bag.add_items(vec![coin1, coin2, coin3]);
        assert_eq!(3, bag.get_contents().len());
        container.add(bag).expect("Bag should have been added to the container!");
        assert_eq!(3, container.get_contents().len());

        // WHEN we call to get the content count
        let count = container.get_top_level_count();
        // THEN we expect only the total of the top-level contents to return
        assert_eq!(3, count);
    }

    #[test]
    fn test_get_content_count_empty() {
        // GIVEN we have a valid container
        let container =  Container::new(Uuid::new_v4(), "Test Container".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        // AND it contains nothing
        assert_eq!(0, container.get_contents().len());
        // WHEN we call to get the content count
        let count = container.get_top_level_count();
        // THEN
        assert_eq!(0, count);
    }

    #[test]
    fn test_get_item_count() {
        // GIVEN we have a valid container
        let mut container =  Container::new(Uuid::new_v4(), "Test Container".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        assert_eq!(0, container.get_contents().len());
        // AND we've added 2 items with different values
        let gold_bar = Item::new(Uuid::new_v4(), "Gold Bar".to_owned(), MaterialType::GOLD,'X', 1.0, 100);
        container.add_item(gold_bar).expect("Gold Bar should have been added to Test Container");
        let silver_bar = Item::new(Uuid::new_v4(), "Silver Bar".to_owned(), MaterialType::SILVER,'X', 1.0, 50);
        container.add_item(silver_bar).expect("Silver Bar should have been added to Test Container");
        assert_eq!(2, container.get_contents().len());

        // WHEN we call to get the item count
        let count = container.get_total_count();
        // THEN we expect the total count of all items to return
        assert_eq!(2, count);
    }

    #[test]
    fn test_get_item_count_nested() {
        // GIVEN we have a valid container
        let mut container =  Container::new(Uuid::new_v4(), "Test Container".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        assert_eq!(0, container.get_contents().len());
        // AND we've added 2 items with different values
        let gold_bar = Item::new(Uuid::new_v4(), "Gold Bar".to_owned(), MaterialType::GOLD,'X', 1.0, 100);
        container.add_item(gold_bar).expect("Gold Bar should have been added to Test Container");
        let silver_bar = Item::new(Uuid::new_v4(), "Silver Bar".to_owned(), MaterialType::SILVER,'X', 1.0, 50);
        container.add_item(silver_bar).expect("Silver Bar should have been added to Test Container");

        // AND we've added a container with it's own series of items
        let mut bag =  Container::new(Uuid::new_v4(), "Bag".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 20);
        let coin1 = Item::new(Uuid::new_v4(), "Gold Coin 1".to_owned(), MaterialType::GOLD,'X', 1.0, 10);
        let coin2 = Item::new(Uuid::new_v4(), "Gold Coin 2".to_owned(), MaterialType::GOLD,'X', 1.0, 10);
        let coin3 = Item::new(Uuid::new_v4(), "Gold Coin 3".to_owned(), MaterialType::GOLD,'X', 1.0, 10);
        bag.add_items(vec![coin1, coin2, coin3]);
        assert_eq!(3, bag.get_contents().len());
        container.add(bag).expect("Bag should have been added to the container!");
        assert_eq!(3, container.get_contents().len());

        // WHEN we call to get the item count
        let count = container.get_total_count();
        // THEN we expect the total count of all items to return
        assert_eq!(6, count);
    }

    #[test]
    fn test_get_item_count_empty() {
        // GIVEN we have a valid container
        let container =  Container::new(Uuid::new_v4(), "Test Container".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        // AND it contains nothing
        assert_eq!(0, container.get_contents().len());
        // WHEN we call to get the item count
        let count = container.get_total_count();
        // THEN
        assert_eq!(0, count);
    }

    #[test]
    fn test_can_open_object() {
        // GIVEN we have a valid container with the OBJECT type (e.g a bag)
        let container =  Container::new(Uuid::new_v4(), "Test Container".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        // WHEN we call to check if it can be opened
        // THEN we expect it to be true
        assert!(container.can_open());
    }

    #[test]
    fn test_can_open_area() {
        // GIVEN we have a valid container with the AREA type (e.g a chest)
        let container =  Container::new(Uuid::new_v4(), "Test Container".to_owned(), 'X', 1.0, 1, ContainerType::AREA, 100);
        // WHEN we call to check if it can be opened
        // THEN we expect it to be true
        assert!(container.can_open());
    }

    #[test]
    fn test_cannot_open_item() {
        // GIVEN we have a valid container with the ITEM type (e.g an item and not a container)
        let container =  Container::new(Uuid::new_v4(), "Test Item".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
        // WHEN we call to check if it can be opened
        // THEN we expect it to be false
        assert_eq!(false, container.can_open());
    }
}