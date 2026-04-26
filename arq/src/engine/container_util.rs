use crate::engine::event::container::*;
use crate::map::Map;
use log::{error, info};
use std::io;

use crate::engine::level::Level;
use crate::error::errors::ErrorWrapper;
use crate::map::objects::container::Container;
use crate::map::objects::items::Item;
use crate::map::position::Position;
use crate::widget::stateful::container_choice_widget::ContainerChoice;

#[derive(Debug, Clone)]
pub struct CopyResult {
    pub copied: Vec<Container>,
    pub uncopied: Vec<Item>,
    pub updated_target : Option<Container>
}

// Finds the Container representations of the to_copy items within source and returns a Vec containing a clone of each of these
fn find_copying_items(source : Container, to_copy: Vec<Item>) -> Vec<Container> {
    let mut results: Vec<Container> = Vec::new();
    for item in to_copy {
        if let Some(container_item) = source.find(&item) {
            results.push(container_item.clone())
        } else {
            log::info!("Cannot add item {}. Failed to find source item: {}", item.get_name(), source.get_self_item().get_name());
        }
    }
    results
}


// Copies Container items from to_copy into the target container via a live mutable reference
fn copy_container_items(to_copy: Vec<Container>, target: &mut Container) -> CopyResult {
    let mut copied = Vec::new();
    let mut uncopied = Vec::new();
    for container_item in to_copy {
        if target.can_fit_container_item(&container_item) {
            let add_clone = container_item.clone();
            let result_clone = container_item.clone();
            match target.add(add_clone) {
                Ok(()) => {
                    copied.push(result_clone);
                },
                Err(e) => {
                    uncopied.push(container_item);
                    error!("Couldn't add the item to the target container: {}", e)
                }
            }
        } else {
            info!("Cannot add item {}. Could not fit it.", container_item.get_self_item().get_name());
            uncopied.push(container_item);
        }
    }

    let uncopied_items : Vec<Item> = uncopied.iter().map(|ci| ci.get_self_item().clone()).collect();
    CopyResult {
        copied,
        uncopied: uncopied_items,
        updated_target: Some(target.clone())
    }
}

pub fn player_take_items(data: TakeItemsRequest, level : &mut Level) -> Result<TakeItemsResponse, ErrorWrapper> {
    let player_result = level.get_player_mut();
    let total_to_take = data.to_take.len();
    let source_container_id = data.source.get_self_item().get_id();
    if let Some(player) = player_result {
        log::info!("[container_util::player_take_items] Found player: {}", player.get_name());
        let mut taken = Vec::new();
        let mut untaken = Vec::new();
        for item in data.to_take {
            if let Some(container_item) = data.source.find(&item) {
                let inventory = player.get_inventory_mut();
                if inventory.can_fit_container_item(container_item) {
                    log::info!("[container_util::player_take_items] Taking item: {}", item.get_name());
                    match player.get_inventory_mut().add(container_item.clone()) {
                        Ok(()) => {
                            // If it's added to the player inventory, go ahead and add it to the taken list for removal
                            taken.push(container_item.clone());
                        },
                        Err(e) => {
                            error!("[container_util::player_take_items] Failed to take item, couldn't add it to the Player's inventory.. {}", e);
                        }
                    }
                } else {
                    untaken.push(item);
                }
            } else {
                untaken.push(item);
            }
        }

        if !taken.is_empty() || !untaken.is_empty() {
            if let Some(pos) = data.position {
                let map_container = level.get_map_mut().unwrap().find_container_mut(&data.source, pos);
                if let Some(source_container) = map_container {
                    source_container.remove_matching_items(taken.clone());
                }
            } else {
                return Err(ErrorWrapper::new_internal( String::from("[container_util::player_take_items] No map position to take items from!")));
            }
        }

        let taken_items_message;
        if taken.is_empty() {
            taken_items_message = "You cannot take anything".to_owned();
        } else if untaken.is_empty() && taken.len() == total_to_take {
            taken_items_message = format!("You take all {} items", total_to_take).to_owned();
        } else {
            // Both taken and untaken items
            taken_items_message = format!("You took {} of the {} items, but could not carry any more", taken.len(), total_to_take);
        }

        log::info!("[take_items] returning TakeItemsResponse with {} un-taken items", untaken.len());
        let response = TakeItemsResponse {
            container_id: source_container_id,
            message: taken_items_message.to_owned(),
            untaken
        };
        return Ok(response);
    } else {
        return Err(ErrorWrapper::new_internal( String::from("[container_util::player_take_items] Failed to find the player in the level.")));
    }
}

pub fn player_drop_items(data: DropItemsRequest, level: &mut Level) -> Result<DropItemsResponse, ErrorWrapper> {
    let to_drop = data.to_drop;
    let total_to_drop = to_drop.len();
    let source_container_id = data.source.get_self_item().get_id();
    let target_position = data.position.unwrap();

    // Find the container on the map and add the "container" wrappers there
    let mut undropped = Vec::new();
    for item in &to_drop {
        undropped.push(item.clone());
    }
    let mut dropped = Vec::new();

    log::info!("container_util::player_drop_items] Dropping {} items at position: {}, {}", total_to_drop,  target_position.x, target_position.y);
    // Find the container at the target position we'll be dropping items into

    // Try and drop each item from the player's inventory
    for item in to_drop {
        let mut dropping_container_item = None;

        // Modify the player inventory first
        let player_result = level.get_player_mut();
        if let Some(player) = player_result {
            let player_inventory = player.get_inventory_mut();
            // Find the "container" wrappper matching the item returned
            if let Some(container_item) = &mut player_inventory.find_mut(&item) {
                let self_item = container_item.get_self_item_mut();
                self_item.unequip();
                dropping_container_item = Some(container_item.clone());
            }
        }

        // Secondly, add the player items to the target container
        if let Some(dropping_container) = dropping_container_item {
            if let Some(target_container) = level.get_map_mut().unwrap().get_container_at_position_mut(target_position) {
                if target_container.can_fit_container_item(&dropping_container) {
                    log::info!("container_util::player_drop_items] Dropping item: {} into: {}", item.get_name(), target_container.get_self_item().get_name());
                    match target_container.add(dropping_container.clone()) {
                        Ok(()) => {
                            let pos = undropped.iter().position(|x| x.id_equals(&item));
                            undropped.remove(pos.unwrap());
                            dropped.push(dropping_container);
                        }
                        Err(e) => {
                            error!("container_util::player_drop_items] Couldn't drop item: {}", e)
                        }
                    }
                } else {
                    log::info!("container_util::player_drop_items] Couldn't drop item. Cannot fit item: {}  into: {}", item.get_name(), target_container.get_self_item().get_name());
                }
            }
        }
    }

    // Finally, remove the dropped items from the player's inventory
    if !dropped.is_empty() {
        let player_inventory = level.get_player_mut().unwrap().get_inventory_mut();
        player_inventory.remove_matching_items(dropped.clone());
    }

    let dropped_items_message;
    if dropped.is_empty() {
        dropped_items_message = "You cannot drop anything".to_owned();
    } else if dropped.is_empty() && dropped.len() == total_to_drop {
        dropped_items_message = format!("You drop all {} items", total_to_drop).to_owned();
    } else {
        // Both taken and untaken items
        dropped_items_message = format!("You dropped {} of the {} items, but could not drop any more", dropped.len(), total_to_drop);
    }

    log::info!("[container_util::player_drop_items] returning DropItemsResponse with {} un-dropped items", undropped.len());
    let response = DropItemsResponse {
        container_id: source_container_id,
        message: dropped_items_message.to_owned(),
        undropped,
    };
    return Ok(response);
}

fn update_source_container(level: &mut Level, request: MoveItemsRequestV2, copy_result: CopyResult) -> Container {
    let source_container : &mut Container;
    match request.source {
        SourceContainerScope::WorldContainer(wc) => {
            let map =  level.map.as_mut().unwrap();
            source_container = find_container_mut(map, wc.position, &wc.container).expect("Failed to find World Container source");
        },
        SourceContainerScope::PlayerInventory(pic) => {
            let player = level.get_player_mut().unwrap();

            if player.get_inventory().get_self_item_id() == pic.container.get_self_item_id()  {
                source_container = player.get_inventory_mut();
            } else {
                let inventory_mut = player.get_inventory_mut();
                source_container = inventory_mut.find_mut(pic.container.get_self_item())
                    .expect("Failed to find Player Inventory Container source (after searching for child containers)");
            }
        }
    }

    source_container.remove_matching_items(copy_result.copied.clone());

    let mut updated_target = copy_result.updated_target;
    if let Some(ut) = &mut updated_target {
        // If the target contains our source (child_source), we need to replace the source there too
        if let Some(child_source) = ut.find_mut(source_container.get_self_item()) {
            child_source.remove_matching_items(copy_result.copied.clone());
        }
    }

    return source_container.clone();
}

fn find_container_mut<'a>(map: &'a mut Map, position: Position, container: &Container) -> Result<&'a mut Container, ErrorWrapper> {
    if let Some(pos_container) = map.containers.get_mut(&position) {
        // Find the real target container in the target container stack
        return if pos_container.id_equals(&container)  {
            Ok(pos_container)
        } else {
          // Search each child container for an ID match
          for child_container in pos_container.get_contents_mut()
              .iter_mut()
              .filter(|c| c.is_true_container()) {

              // Check the container itself before diving into it
              if child_container.id_equals(&container)  {
                  return Ok(child_container);
              }

              // Check it's contents otherwise
              let found_container = child_container.find_mut(container.get_self_item());
              if let Some(c) = found_container {
                  return Ok(c)
              }
          }

          Err(ErrorWrapper::new_internal(String::from("Failed to find container on the map after searching position and children")))
        };
    } else {
        Err(ErrorWrapper::new_internal(String::from("Failed to find any container on the map at position")))
    }
}

// Updates the containers within the request to Move items to a specific position within the source_container
fn move_items_to_source_position(request: MoveItemsRequestV2) -> Option<MoveItemsResponseV2> {
    let request_result_copy = request.clone();

    let mut source_container;
    let mut source_position = None;
    if let SourceContainerScope::WorldContainer(wc) = request.source {
        source_container = wc.container.clone();
        source_position = Some(wc.position.clone());
    } else if let SourceContainerScope::PlayerInventory(pic) = request.source {
        source_container = pic.container.clone();
    } else {
        log::error!("Failed to move items to source position. Failed to find source container.");
        return None;
    }

    if let TargetContainerScope::PlayerInventoryItemPosition(piip) = request.target {
        if let Some(_pos) = source_container.item_position(&piip.target_position_item) {
            let mut moved: Vec<Container> = Vec::new();
            let mut unmoved = Vec::new();
            let mut moving = Vec::new();

            for item in &request.to_move {
                if let Some(container_item) = source_container.find_mut(&item) {
                    moving.push(container_item.clone());
                    moved.push(container_item.clone());
                } else {
                    unmoved.push(item.clone());
                }
            }

            // Update the source and target
            source_container.remove_matching_items(moving.clone());
            // Get the position again in the updated sources
            let target_pos = source_container.item_position(&piip.target_position_item).unwrap();
            source_container.insert(target_pos, moving.clone());

            let data = MoveItemsResponseV2 {
                request: request_result_copy,
                success: true,
                updated_scopes: UpdatedScopes {
                    // If we're moving to a player item position, then we're still in the player inventory
                    source: SourceContainerScope::PlayerInventory(
                        PlayerInventoryContainer {
                            container: source_container.clone()
                        }
                    ),
                    target: TargetContainerScope::PlayerInventoryItemPosition(
                        PlayerInventoryItemPosition {
                            target_position_item: piip.target_position_item.clone()
                        }
                    )
                },
                unmoved: unmoved,
                message: format!("Moved {} of {} items", moved.len(), request.to_move.len())
            };
            return Some(data);
        }
    } else if let TargetContainerScope::WorldContainerItemPosition(wcip) = request.target {
        if let Some(_pos) = source_container.item_position(&wcip.target_position_item) {
            let mut moved: Vec<Container> = Vec::new();
            let mut unmoved = Vec::new();
            let mut moving = Vec::new();

            for item in &request.to_move {
                if let Some(container_item) = source_container.find_mut(&item) {
                    moving.push(container_item.clone());
                    moved.push(container_item.clone());
                } else {
                    unmoved.push(item.clone());
                }
            }

            // Update the source and target
            source_container.remove_matching_items(moving.clone());
            // Get the position again in the updated sources
            let target_pos = source_container.item_position(&wcip.target_position_item).unwrap();
            source_container.insert(target_pos, moving.clone());

            let data = MoveItemsResponseV2 {
                request: request_result_copy,
                success: true,
                updated_scopes: UpdatedScopes {
                    // If we're moving to a player item position, then we're still in the player inventory
                    source: SourceContainerScope::WorldContainer(
                        WorldContainer {
                            container: source_container.clone(),
                            position: source_position.unwrap(),
                        }
                    ),
                    target: TargetContainerScope::WorldContainerItemPosition(
                        WorldContainerItemPosition {
                            target_position_item: wcip.target_position_item.clone(),
                            position: wcip.position.clone(),
                        }
                    )
                },
                unmoved: unmoved,
                message: format!("Moved {} of {} items", moved.len(), request.to_move.len())
            };
            return Some(data);
        }
    }

    None
}

pub fn move_items(request: MoveItemsRequestV2, level: &mut Level) -> Result<MoveItemsResponseV2, ErrorWrapper> {
    // Copy our request data so we're not moving it around too much
    let request_result_copy = request.clone();

    let mut response : Result<MoveItemsResponseV2, ErrorWrapper>= Err(ErrorWrapper::new_internal(String::from("Failed to move items")));

    if let Some(_map) = &mut level.map {
        // Clone out the request fields so we can use them safely without moving the request
        let source = request.source.clone();
        let target = request.target.clone();
        let _to_mvoe = request.to_move.clone();

        // Source cannot be the target when moving to another container
        if target.is_targeting_another_container() && source.get_container().id_equals(&target.get_container().unwrap())  {
            return Err(ErrorWrapper::new_internal(String::from("Cannot move items. Source cannot be the target")));
        }

        if source.is_world_scope() && target.is_player_scope()  {
            return Err(ErrorWrapper::new_internal(String::from("Unsupported operation. Moving from world container to Player Inventory is supported by TakeItems instead.")));
        }

        let source_container = source.get_container().clone();
        if target.is_targeting_another_container()  {
            let live_target: &mut Container;
            match target {
                TargetContainerScope::WorldContainer(target) => {
                    live_target = level
                        .get_map_mut().unwrap()
                        .find_container_mut(&target.container, target.position)
                        .expect("Failed to find target container on the map");
                },
                TargetContainerScope::PlayerInventory(target) => {
                    let live_player_inventory =
                        level.get_player_mut().unwrap().get_inventory_mut();
                    // Check if we're targeting the top-most container (The Player inventory)
                    if live_player_inventory.get_self_item_id() == target.container.get_self_item_id()  {
                        live_target = live_player_inventory;
                    } else {
                        let live_target_match = live_player_inventory
                            .find_mut(target.container.get_self_item())
                            .expect("Failed to find target container in the Player's Inventory");
                        live_target = live_target_match;
                    }
                },
                _ => {
                    return Err(ErrorWrapper::new_internal(String::from("Unsupported operation")));
                }
            }

            let to_copy = find_copying_items(source_container, request.to_move.clone());
            let copy_result = copy_container_items(to_copy.clone(), live_target);
            log::info!("(Move) Copied {}/{} requested items", copy_result.copied.len(), request.to_move.len());
            let live_updated_target = live_target.clone();

            let moved = copy_result.copied.clone();
            let unmoved = copy_result.uncopied.clone();
            let updated_source = update_source_container(level, request.clone(), copy_result);

            let updated_source_scope;
            if request.source.is_world_scope()  {
                if let SourceContainerScope::WorldContainer(wc) = request.source.clone() {
                    updated_source_scope = SourceContainerScope::WorldContainer(
                        WorldContainer {
                            container: updated_source.clone(),
                            position: wc.position.clone()
                        }
                    )
                } else {
                    return Err(ErrorWrapper::new_internal(String::from("Couldn't convert source to SourceContainerScope::WorldContainer")));
                }
            } else {
                if let SourceContainerScope::PlayerInventory(_pic) = request.source.clone() {
                    updated_source_scope = SourceContainerScope::PlayerInventory(
                        PlayerInventoryContainer {
                            container: updated_source.clone()
                        }
                    )
                } else {
                    return Err(ErrorWrapper::new_internal(String::from("Couldn't convert source to SourceContainerScope::WorldContainer")));
                }
            }

            let updated_target_scope;
            if request.target.is_world_scope()  {
                if let TargetContainerScope::WorldContainer(wc) = request.target {
                    updated_target_scope = TargetContainerScope::WorldContainer(
                        WorldContainer {
                            container: live_updated_target,
                            position: wc.position.clone()
                        }
                    )
                } else {
                    return Err(ErrorWrapper::new_internal(String::from("Couldn't convert target")));
                }
            } else {
                if let TargetContainerScope::PlayerInventory(_pic) = request.target {
                    updated_target_scope = TargetContainerScope::PlayerInventory(
                        PlayerInventoryContainer {
                            container: live_updated_target.clone()
                        }
                    )
                } else {
                    return Err(ErrorWrapper::new_internal(String::from("Couldn't convert target")));
                }
            }

            log::info!("Returning MoveItemBetweenResponse with {} moved, {} unmoved items", moved.len(), unmoved.len());
            response = Ok(MoveItemsResponseV2 {
                request: request_result_copy,
                success: true,
                updated_scopes: UpdatedScopes {
                    source: updated_source_scope,
                    target: updated_target_scope
                },
                unmoved: unmoved.clone(),
                message: format!("Moved {}/{} items", moved.len(), unmoved.len())
            })
        } else {
            let move_response = move_items_to_source_position(request.clone());
            if move_response.is_none()  {
                return Err(ErrorWrapper::new_internal(String::from("Failed to move items to source position")));
            } else {
                response = Ok(move_response.unwrap());
            }
        }
    } else {
        return Err(ErrorWrapper::new_internal(String::from("Cannot move items. No map available.")));
    }

    // Not we've updated the target and made an updated copy of the source, we can live update the source container
    let live_source: &mut Container;
    match request.source {
        SourceContainerScope::WorldContainer(ref wc) => {
            live_source = level
                .get_map_mut().unwrap()
                .find_container_mut(&wc.container, wc.position)
                .expect("Failed to find target container on the map");
        }
        SourceContainerScope::PlayerInventory(source) => {
            let live_player_inventory = level.get_player_mut().unwrap().get_inventory_mut();
            // Check if we're targeting the top-most container (The Player inventory)
            if live_player_inventory.get_self_item_id() == source.container.get_self_item_id()  {
                live_source = live_player_inventory;
            } else {
                let live_source_match = live_player_inventory.get_contents_mut()
                    .iter_mut()
                    .find(|c| c.get_self_item().get_id() == source.container.get_self_item().get_id())
                    .expect("Failed to find source container in the Player's Inventory");
                live_source = live_source_match;
            }
        },
        _ => {
            return Err(ErrorWrapper::new_internal(String::from("Unsupported operation")));
        }
    }

    if let Ok(rs) = response {
        // Replace the source with our updated one
        *live_source = rs.updated_scopes.source.get_container().clone();
        return Ok(rs);
    } else {
        return response
    }
}

pub fn build_container_choices(_source: &Container, parent_scope: TargetContainerScope, parent_position: Position) -> Result<Vec<ContainerChoice>, io::Error> {
    let parent_container = parent_scope.get_container().unwrap();
    let parent_item_name = parent_scope.get_self_item().get_name();
    let mut sub_containers = parent_container.find_subcontainer_choices(parent_scope.clone(), parent_position);
    // Add the parent too
    sub_containers.push(
        ContainerChoice {
            container_scope: parent_scope,
            location_name: parent_item_name
        }
    );

    let mut idx = 0;
    for c in &sub_containers {
        let choice_item = c.container_scope.get_self_item();
        log::info!("{} - Available container: {}", idx, choice_item.get_name());
        idx +=1;
    }
    return Ok(sub_containers);
}

// Major test scenarios
// A1. Within a WorldContainer - Moving items/containers into another container
// A2. Within a WorldContainer - Moving items/containers from child to parent
// A3. Within a WorldContainer - Moving items/containers from parent to child
// A4. Within a WorldContainer - Moving items/containers to the bottom of the container
// A5. Within a WorldContainer - Moving items/containers to the top of the container
// A6. Within a WorldContainer - Moving item/container selection to the middle of the container
// A7. Within a WorldContainer - Moving split item/container selection into another container
// A8. Within a WorldContainer - Moving split item/container selection to a specific spot

// B1. Within PlayerInventory - Moving items/containers into another container
// B2. Within PlayerInventory - Moving items/containers from child to parent
// B3. Within PlayerInventory - Moving items/containers from parent to child
// B4. Within PlayerInventory - Moving items/containers to the bottom of the container
// B5. Within a PlayerInventory - Moving items/containers to the top of the container
// B6. Within a PlayerInventory - Moving item/container selection to the middle of the container
// B7. Within a PlayerInventory - Moving split item/container selection into another container
// B8. Within a PlayerInventory - Moving split item/container selection to a specific spot

// C1. Moving items/container from the PlayerInventory to World Container
// C2. Moving items/container from a World Container to the Player Inventory (should error as this is just TakeItems)

#[cfg(test)]
mod tests {
    use crate::engine::event::container::{MoveItemsRequestV2, MoveItemsWithinSourceRequest, PlayerInventoryContainer, PlayerInventoryItemPosition, SourceContainerScope, TargetContainerScope, WorldContainer, WorldContainerItemPosition};
    use crate::error::errors::ErrorType;
    use std::collections::HashMap;

    use uuid::Uuid;

    use crate::character::builder::character_builder::{CharacterBuilder, CharacterPattern};
    use crate::character::characters::Characters;
    use crate::engine::container_util::move_items;
    use crate::engine::level::Level;
    use crate::map::objects::container::{Container, ContainerType};
    use crate::map::objects::items::Item;
    use crate::map::position::{build_square_area, Position};
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
        return  Level { map: Some(map) , characters: Characters::new( Some(player), Vec::new())  };
    }

    fn build_player_test_level() -> Level {
        let tile_library = crate::map::tile::build_library();
        let rom = tile_library[&TileType::Room].clone();
        let wall = tile_library[&TileType::Wall].clone();
        let map_pos = Position { x: 0, y: 0 };
        let map_area = build_square_area(map_pos, 3);
        let map = crate::map::Map {
            area: map_area,
            tiles : Tiles { tiles : vec![
                vec![ wall.clone(), wall.clone(), wall.clone() ],
                vec![ wall.clone(), rom.clone(), wall.clone() ],
                vec![ wall.clone(), wall.clone(), wall.clone() ],
            ]},
            rooms: Vec::new(),
            containers: HashMap::new()
        };
        let player_pattern_result = CharacterPattern::new_player();
        assert!(player_pattern_result.is_ok(), "Failed to build player CharacterPattern!");
        let player =  CharacterBuilder::new(player_pattern_result.unwrap())
            .build(String::from("Test Player"));
        return  Level { map: Some(map) , characters: Characters::new( Some(player), Vec::new())  };
    }

    fn validate_player_inventory(player_inventory: &Container) {
        let player_inventory_contents= player_inventory.get_contents();
        assert_eq!(62, player_inventory_contents.len());
        assert_eq!("Bag", player_inventory_contents.get(0).unwrap().get_self_item().get_name());

        let bag = player_inventory_contents.get(0).unwrap().clone();
        let bag_contents = bag.get_contents();
        assert_eq!(2, bag_contents.len());
        assert_eq!("Carton", bag_contents.get(0).unwrap().get_self_item().get_name());
        assert_eq!("Bronze Bar", bag_contents.get(1).unwrap().get_self_item().get_name());

        assert_eq!("Test Item 1", player_inventory_contents.get(1).unwrap().get_self_item().get_name());
        assert_eq!("Test Item 2", player_inventory_contents.get(2).unwrap().get_self_item().get_name());
        assert_eq!("Test Item 3", player_inventory_contents.get(3).unwrap().get_self_item().get_name());

        // AND so on until item 60
        assert_eq!("Test Item 60", player_inventory_contents.get(60).unwrap().get_self_item().get_name());
        assert_eq!("Steel Arming Sword", player_inventory_contents.get(61).unwrap().get_self_item().get_name());
    }

    #[test]
    #[allow(non_snake_case)]
    // A1. Within a WorldContainer - Moving items/containers into another container
    fn MoveItems_A1_WorldContainer_MoveItemsToAnotherContainer() {
        // GIVEN a World Container Chest containing 2 containers and 2 items
        let mut chest = Container::new(Uuid::new_v4(), "Chest".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 600);
        let container1 = Container::new(Uuid::new_v4(), "Test Container 1".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 600);
        let container2 = Container::new(Uuid::new_v4(), "Test Container 2".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 600);
        let item1 = Container::wrap(Item::with_defaults(String::from("Test Item 1"), 1.0, 600));
        let item2 = Container::wrap(Item::with_defaults(String::from("Test Item 2"), 1.0, 600));

        chest.push(vec![
            container1.clone(),
            container2.clone(),
            item1.clone(),
            item2.clone()
        ]);
        assert_eq!(4, chest.get_total_count());

        // And we want to move Container 1 and Item 1 into Container 2
        let to_move = vec![
            container1.get_self_item().clone(),
            item1.get_self_item().clone()
        ];
        let source = chest.clone();
        let container_pos =  Position { x: 1, y: 1};

        // AND all of this is present in a level
        let mut level = build_test_level(container_pos.clone(), chest.clone());

        // WHEN we call to these into container 2
        let request = MoveItemsRequestV2 {
            source: SourceContainerScope::WorldContainer(
                WorldContainer {
                    container: chest.clone(),
                    position: container_pos
                }
            ),
            target: TargetContainerScope::WorldContainer(
                WorldContainer {
                    container: container2.clone(),
                    position: container_pos
                }
            ),
            to_move: to_move
        };

        let result = move_items(request.clone(), &mut level);

        // THEN we expect a result to return
        assert!(result.is_ok());

        // THEN we expect a valid result
        if let Ok(response) = result {
            assert_eq!(true, response.success);
            assert!(response.unmoved.is_empty());

            // And the 'target' (Container 2) container will have the item now
            let target_container = &request.target.get_container().unwrap();
            let updated_target = level.get_map_mut().unwrap().find_container_mut(target_container, container_pos);
            if let Some(c) = updated_target {
                // There should be 2 items in container
                assert_eq!(2, c.get_total_count());
                // And it should be our Container 1 and Test Item 1
                let first_item = c.get_contents()[0].clone();
                assert_eq!(first_item, container1);

                let second_item = c.get_contents()[1].clone();
                assert_eq!(second_item, item1);
            }

            // AND The map 'source' container will have the item removed
            let updated_source = level.get_map_mut().unwrap().find_container_mut(&request.source.get_container(), container_pos);
            if let Some(c) = updated_source {
                // There should be 2 items in to root container's top level
                assert_eq!(2, c.get_top_level_count());

                // AND these should be Container 2 and Test Item 2
                // And it should be our Container 1 and Test Item 1
                let first_item = c.get_contents()[0].get_self_item().clone();
                assert_eq!(first_item, container2.get_self_item().clone());

                let second_item = c.get_contents()[1].get_self_item().clone();
                assert_eq!(second_item, item2.get_self_item().clone());
                return; // pass
            }
        }
        assert!(false);
    }

    #[test]
    #[allow(non_snake_case)]
    // A2. Within a WorldContainer - Moving items/containers from child to parent
    fn MoveItems_A2_WorldContainer_MoveItemsFromChildToParent() {
        // GIVEN a valid map

        // AND a chest that contains a nested bag and carton
        // Our container Hierarchy should now be
        //  Chest -> Bag -> Carton
        // Chest contents: [Test Item 1, Test Container 1]
        let mut chest = Container::new(Uuid::new_v4(), "Chest".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 1000000);
        // AND each of them contain some other items
        let item1 = Container::wrap(Item::with_defaults(String::from("Test Item 1"), 1.0, 100));
        let container1 = Container::new(Uuid::new_v4(), "Test Container 1".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 600);
        chest.push(vec![item1.clone(), container1.clone()]);

        // Bag contents: [Test Item 2, Test Container 2]
        let mut bag = Container::new(Uuid::new_v4(), "Bag".to_owned(), 'X', 5.0, 1, ContainerType::OBJECT, 600);
        let item2 = Container::wrap(Item::with_defaults(String::from("Test Item 2"), 1.0, 100));
        let container2 = Container::new(Uuid::new_v4(), "Test Container 2".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 600);
        bag.push(vec![item2.clone(), container2.clone()]);

        // Carton contents: [Test Item 3, Test Container 3]
        let mut carton = Container::new(Uuid::new_v4(), "Carton".to_owned(), 'X', 5.0, 1, ContainerType::OBJECT, 50);
        let item3 = Container::wrap(Item::with_defaults(String::from("Test Item 3"), 1.0, 100));
        // The container has to be small enough to fit in the Bag
        let container3 = Container::new(Uuid::new_v4(), "Test Container 3".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        carton.push(vec![item3.clone(), container3.clone()]);

        bag.push(vec![carton.clone()]);
        assert_eq!(3, bag.get_top_level_count());

        chest.push(vec![bag.clone()]);
        assert_eq!(3, chest.get_top_level_count());

        // AND all of this is stored in a test level
        let container_pos = Position { x: 1, y: 1 };
        let mut level = build_test_level(container_pos, chest);

        // WHEN we call to move items
        // - from the lowest container (Carton's Test Iem 3 and Test Container 3)
        // - into the parent container (Bag)
        let to_move = vec![item3.get_self_item().clone(), container3.get_self_item().clone()];
        let request = MoveItemsRequestV2 {
            source: SourceContainerScope::WorldContainer(
                WorldContainer {
                    container: carton.clone(),
                    position: container_pos
                }
            ),
            target: TargetContainerScope::WorldContainer(
                WorldContainer {
                    container: bag.clone(),
                    position: container_pos
                }
            ),
            to_move: to_move
        };
        let result = move_items(request.clone(), &mut level);

        // THEN we expect a result to return
        assert!(result.is_ok());

        // THEN we expect a result that confirms this
        if let Ok(response) = result {
            assert_eq!(true, response.success);
            assert_eq!(0, response.unmoved.len());

            // And the response should return a copy of the updated source container
            assert_eq!(response.updated_scopes.source.get_self_item().get_id(), carton.get_self_item().get_id());
            // the carton should now be empty
            assert_eq!(0, response.updated_scopes.source.get_container().get_top_level_count());

            // AND the map will be updated to reflect this
            // Find the Carton
            let source_updated = level.get_map_mut().unwrap().find_container_mut(&carton, container_pos);
            // the carton should now be empty
            assert_eq!(0, source_updated.unwrap().get_top_level_count());

            // Find the bag, which should contain:
            // 1. Test Item 2
            // 2. Test Container 2
            // 3. the now Empty Carton
            // 4. Test Item 3
            // 5. Test Container 3
            let target_container = &request.target.get_container().unwrap();
            let mut target_updated = level.get_map_mut().unwrap().find_container_mut(target_container, container_pos);
            assert_eq!(5, target_updated.as_ref().unwrap().get_top_level_count());

            let target_contents = target_updated.as_mut().unwrap().get_contents();
            assert_eq!(item2.get_self_item_id(), target_contents.get(0).unwrap().get_self_item_id());
            assert_eq!(container2.get_self_item_id(), target_contents.get(1).unwrap().get_self_item_id());
            assert_eq!(carton.get_self_item_id(), target_contents.get(2).unwrap().get_self_item_id());
            assert_eq!(item3.get_self_item_id(), target_contents.get(3).unwrap().get_self_item_id());
            assert_eq!(container3.get_self_item_id(), target_contents.get(4).unwrap().get_self_item_id());
            return;
        }

        // Fail if we don't hit our logic
        assert!(false);
    }

    #[test]
    #[allow(non_snake_case)]
    // A3. Within a WorldContainer - Moving items/containers from parent to child
    fn MoveItems_A3_WorldContainer_MoveItemsFromParentToChild() {
        // GIVEN a valid map

        // AND a chest that contains a nested bag and carton
        // Our container Hierarchy should now be
        //  Chest -> Bag -> Carton
        // Chest contents: [Test Item 1, Test Container 1]
        let mut chest = Container::new(Uuid::new_v4(), "Chest".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 1000000);
        // AND each of them contain some other items
        let item1 = Container::wrap(Item::with_defaults(String::from("Test Item 1"), 1.0, 100));
        let container1 = Container::new(Uuid::new_v4(), "Test Container 1".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 600);
        chest.push(vec![item1.clone(), container1.clone()]);

        // Bag contents: [Test Item 2, Test Container 2]
        let mut bag = Container::new(Uuid::new_v4(), "Bag".to_owned(), 'X', 5.0, 1, ContainerType::OBJECT, 600);
        let item2 = Container::wrap(Item::with_defaults(String::from("Test Item 2"), 1.0, 100));
        let container2 = Container::new(Uuid::new_v4(), "Test Container 2".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 600);
        bag.push(vec![item2.clone(), container2.clone()]);

        // Carton contents: [Test Item 3, Test Container 3]
        let mut carton = Container::new(Uuid::new_v4(), "Carton".to_owned(), 'X', 5.0, 1, ContainerType::OBJECT, 500);
        let item3 = Container::wrap(Item::with_defaults(String::from("Test Item 3"), 1.0, 100));
        // The container has to be small enough to fit in the Bag
        let container3 = Container::new(Uuid::new_v4(), "Test Container 3".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        carton.push(vec![item3.clone(), container3.clone()]);

        bag.push(vec![carton.clone()]);
        assert_eq!(3, bag.get_top_level_count());

        chest.push(vec![bag.clone()]);
        assert_eq!(3, chest.get_top_level_count());

        // AND all of this is stored in a test level
        let container_pos = Position { x: 1, y: 1 };
        let mut level = build_test_level(container_pos, chest);

        // WHEN we call to move items
        // - from the first child container (Bags's Test Iem 2 and Test Container 2)
        // - into the child container (Carton)
        let to_move = vec![item2.get_self_item().clone(), container2.get_self_item().clone()];
        let request = MoveItemsRequestV2 {
            source: SourceContainerScope::WorldContainer(
                WorldContainer {
                    container: bag.clone(),
                    position: container_pos
                }
            ),
            target: TargetContainerScope::WorldContainer(
                WorldContainer {
                    container: carton.clone(),
                    position: container_pos
                }
            ),
            to_move: to_move
        };
        let result = move_items(request.clone(), &mut level);

        // THEN we expect a result to return
        assert!(result.is_ok());

        // THEN we expect a result that confirms this
        if let Ok(response) = result {
            assert_eq!(true, response.success);
            assert_eq!(0, response.unmoved.len());

            // -- Check the response source
            // And the response should return a copy of the updated source container
            assert_eq!(bag.get_self_item().get_id(), response.updated_scopes.source.get_self_item().get_id());
            assert_eq!("Bag", response.updated_scopes.source.get_self_item().get_name());
            // the Bag should now have 1 item inside (The Carton)
            let updated_source_contents = response.updated_scopes.source.get_container().get_contents();
            assert_eq!(1, updated_source_contents.len());
            assert_eq!("Carton", updated_source_contents.get(0).unwrap().get_self_item().get_name());

            // -- Check the response target
            // And the response should return a copy of the updated target container (the Carton)
            assert_eq!("Carton", response.updated_scopes.target.get_self_item().get_name());
            // the Carton should now have 4 items inside:
            // Test Item 3
            // Test Container 3
            // Test Item 2 (moved)
            // Test Container 2 (moved)
            assert_eq!(4, response.updated_scopes.target.get_container().unwrap().get_top_level_count());
            let updated_target_contents = response.updated_scopes.target.get_container().unwrap().get_contents().clone();
            assert_eq!("Test Item 3", updated_target_contents.get(0).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 3", updated_target_contents.get(1).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 2", updated_target_contents.get(2).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 2", updated_target_contents.get(3).unwrap().get_self_item().get_name());

            // AND the map will be updated to reflect this
            // -- Check the map source
            let source_updated = level.get_map_mut().unwrap().find_container_mut(&bag, container_pos).expect("Failed to find source container on the map");
            assert_eq!(bag.get_self_item().get_id(), source_updated.get_self_item().get_id());
            assert_eq!("Bag", source_updated.get_self_item().get_name());
            // the Bag should now have 1 item inside (The Carton)
            let updated_source_contents = source_updated.get_contents();
            assert_eq!(1, updated_source_contents.len());
            assert_eq!("Carton", updated_source_contents.get(0).unwrap().get_self_item().get_name());

            // -- Check the map target
            let target_updated = level.get_map_mut().unwrap().find_container_mut(&carton, container_pos).expect("Failed to find target container on the map");
            assert_eq!(4, target_updated.get_top_level_count());
            let updated_target_contents = target_updated.get_contents().clone();
            assert_eq!("Test Item 3", updated_target_contents.get(0).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 3", updated_target_contents.get(1).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 2", updated_target_contents.get(2).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 2", updated_target_contents.get(3).unwrap().get_self_item().get_name());

            return;
        }

        // Fail if we don't hit our logic
        assert!(false);


        // Fail if we don't hit our logic
        assert!(false);
    }

    #[test]
    #[allow(non_snake_case)]
    // A4. Within a WorldContainer - Moving items/containers to the bottom of the container
    fn MoveItems_A4_WorldContainer_MoveItemsToBottom() {
        // GIVEN a valid map
        // that holds a Chest containing 6 containers (Each with a unique name)
        let mut chest = Container::new(Uuid::new_v4(), "Chest".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container1 = Container::new(Uuid::new_v4(), "Test Container 1".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container2 = Container::new(Uuid::new_v4(), "Test Container 2".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container3 = Container::new(Uuid::new_v4(), "Test Container 3".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container4 = Container::new(Uuid::new_v4(), "Test Container 4".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container5 = Container::new(Uuid::new_v4(), "Test Container 5".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container6 = Container::new(Uuid::new_v4(), "Test Container 6".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);

        // Clone everything before moving
        let to_move = vec![container1.get_self_item().clone(), container2.get_self_item().clone()];
        chest.push(vec![container1.clone(), container2.clone(), container3.clone(),  container4.clone(),  container5.clone(), container6.clone()], );
        let source_copy = chest.clone();
        assert_eq!(6, chest.get_total_count());

        let container_pos =  Position { x: 1, y: 1};

        let mut level = build_test_level(container_pos, chest.clone());

        // WHEN we call to move container 1 and 2 to the bottom of the list (Container 6's location)
        let bottom_item_target = chest.get(5).get_self_item().clone();
        let _expected_target = bottom_item_target.clone();
        let request = MoveItemsRequestV2 {
            source: SourceContainerScope::WorldContainer(
                WorldContainer {
                    container: chest.clone(),
                    position: container_pos
                }
            ),
            target: TargetContainerScope::WorldContainerItemPosition(
                WorldContainerItemPosition {
                    target_position_item: bottom_item_target,
                    position: container_pos
                }
            ),
            to_move: to_move
        };

        let result = move_items(request.clone(), &mut level);

        // THEN we expect a result to return
        assert!(result.is_ok());

        // AND we expect a valid result
        if let Ok(response) = result {
            assert!(response.all_items_moved());

            // AND the source will have been updated, with container 1 and 2 above the bottom item
            let updated_source_contents = response.updated_scopes.source.get_container().get_contents();
            assert_eq!(6, updated_source_contents.len());
            assert_eq!("Test Container 3", updated_source_contents.get(0).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 4", updated_source_contents.get(1).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 5", updated_source_contents.get(2).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 1", updated_source_contents.get(3).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 2", updated_source_contents.get(4).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 6", updated_source_contents.get(5).unwrap().get_self_item().get_name());

            let updated_target = response.updated_scopes.target;

            let map_source_contents = level.get_map_mut().unwrap().find_container_mut(&response.updated_scopes.source.get_container(), container_pos).unwrap().get_contents();
            assert_eq!(6, map_source_contents.len());
            assert_eq!("Test Container 3", map_source_contents.get(0).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 4", map_source_contents.get(1).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 5", map_source_contents.get(2).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 1", map_source_contents.get(3).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 2", map_source_contents.get(4).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 6", map_source_contents.get(5).unwrap().get_self_item().get_name());
            return; // pass
        }

        // Fail if we don't hit our logic
        assert!(false)
    }

    #[test]
    #[allow(non_snake_case)]
    // A5. Within a WorldContainer - Moving items/containers to the top of the container
    fn MoveItems_A5_WorldContainer_MoveItemsToTop() {
        // GIVEN a valid map
        // that holds a Chest containing 6 containers (Each with a unique name)
        let mut chest = Container::new(Uuid::new_v4(), "Chest".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container1 = Container::new(Uuid::new_v4(), "Test Container 1".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container2 = Container::new(Uuid::new_v4(), "Test Container 2".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container3 = Container::new(Uuid::new_v4(), "Test Container 3".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container4 = Container::new(Uuid::new_v4(), "Test Container 4".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container5 = Container::new(Uuid::new_v4(), "Test Container 5".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container6 = Container::new(Uuid::new_v4(), "Test Container 6".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);

        // Clone everything before moving
        let to_move = vec![container5.get_self_item().clone(), container6.get_self_item().clone()];
        chest.push(vec![container1.clone(), container2.clone(), container3.clone(),  container4.clone(),  container5.clone(), container6.clone()], );
        let source_copy = chest.clone();
        assert_eq!(6, chest.get_total_count());

        let container_pos =  Position { x: 1, y: 1};

        let mut level = build_test_level(container_pos, chest.clone());

        // WHEN we call to move container 5 and 6 to the top of the list (Container 1's location)
        let top_item_target = chest.get(0).get_self_item().clone();
        let _expected_target = top_item_target.clone();
        let request = MoveItemsRequestV2 {
            source: SourceContainerScope::WorldContainer(
                WorldContainer {
                    container: chest.clone(),
                    position: container_pos
                }
            ),
            target: TargetContainerScope::WorldContainerItemPosition(
                WorldContainerItemPosition {
                    target_position_item: top_item_target,
                    position: container_pos
                }
            ),
            // Container 5 and 6, cloned earlier
            to_move: to_move
        };

        let result = move_items(request.clone(), &mut level);

        // THEN we expect a valid result
        if let Ok(response) = result {
            assert!(response.all_items_moved());

            // AND the source will have been updated, with container 1 and 2 above the top item
            let updated_source_contents = response.updated_scopes.source.get_container().get_contents();
            assert_eq!(6, updated_source_contents.len());
            assert_eq!("Test Container 5", updated_source_contents.get(0).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 6", updated_source_contents.get(1).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 1", updated_source_contents.get(2).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 2", updated_source_contents.get(3).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 3", updated_source_contents.get(4).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 4", updated_source_contents.get(5).unwrap().get_self_item().get_name());

            let updated_target = response.updated_scopes.target;

            let map_source_contents = level.get_map_mut().unwrap().find_container_mut(&response.updated_scopes.source.get_container(), container_pos).unwrap().get_contents();
            assert_eq!(6, map_source_contents.len());
            assert_eq!("Test Container 5", map_source_contents.get(0).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 6", map_source_contents.get(1).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 1", map_source_contents.get(2).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 2", map_source_contents.get(3).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 3", map_source_contents.get(4).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 4", map_source_contents.get(5).unwrap().get_self_item().get_name());
            return; // pass
        }

        // Fail if we don't hit our logic
        assert!(false)
    }

    #[test]
    #[allow(non_snake_case)]
    // A6. Within a WorldContainer - Moving item/container selection to the middle of the container
    fn MoveItems_A6_WorldContainer_MoveItemsToMiddle() {
        // GIVEN a valid map
        // that holds a Chest containing 6 containers (Each with a unique name)
        let mut chest = Container::new(Uuid::new_v4(), "Chest".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container1 = Container::new(Uuid::new_v4(), "Test Container 1".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container2 = Container::new(Uuid::new_v4(), "Test Container 2".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container3 = Container::new(Uuid::new_v4(), "Test Container 3".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container4 = Container::new(Uuid::new_v4(), "Test Container 4".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container5 = Container::new(Uuid::new_v4(), "Test Container 5".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container6 = Container::new(Uuid::new_v4(), "Test Container 6".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);

        // Clone everything before moving
        chest.push(vec![container1.clone(), container2.clone(), container3.clone(),  container4.clone(),  container5.clone(), container6.clone()], );
        let source_copy = chest.clone();
        assert_eq!(6, chest.get_total_count());

        let container_pos =  Position { x: 1, y: 1};

        let mut level = build_test_level(container_pos, chest.clone());

        // WHEN we call to move container 1 to the middle of the list (Container 3's location)
        let to_move = vec![container1.get_self_item().clone()];
        let middle_item_target = chest.get(2).get_self_item().clone();
        let _expected_target = middle_item_target.clone();
        let request = MoveItemsRequestV2 {
            source: SourceContainerScope::WorldContainer(
                WorldContainer {
                    container: chest.clone(),
                    position: container_pos
                }
            ),
            target: TargetContainerScope::WorldContainerItemPosition(
                WorldContainerItemPosition {
                    target_position_item: middle_item_target,
                    position: container_pos
                }
            ),
            to_move: to_move
        };

        let result = move_items(request.clone(), &mut level);

        // THEN we expect a valid result
        if let Ok(response) = result {
            assert!(response.all_items_moved());

            // AND the source will have been updated, with container 1 2nd in the list (above the target middle item)
            let updated_source_contents = response.updated_scopes.source.get_container().get_contents();
            assert_eq!(6, updated_source_contents.len());
            assert_eq!("Test Container 2", updated_source_contents.get(0).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 1", updated_source_contents.get(1).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 3", updated_source_contents.get(2).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 4", updated_source_contents.get(3).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 5", updated_source_contents.get(4).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 6", updated_source_contents.get(5).unwrap().get_self_item().get_name());

            let updated_target = response.updated_scopes.target;

            // AND the real map source will match this response
            let map_source_contents = level.get_map_mut().unwrap().find_container_mut(&response.updated_scopes.source.get_container(), container_pos).unwrap().get_contents();
            assert_eq!(6, map_source_contents.len());
            assert_eq!("Test Container 2", map_source_contents.get(0).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 1", map_source_contents.get(1).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 3", map_source_contents.get(2).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 4", map_source_contents.get(3).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 5", map_source_contents.get(4).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 6", map_source_contents.get(5).unwrap().get_self_item().get_name());
            return; // pass
        }

        // Fail if we don't hit our logic
        assert!(false)
    }

    #[test]
    #[allow(non_snake_case)]
    //A7. Within a WorldContainer - Moving split item/container selection into another container
    fn MoveItems_A7_WorldContainer_MovingSplitSelectionToAnotherContainer() {
        // GIVEN a valid map
        // that holds a Chest containing 6 containers (Each with a unique name)
        let mut chest = Container::new(Uuid::new_v4(), "Chest".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container1 = Container::new(Uuid::new_v4(), "Test Container 1".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container2 = Container::new(Uuid::new_v4(), "Test Container 2".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container3 = Container::new(Uuid::new_v4(), "Test Container 3".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container4 = Container::new(Uuid::new_v4(), "Test Container 4".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container5 = Container::new(Uuid::new_v4(), "Test Container 5".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container6 = Container::new(Uuid::new_v4(), "Test Container 6".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);

        // Clone everything before moving
        chest.push(vec![container1.clone(), container2.clone(), container3.clone(),  container4.clone(),  container5.clone(), container6.clone()], );
        let source_copy = chest.clone();
        assert_eq!(6, chest.get_total_count());

        let container_pos =  Position { x: 1, y: 1};

        let mut level = build_test_level(container_pos, chest.clone());

        // WHEN we call to move container 2 and 5 (completely split selections) to the middle container (Test Container 3)
        let to_move = vec![container2.get_self_item().clone(), container5.get_self_item().clone()];
        let container_target = chest.get(2).clone();
        let _expected_target = container_target.clone();
        let request = MoveItemsRequestV2 {
            source: SourceContainerScope::WorldContainer(
                WorldContainer {
                    container: chest.clone(),
                    position: container_pos
                }
            ),
            target: TargetContainerScope::WorldContainer(
                WorldContainer {
                    container: container_target.clone(),
                    position: container_pos
                }
            ),
            to_move: to_move
        };

        let result = move_items(request.clone(), &mut level);

        // THEN we expect a valid result
        if let Ok(response) = result {
            assert!(response.all_items_moved());

            // -- Verify response source
            // AND the source will have been updated, with container 2 and 5 missing
            let updated_source =  response.updated_scopes.source;
            assert_eq!("Chest", updated_source.get_self_item().get_name());
            let updated_source_contents = updated_source.get_container().get_contents();
            assert_eq!(4, updated_source_contents.len());
            assert_eq!("Test Container 1", updated_source_contents.get(0).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 3", updated_source_contents.get(1).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 4", updated_source_contents.get(2).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 6", updated_source_contents.get(3).unwrap().get_self_item().get_name());

            // AND Test Container 3 will now contain container's 2 and 5
            let source_container_3 = updated_source_contents.get(1).unwrap();
            assert_eq!(2, source_container_3.get_contents().len());
            assert_eq!("Test Container 2", source_container_3.get(0).get_self_item().get_name());
            assert_eq!("Test Container 5", source_container_3.get(1).get_self_item().get_name());

            // -- Verify response target
            let updated_target = response.updated_scopes.target.get_container().unwrap();
            assert_eq!("Test Container 3", updated_target.get_self_item().get_name());
            assert_eq!(2, updated_target.get_contents().len());
            assert_eq!("Test Container 2", updated_target.get(0).get_self_item().get_name());
            assert_eq!("Test Container 5", updated_target.get(1).get_self_item().get_name());

            let map = level.get_map().unwrap();

            // -- Verify map source
            let map_source_contents = map.find_container(&updated_source.get_container(), container_pos).unwrap().get_contents();
            assert_eq!(4, map_source_contents.len());
            assert_eq!("Test Container 1", map_source_contents.get(0).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 3", map_source_contents.get(1).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 4", map_source_contents.get(2).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 6", map_source_contents.get(3).unwrap().get_self_item().get_name());

            let map_target_contents = map.find_container(&response.updated_scopes.target.get_container().unwrap(), container_pos).unwrap().get_contents();
            assert_eq!(2, map_target_contents.len());
            assert_eq!("Test Container 2", map_target_contents.get(0).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 5", map_target_contents.get(1).unwrap().get_self_item().get_name());

            return; // pass
        }

        // Fail if we don't hit our logic
        assert!(false)
    }

    #[test]
    #[allow(non_snake_case)]
    // A8. Within a WorldContainer - Moving split item/container selection to a specific spot
    fn MoveItems_A8_WorldContainer_MovingSplitSelectionToSpot() {
        // GIVEN a valid map
        // that holds a Chest containing 6 containers (Each with a unique name)
        let mut chest = Container::new(Uuid::new_v4(), "Chest".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container1 = Container::new(Uuid::new_v4(), "Test Container 1".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container2 = Container::new(Uuid::new_v4(), "Test Container 2".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container3 = Container::new(Uuid::new_v4(), "Test Container 3".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container4 = Container::new(Uuid::new_v4(), "Test Container 4".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container5 = Container::new(Uuid::new_v4(), "Test Container 5".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container6 = Container::new(Uuid::new_v4(), "Test Container 6".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);

        // Clone everything before moving
        chest.push(vec![container1.clone(), container2.clone(), container3.clone(),  container4.clone(),  container5.clone(), container6.clone()], );
        let source_copy = chest.clone();
        assert_eq!(6, chest.get_total_count());

        let container_pos =  Position { x: 1, y: 1};

        let mut level = build_test_level(container_pos, chest.clone());

        // WHEN we call to move container 2 and 5 (completely split selections) to the middle of the list (Container 3's location)
        let to_move = vec![container2.get_self_item().clone(), container5.get_self_item().clone()];
        let middle_item_target = chest.get(2).get_self_item().clone();
        let _expected_target = middle_item_target.clone();
        let request = MoveItemsRequestV2 {
            source: SourceContainerScope::WorldContainer(
                WorldContainer {
                    container: chest.clone(),
                    position: container_pos
                }
            ),
            target: TargetContainerScope::WorldContainerItemPosition(
                WorldContainerItemPosition {
                    target_position_item: middle_item_target,
                    position: container_pos
                }
            ),
            to_move: to_move
        };

        let result = move_items(request.clone(), &mut level);

        // THEN we expect a valid result
        if let Ok(response) = result {
            assert!(response.all_items_moved());

            // AND the source will have been updated, with container 1 2nd in the list (above the target middle item)
            let updated_source_contents = response.updated_scopes.source.get_container().get_contents();
            assert_eq!(6, updated_source_contents.len());
            assert_eq!("Test Container 1", updated_source_contents.get(0).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 2", updated_source_contents.get(1).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 5", updated_source_contents.get(2).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 3", updated_source_contents.get(3).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 4", updated_source_contents.get(4).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 6", updated_source_contents.get(5).unwrap().get_self_item().get_name());

            let updated_target = response.updated_scopes.target;

            // AND the real map source will match this response
            let map_source_contents = level.get_map_mut().unwrap().find_container_mut(&response.updated_scopes.source.get_container(), container_pos).unwrap().get_contents();
            assert_eq!(6, map_source_contents.len());
            assert_eq!("Test Container 1", map_source_contents.get(0).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 2", map_source_contents.get(1).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 5", map_source_contents.get(2).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 3", map_source_contents.get(3).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 4", map_source_contents.get(4).unwrap().get_self_item().get_name());
            assert_eq!("Test Container 6", map_source_contents.get(5).unwrap().get_self_item().get_name());
            return; // pass
        }

        // Fail if we don't hit our logic
        assert!(false)
    }

    #[test]
    #[allow(non_snake_case)]
    // B1. Within PlayerInventory - Moving items/containers into another container
    fn MoveItems_B1_PlayerInventory_MoveItemsToAnotherContainer() {
        // GIVEN a player focused test level (which has a player and their inventory)
        let mut level = build_player_test_level();

        let player = level.characters.get_player().unwrap();

        // AND the player inventory is in the expected state
        let player_inventory = player.get_inventory();
        validate_player_inventory(player_inventory);
        let player_inventory_contents= player_inventory.get_contents();
        let bag = player_inventory_contents.get(0).unwrap().clone();

        // WHEN we call to move test items 1,2,and 3 into the Bag
        let test_item_1 = player_inventory_contents.get(1).unwrap().clone();
        let test_item_2 = player_inventory_contents.get(2).unwrap().clone();
        let test_item_3 = player_inventory_contents.get(3).unwrap().clone();
        let request = MoveItemsRequestV2 {
            source: SourceContainerScope::PlayerInventory(
                PlayerInventoryContainer {
                    container: player_inventory.clone()
                }
            ),
            target: TargetContainerScope::PlayerInventory(
                PlayerInventoryContainer {
                    container: bag.clone()
                }
            ),
            to_move: vec![
                test_item_1.get_self_item().clone(),
                test_item_2.get_self_item().clone(),
                test_item_3.get_self_item().clone()
            ]
        };

        let result = move_items(request.clone(), &mut level);

        // THEN we expect a result to return
        assert!(result.is_ok());

        // AND we expect a valid result
        if let Ok(response) = result {
            assert!(response.all_items_moved());

            // AND the source will have been updated, with test items 1-3 moved out
            let updated_source_contents = response.updated_scopes.source.get_container().get_contents();
            assert_eq!(59, updated_source_contents.len());
            // The top items are now Bag and then items 4-60
            assert_eq!("Bag", updated_source_contents.get(0).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 4", updated_source_contents.get(1).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 5", updated_source_contents.get(2).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 6", updated_source_contents.get(3).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 60", updated_source_contents.get(57).unwrap().get_self_item().get_name());
            assert_eq!("Steel Arming Sword", updated_source_contents.get(58).unwrap().get_self_item().get_name());

            // AND the real Player's inventory (the source) should be updated as well
            let real_player_inventory = level.get_player().unwrap().get_inventory();
            // The top items are now Bag and then items 4-60
            assert_eq!("Bag", real_player_inventory.get(0).get_self_item().get_name());
            assert_eq!("Test Item 4", real_player_inventory.get(1).get_self_item().get_name());
            assert_eq!("Test Item 5", real_player_inventory.get(2).get_self_item().get_name());
            assert_eq!("Test Item 6", real_player_inventory.get(3).get_self_item().get_name());
            assert_eq!("Test Item 60", real_player_inventory.get(57).get_self_item().get_name());
            assert_eq!("Steel Arming Sword", real_player_inventory.get(58).get_self_item().get_name());


            // AND the real Bag within the Player's inventory should match the response above
            let real_updated_bag_contents = real_player_inventory.get_contents().iter().find(|c| c.get_self_item().get_id() == request.target.get_container().unwrap().get_self_item().get_id())
                .expect("Failed to find target container in the Player's Inventory")
                .get_contents();
            assert_eq!(5, real_updated_bag_contents.len());
            assert_eq!("Carton", real_updated_bag_contents.get(0).unwrap().get_self_item().get_name());
            assert_eq!("Bronze Bar", real_updated_bag_contents.get(1).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 1", real_updated_bag_contents.get(2).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 2", real_updated_bag_contents.get(3).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 3", real_updated_bag_contents.get(4).unwrap().get_self_item().get_name());

            return; // pass
        }

        // Fail if we don't hit our logic
        assert!(false)
    }

    #[test]
    #[allow(non_snake_case)]
    // B2. Within PlayerInventory - Moving items/containers from child to parent
    fn MoveItems_B2_Player_Inventory_MovingItemsFromChildToParent() {
        // GIVEN a player focused test level (which has a player and their inventory)
        let mut level = build_player_test_level();

        let player = level.characters.get_player().unwrap();

        // AND the player inventory is in the expected state
        let player_inventory = player.get_inventory();
        validate_player_inventory(player_inventory);
        let player_inventory_contents= player_inventory.get_contents();

        // AND we have a Bag in the inventory that contains a Carton and Bronze Bar
        let bag = player_inventory_contents.get(0).unwrap().clone();
        let bag_contents = bag.get_contents();
        // Assert source size
        assert_eq!(2, bag_contents.len());
        assert_eq!("Carton", bag_contents.get(0).unwrap().get_self_item().get_name());
        assert_eq!("Bronze Bar", bag_contents.get(1).unwrap().get_self_item().get_name());

        // Assert target size (top-level player inventory size)
        assert_eq!(62, player_inventory.get_contents().len());

        // WHEN we call to move
        // the Carton and Bronze Bar
        // From the Bag into the top-level inventory
        let carton = bag_contents.get(0).unwrap().clone();
        let bronze_bar = bag_contents.get(1).unwrap().clone();
        let request = MoveItemsRequestV2 {
            source: SourceContainerScope::PlayerInventory(
                PlayerInventoryContainer {
                    container: bag.clone()
                }
            ),
            target: TargetContainerScope::PlayerInventory(
                PlayerInventoryContainer {
                    container: player_inventory.clone()
                }
            ),
            to_move: vec![
                carton.get_self_item().clone(),
                bronze_bar.get_self_item().clone()
            ]
        };

        let result = move_items(request.clone(), &mut level);

        // THEN we expect a successful result to return
        if let Ok(response) = result {
            assert!(response.all_items_moved());
            // AND the source Bag will have been updated, with the Carton and Bronze Bar removed
            let updated_source = response.updated_scopes.source;
            assert_eq!("Bag", updated_source.get_self_item().get_name());
            let updated_source_contents = updated_source.get_container().get_contents();
            // The source (Bag) should now be empty
            assert_eq!(0, updated_source_contents.len());

            // The target (inventory root) should have 2 more items
            let updated_target = response.updated_scopes.target;
            assert_eq!("Test Player's Inventory", updated_target.get_self_item().get_name());
            assert_eq!(64, updated_target.get_container().unwrap().get_contents().len());

            let real_player_inventory = level.get_player().unwrap().get_inventory();
            let real_player_inventory_contents = real_player_inventory.get_contents();
            assert_eq!(64, real_player_inventory_contents.len());

            // The top items are unchanged (Bag, Test Item 1, 2, etc)
            assert_eq!("Bag", real_player_inventory_contents.get(0).unwrap().get_self_item().get_name());
            // And the real Bag is now empty
            let bag = real_player_inventory_contents.get(0).unwrap().clone();
            let bag_contents = bag.get_contents();
            assert_eq!(0, bag_contents.len());
            // And everything following that is the test items..
            assert_eq!("Test Item 1", real_player_inventory_contents.get(1).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 2", real_player_inventory_contents.get(2).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 3", real_player_inventory_contents.get(3).unwrap().get_self_item().get_name());

            // And finally, at the bottom of the container are the Carton and Bronze Bar we moved
            assert_eq!("Carton", real_player_inventory_contents.get(62).unwrap().get_self_item().get_name());
            assert_eq!("Bronze Bar", real_player_inventory_contents.get(63).unwrap().get_self_item().get_name());

            return; // pass
        }

        // Fail if we don't hit our logic
        assert!(false)
    }

    #[test]
    #[allow(non_snake_case)]
    // B3. Within PlayerInventory - Moving items/containers from parent to child
    fn MoveItems_B3_PlayerInventory_MovingItemsFromParentToChild() {
        // GIVEN a player focused test level (which has a player and their inventory)
        let mut level = build_player_test_level();

        let player = level.characters.get_player().unwrap();

        // AND the player inventory is in the expected state
        let player_inventory = player.get_inventory();
        validate_player_inventory(player_inventory);
        let player_inventory_contents= player_inventory.get_contents();

        // AND we have a Bag in the inventory that contains a Carton and Bronze Bar
        let bag = player_inventory_contents.get(0).unwrap().clone();
        let bag_contents = bag.get_contents();
        // Assert source size
        assert_eq!(2, bag_contents.len());
        assert_eq!("Carton", bag_contents.get(0).unwrap().get_self_item().get_name());
        assert_eq!("Bronze Bar", bag_contents.get(1).unwrap().get_self_item().get_name());

        // AND GIVEN the hierarchy of:
        // Inventory -> Bag -> Carton
        // And the Bag contains the Carton and a Bronze Bar

        // WHEN we call to move the Bag's Bronze Bar (2nd item in the Bag) into the Carton
        let carton = bag_contents.get(0).unwrap().clone();
        let bronze_bar = bag_contents.get(1).unwrap().clone();
        let request = MoveItemsRequestV2 {
            source: SourceContainerScope::PlayerInventory(
                PlayerInventoryContainer {
                    container: bag.clone()
                }
            ),
            target: TargetContainerScope::PlayerInventory(
                PlayerInventoryContainer {
                    container: carton.clone()
                }
            ),
            to_move: vec![
                bronze_bar.get_self_item().clone()
            ]
        };

        let result = move_items(request.clone(), &mut level);

        // THEN we expect a successful result to return
        if let Ok(response) = result {
            assert!(response.all_items_moved());

            // -- Validate response source
            let updated_source = response.updated_scopes.source;
            assert_eq!("Bag", updated_source.get_self_item().get_name());
            assert_eq!(1, updated_source.get_container().get_contents().len());
            assert_eq!("Carton", updated_source.get_container().get_contents().get(0).unwrap().get_name());

            // -- Validate response target
            let updated_target = response.updated_scopes.target;
            assert_eq!("Carton", updated_target.get_container().unwrap().get_name());
            assert_eq!(2, updated_target.get_container().unwrap().get_contents().len());
            assert_eq!("Tin Bar", updated_target.get_container().unwrap().get(0).get_name());
            assert_eq!("Bronze Bar", updated_target.get_container().unwrap().get(1).get_name());

            // -- Validate map source (The Bag)
            let player = level.get_player().unwrap();
            let real_source = player.get_inventory().find(bag.get_self_item()).expect("Failed to find real source container");
            assert_eq!("Bag", real_source.get_name());
            assert_eq!(1, real_source.get_contents().len());
            assert_eq!("Carton", real_source.get_contents().get(0).unwrap().get_name());

            // -- Validate map target (The Carton)
            let real_target = player.get_inventory().find(carton.get_self_item()).expect("Failed to find real source container");
            assert_eq!("Carton", real_target.get_name());
            assert_eq!(2, real_target.get_contents().len());
            assert_eq!("Tin Bar", real_target.get(0).get_name());
            assert_eq!("Bronze Bar", real_target.get(1).get_name());

            return; // pass
        }

        // Fail if we don't hit our logic
        assert!(false)
    }

    #[test]
    #[allow(non_snake_case)]
    // B4. Within PlayerInventory - Moving items/containers to the bottom of the container
    fn MoveItems_B4_PlayerInventory_MovingItemsToBottom() {
        // GIVEN a player focused test level (which has a player and their inventory)
        let mut level = build_player_test_level();

        let player = level.characters.get_player().unwrap();

        // AND the player inventory is in the expected state
        let player_inventory = player.get_inventory();
        validate_player_inventory(player_inventory);
        let player_inventory_contents= player_inventory.get_contents();
        assert_eq!(62, player_inventory_contents.len());

        let test_item_1 = player_inventory_contents.get(1).unwrap();
        let test_item_2 = player_inventory_contents.get(2).unwrap();
        assert_eq!("Test Item 1", test_item_1.get_self_item().get_name());
        assert_eq!("Test Item 2", test_item_2.get_self_item().get_name());

        // WHEN we call to move
        // Test Item 1 and 2 near the top (in the top-level inventory)
        // To the bottom item's location
        let bottom_position_item = player_inventory_contents.get(61).unwrap().get_self_item();
        let request = MoveItemsRequestV2 {
            source: SourceContainerScope::PlayerInventory(
                PlayerInventoryContainer {
                    container: player_inventory.clone()
                }
            ),
            target: TargetContainerScope::PlayerInventoryItemPosition(
                PlayerInventoryItemPosition {
                    target_position_item: bottom_position_item.clone()
                }
            ),
            to_move: vec![
                test_item_1.get_self_item().clone(),
                test_item_2.get_self_item().clone()
            ]
        };

        let result = move_items(request.clone(), &mut level);

        // THEN we expect a successful result to return
        if let Ok(response) = result {
            assert!(response.all_items_moved());

            // AND the response source should be the Player's inventory
            let updated_source = response.updated_scopes.source;
            let updated_source_contents = updated_source.get_container().get_contents();
            assert!(updated_source.is_player_scope());
            assert_eq!("Test Player's Inventory", updated_source.get_self_item().get_name());
            // AND it's size should be unchanged
            assert_eq!(62, updated_source_contents.len());

            // AND the top of the source should now be Bag, then Test item 3 onwards
            assert_eq!("Bag", updated_source_contents.get(0).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 3", updated_source_contents.get(1).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 4", updated_source_contents.get(2).unwrap().get_self_item().get_name());

            // AND we should have Test Iem 1 and 2 above the last item in the source
            assert_eq!("Test Item 1", updated_source_contents.get(59).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 2", updated_source_contents.get(60).unwrap().get_self_item().get_name());
            assert_eq!("Steel Arming Sword", updated_source_contents.get(61).unwrap().get_self_item().get_name());

            // AND the updated target should be the item we specified earlier
            let updated_target = response.updated_scopes.target;
            assert!(updated_target.is_targeting_item_position());
            assert_eq!("Steel Arming Sword", updated_target.get_self_item().get_name());

            // AND the real player inventory in the level should match this
            let real_player_inventory = level.get_player().unwrap().get_inventory();
            let real_player_inventory_contents = real_player_inventory.get_contents();
            assert_eq!(62, real_player_inventory_contents.len());
            // AND the top of the inventory should now be Bag, then Test item 3 onwards
            assert_eq!("Bag", real_player_inventory_contents.get(0).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 3", real_player_inventory_contents.get(1).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 4", real_player_inventory_contents.get(2).unwrap().get_self_item().get_name());

            // AND we should have Test Iem 1 and 2 above the last item in the inventory
            assert_eq!("Test Item 1", real_player_inventory_contents.get(59).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 2", real_player_inventory_contents.get(60).unwrap().get_self_item().get_name());
            assert_eq!("Steel Arming Sword", real_player_inventory_contents.get(61).unwrap().get_self_item().get_name());

            return; // pass
        }
        // Fail if we don't hit our logic
        assert!(false)
    }


    #[test]
    #[allow(non_snake_case)]
    // B5. Within a PlayerInventory - Moving items/containers to the top of the container
    fn MoveItems_B5_PlayerInventory_MovingItemsToTop() {
        // GIVEN a player focused test level (which has a player and their inventory)
        let mut level = build_player_test_level();

        let player = level.characters.get_player().unwrap();

        // AND the player inventory is in the expected state
        let player_inventory = player.get_inventory();
        validate_player_inventory(player_inventory);
        let player_inventory_contents= player_inventory.get_contents();
        assert_eq!(62, player_inventory_contents.len());

        // WHEN we call to move
        // Test Item 59 and 60 near the bottom (in the top-level inventory)
        // To the top item's location (the Bag)
        let test_item_59 = player_inventory_contents.get(59).unwrap();
        let test_item_60 = player_inventory_contents.get(60).unwrap();
        assert_eq!("Test Item 59", test_item_59.get_self_item().get_name());
        assert_eq!("Test Item 60", test_item_60.get_self_item().get_name());
        let top_position_item = player_inventory_contents.get(0).unwrap().get_self_item();

        let request = MoveItemsRequestV2 {
            source: SourceContainerScope::PlayerInventory(
                PlayerInventoryContainer {
                    container: player_inventory.clone()
                }
            ),
            target: TargetContainerScope::PlayerInventoryItemPosition(
                PlayerInventoryItemPosition {
                    target_position_item: top_position_item.clone()
                }
            ),
            to_move: vec![
                test_item_59.get_self_item().clone(),
                test_item_60.get_self_item().clone()
            ]
        };

        let result = move_items(request.clone(), &mut level);

        // THEN we expect a successful result to return
        if let Ok(response) = result {
            assert!(response.all_items_moved());

            // AND the response source should be the Player's inventory
            let updated_source = response.updated_scopes.source;
            let updated_source_contents = updated_source.get_container().get_contents();
            assert!(updated_source.is_player_scope());
            assert_eq!("Test Player's Inventory", updated_source.get_self_item().get_name());
            // AND it's size should be unchanged
            assert_eq!(62, updated_source_contents.len());

            // AND the top of the source should now be Test item 59, 60, the Bag, and then Test Item 1 onwards
            assert_eq!("Test Item 59", updated_source_contents.get(0).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 60", updated_source_contents.get(1).unwrap().get_self_item().get_name());
            assert_eq!("Bag", updated_source_contents.get(2).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 1", updated_source_contents.get(3).unwrap().get_self_item().get_name());

            // AND we should have Test Iem 57, 58, and the Arming sword as the last items in the container
            assert_eq!("Test Item 57", updated_source_contents.get(59).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 58", updated_source_contents.get(60).unwrap().get_self_item().get_name());
            assert_eq!("Steel Arming Sword", updated_source_contents.get(61).unwrap().get_self_item().get_name());

            // AND the updated target should be the item we specified earlier
            let updated_target = response.updated_scopes.target;
            assert!(updated_target.is_targeting_item_position());
            assert_eq!("Bag", updated_target.get_self_item().get_name());

            // AND the real player inventory in the level should match this
            let real_player_inventory = level.get_player().unwrap().get_inventory();
            let real_player_inventory_contents = real_player_inventory.get_contents();
            assert_eq!(62, real_player_inventory_contents.len());
            // AND the top of the inventory should now be Bag, then Test item 3 onwards
            assert_eq!("Test Item 59", real_player_inventory_contents.get(0).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 60", real_player_inventory_contents.get(1).unwrap().get_self_item().get_name());
            assert_eq!("Bag", real_player_inventory_contents.get(2).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 1", real_player_inventory_contents.get(3).unwrap().get_self_item().get_name());

            // AND we should have Test Iem 57, 58, and the Arming sword as the last items in the container
            assert_eq!("Test Item 57", real_player_inventory_contents.get(59).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 58", real_player_inventory_contents.get(60).unwrap().get_self_item().get_name());
            assert_eq!("Steel Arming Sword", real_player_inventory_contents.get(61).unwrap().get_self_item().get_name());

            return; // pass
        }
        // Fail if we don't hit our logic
        assert!(false)
    }

    #[test]
    #[allow(non_snake_case)]
    // B6. Within a PlayerInventory - Moving item/container selection to the middle of the container
    fn MoveItems_B6_PlayerInventory_MovingItemsToMiddle() {
        // GIVEN a player focused test level (which has a player and their inventory)
        let mut level = build_player_test_level();

        let player = level.characters.get_player().unwrap();

        // AND the player inventory is in the expected state
        let player_inventory = player.get_inventory();
        validate_player_inventory(player_inventory);
        let player_inventory_contents= player_inventory.get_contents();
        assert_eq!(62, player_inventory_contents.len());

        // WHEN we call to move the first item (Bag) to the middle of the container
        let bag = player_inventory_contents.get(0).unwrap();
        assert_eq!("Bag", bag.get_self_item().get_name());
        let middle_position_item = player_inventory_contents.get(30).unwrap().get_self_item();

        let request = MoveItemsRequestV2 {
            source: SourceContainerScope::PlayerInventory(
                PlayerInventoryContainer {
                    container: player_inventory.clone()
                }
            ),
            target: TargetContainerScope::PlayerInventoryItemPosition(
                PlayerInventoryItemPosition {
                    target_position_item: middle_position_item.clone()
                }
            ),
            to_move: vec![
                bag.get_self_item().clone()
            ]
        };

        let result = move_items(request.clone(), &mut level);

        // THEN we expect a successful result to return
        if let Ok(response) = result {
            assert!(response.all_items_moved());

            // AND the response source should be the Player's inventory
            let updated_source = response.updated_scopes.source;
            let updated_source_contents = updated_source.get_container().get_contents();
            assert!(updated_source.is_player_scope());
            assert_eq!("Test Player's Inventory", updated_source.get_self_item().get_name());
            // AND it's size should be unchanged
            assert_eq!(62, updated_source_contents.len());

            // AND the top of the source should now be Test item 1, 2, onwards
            assert_eq!("Test Item 1", updated_source_contents.get(0).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 2", updated_source_contents.get(1).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 3", updated_source_contents.get(2).unwrap().get_self_item().get_name());

            // AND right in the middle should be the Bag and then Test Item 30 and Test Item 31
            // As moving moves the Bag above the target (Test Item 30)
            assert_eq!("Bag", updated_source_contents.get(29).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 30", updated_source_contents.get(30).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 31", updated_source_contents.get(31).unwrap().get_self_item().get_name());

            // AND we should have Test Iem 59, 60, and the Arming sword as the last items in the container
            assert_eq!("Test Item 59", updated_source_contents.get(59).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 60", updated_source_contents.get(60).unwrap().get_self_item().get_name());
            assert_eq!("Steel Arming Sword", updated_source_contents.get(61).unwrap().get_self_item().get_name());

            // AND the updated target should be the item we specified earlier
            let updated_target = response.updated_scopes.target;
            assert!(updated_target.is_targeting_item_position());
            assert_eq!("Test Item 30", updated_target.get_self_item().get_name());

            // AND the real player inventory in the level should match this
            let real_player_inventory = level.get_player().unwrap().get_inventory();
            let real_player_inventory_contents = real_player_inventory.get_contents();
            assert_eq!(62, real_player_inventory_contents.len());

            // AND the top of the inventory should now be Test item 1, 2, onwards
            assert_eq!("Test Item 1", real_player_inventory_contents.get(0).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 2", real_player_inventory_contents.get(1).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 3", real_player_inventory_contents.get(2).unwrap().get_self_item().get_name());

            // AND right in the middle should be the Bag and then Test Item 30 and Test Item 31
            // As moving moves the Bag above the target (Test Item 30)
            assert_eq!("Bag", real_player_inventory_contents.get(29).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 30", real_player_inventory_contents.get(30).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 31", real_player_inventory_contents.get(31).unwrap().get_self_item().get_name());

            // AND we should have Test Iem 59, 60, and the Arming sword as the last items in the inventory
            assert_eq!("Test Item 59", real_player_inventory_contents.get(59).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 60", real_player_inventory_contents.get(60).unwrap().get_self_item().get_name());
            assert_eq!("Steel Arming Sword", real_player_inventory_contents.get(61).unwrap().get_self_item().get_name());

            return; // pass
        }

        // Fail if we don't hit our logic
        assert!(false)
    }

    #[test]
    #[allow(non_snake_case)]
    // B7. Within a PlayerInventory - Moving split item/container selection into another container
    fn MoveItems_B7_PlayerInventory_MovingSplitSelectionToAnotherContainer() {
        // GIVEN a player focused test level (which has a player and their inventory)
        let mut level = build_player_test_level();

        let player = level.characters.get_player().unwrap();

        // AND the player inventory is in the expected state
        let player_inventory = player.get_inventory();
        validate_player_inventory(player_inventory);
        let player_inventory_contents= player_inventory.get_contents();
        assert_eq!(62, player_inventory_contents.len());

        // WHEN we call to move a split selection of:
        // 1. The first test item (Bag)
        // 2. The last item (Steel Arming Sword)
        // Into the Bag
        let test_item_1 = player_inventory_contents.get(1).unwrap();
        assert_eq!("Test Item 1", test_item_1.get_self_item().get_name());

        let steel_arming_sword = player_inventory_contents.get(61).unwrap();
        assert_eq!("Steel Arming Sword", steel_arming_sword.get_self_item().get_name());

        let bag = player_inventory_contents.get(0).unwrap();
        assert_eq!("Bag", bag.get_self_item().get_name());

        let request = MoveItemsRequestV2 {
            source: SourceContainerScope::PlayerInventory(
                PlayerInventoryContainer {
                    container: player_inventory.clone()
                }
            ),
            target: TargetContainerScope::PlayerInventory(
                PlayerInventoryContainer {
                    container: bag.clone()
                }
            ),
            to_move: vec![
                test_item_1.get_self_item().clone(),
                steel_arming_sword.get_self_item().clone()
            ]
        };

        let result = move_items(request.clone(), &mut level);

        // THEN we expect a successful result to return
        if let Ok(response) = result {
            assert!(response.all_items_moved());

            // -- Validate response source
            let updated_source = response.updated_scopes.source;
            assert_eq!(60, updated_source.get_container().get_contents().len());
            assert_eq!("Test Player's Inventory", updated_source.get_self_item().get_name());

            // And the first few items in the inventory will now be:
            // Bag
            let bag = updated_source.get_container().get(0);
            assert_eq!("Bag", bag.get_self_item().get_name());
            // Test Item 2
            let test_item_2 = updated_source.get_container().get(1);
            assert_eq!("Test Item 2", test_item_2.get_self_item().get_name());

            // And the last few items will be there apart from the Sword
            assert_eq!("Test Item 59", updated_source.get_container().get(58).get_self_item().get_name());
            assert_eq!("Test Item 60", updated_source.get_container().get(59).get_self_item().get_name());

            // -- Validate response target
            let updated_target = response.updated_scopes.target;
            assert_eq!("Bag", updated_target.get_self_item().get_name());
            assert_eq!(4, updated_target.get_container().unwrap().get_contents().len());
            let updated_target_contents = updated_target.get_container().unwrap();
            assert_eq!("Carton", updated_target_contents.get(0).get_self_item().get_name());
            assert_eq!("Bronze Bar", updated_target_contents.get(1).get_self_item().get_name());
            assert_eq!("Test Item 1", updated_target_contents.get(2).get_self_item().get_name());
            assert_eq!("Steel Arming Sword", updated_target_contents.get(3).get_self_item().get_name());


            let real_player_inventory = level.get_player().unwrap().get_inventory();
            let real_player_inventory_contents = real_player_inventory.get_contents();
            assert_eq!(60, real_player_inventory_contents.len());
            // -- Validate real inventory source
            assert_eq!("Bag", real_player_inventory_contents.get(0).unwrap().get_name());
            assert_eq!("Test Item 2", real_player_inventory_contents.get(1).unwrap().get_name());

            // And the last few items will be there apart from the Sword
            assert_eq!("Test Item 59", real_player_inventory_contents.get(58).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 60", real_player_inventory_contents.get(59).unwrap().get_self_item().get_name());

            // -- Validate real inventory target
            let real_target = real_player_inventory.find(bag.get_self_item()).unwrap();
            assert_eq!(4, real_target.get_contents().len());
            assert_eq!("Bag", real_target.get_name());
            let real_target_contents = real_target.get_contents();
            assert_eq!("Carton", real_target_contents.get(0).unwrap().get_self_item().get_name());
            assert_eq!("Bronze Bar", real_target_contents.get(1).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 1", real_target_contents.get(2).unwrap().get_self_item().get_name());
            assert_eq!("Steel Arming Sword", real_target_contents.get(3).unwrap().get_self_item().get_name());

            return; // pass
        }

        assert!(false)
    }

    #[test]
    #[allow(non_snake_case)]
    // B8. Within a PlayerInventory - Moving split item/container selection to a specific spot
    fn MoveItems_B8_PlayerInventory_MovingSplitSelectionToSpot() {
        // GIVEN a player focused test level (which has a player and their inventory)
        let mut level = build_player_test_level();

        let player = level.characters.get_player().unwrap();

        // AND the player inventory is in the expected state
        let player_inventory = player.get_inventory();
        validate_player_inventory(player_inventory);
        let player_inventory_contents= player_inventory.get_contents();
        assert_eq!(62, player_inventory_contents.len());

        // WHEN we call to move the first item (Bag) and the last (split selection)
        // to the middle of the container
        let bag = player_inventory_contents.get(0).unwrap();
        assert_eq!("Bag", bag.get_self_item().get_name());

        let steel_arming_sword = player_inventory_contents.get(61).unwrap();
        assert_eq!("Steel Arming Sword", steel_arming_sword.get_self_item().get_name());

        let middle_position_item = player_inventory_contents.get(30).unwrap().get_self_item();

        let request = MoveItemsRequestV2 {
            source: SourceContainerScope::PlayerInventory(
                PlayerInventoryContainer {
                    container: player_inventory.clone()
                }
            ),
            target: TargetContainerScope::PlayerInventoryItemPosition(
                PlayerInventoryItemPosition {
                    target_position_item: middle_position_item.clone()
                }
            ),
            to_move: vec![
                bag.get_self_item().clone(),
                steel_arming_sword.get_self_item().clone()
            ]
        };

        let result = move_items(request.clone(), &mut level);

        // THEN we expect a successful result to return
        if let Ok(response) = result {
            assert!(response.all_items_moved());

            // AND the response source should be the Player's inventory
            let updated_source = response.updated_scopes.source;
            let updated_source_contents = updated_source.get_container().get_contents();
            assert!(updated_source.is_player_scope());
            assert_eq!("Test Player's Inventory", updated_source.get_self_item().get_name());
            // AND it's size should be unchanged
            assert_eq!(62, updated_source_contents.len());

            // AND the top of the source should now be Test item 1, 2, onwards
            assert_eq!("Test Item 1", updated_source_contents.get(0).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 2", updated_source_contents.get(1).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 3", updated_source_contents.get(2).unwrap().get_self_item().get_name());

            // AND right in the middle should be:
            // Test Item 29, the Bag, the Sword, and then Test Item 30 and Test Item 31 onwards
            assert_eq!("Test Item 29", updated_source_contents.get(28).unwrap().get_self_item().get_name());
            assert_eq!("Bag", updated_source_contents.get(29).unwrap().get_self_item().get_name());
            assert_eq!("Steel Arming Sword", updated_source_contents.get(30).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 30", updated_source_contents.get(31).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 31", updated_source_contents.get(32).unwrap().get_self_item().get_name());

            // AND we should have Test Item 58, 59, and 60 at the end (at the last item, the sword, has moved above)
            assert_eq!("Test Item 58", updated_source_contents.get(59).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 59", updated_source_contents.get(60).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 60", updated_source_contents.get(61).unwrap().get_self_item().get_name());

            // AND the updated target should be the item we specified earlier
            let updated_target = response.updated_scopes.target;
            assert!(updated_target.is_targeting_item_position());
            assert_eq!("Test Item 30", updated_target.get_self_item().get_name());

            // AND the real player inventory in the level should match this
            let real_player_inventory = level.get_player().unwrap().get_inventory();
            let real_player_inventory_contents = real_player_inventory.get_contents();
            assert_eq!(62, real_player_inventory_contents.len());

            // AND the top of the source should now be Test item 1, 2, onwards
            assert_eq!("Test Item 1", real_player_inventory_contents.get(0).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 2", real_player_inventory_contents.get(1).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 3", real_player_inventory_contents.get(2).unwrap().get_self_item().get_name());

            // AND right in the middle should be:
            // Test Item 29, the Bag, the Sword, and then Test Item 30 and Test Item 31 onwards
            assert_eq!("Test Item 29", real_player_inventory_contents.get(28).unwrap().get_self_item().get_name());
            assert_eq!("Bag", real_player_inventory_contents.get(29).unwrap().get_self_item().get_name());
            assert_eq!("Steel Arming Sword", real_player_inventory_contents.get(30).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 30", real_player_inventory_contents.get(31).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 31", real_player_inventory_contents.get(32).unwrap().get_self_item().get_name());

            // AND we should have Test Item 58, 59, and 60 at the end (at the last item, the sword, has moved above)
            assert_eq!("Test Item 58", real_player_inventory_contents.get(59).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 59", real_player_inventory_contents.get(60).unwrap().get_self_item().get_name());
            assert_eq!("Test Item 60", real_player_inventory_contents.get(61).unwrap().get_self_item().get_name());

            return; // pass
        }

        // Fail if we don't hit our logic
        assert!(false)
    }

    #[test]
    #[allow(non_snake_case)]
    // C1. Moving items/container from the PlayerInventory to World Container
    fn MoveItems_C1_MoveItemsFromPlayerInventoryToWorldContainer() {
        // GIVEN a valid map
        // that holds a Chest containing 6 containers (Each with a unique name)
        // AND this also has a player inventory
        let mut chest = Container::new(Uuid::new_v4(), "Chest".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container1 = Container::new(Uuid::new_v4(), "Test Container 1".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container2 = Container::new(Uuid::new_v4(), "Test Container 2".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container3 = Container::new(Uuid::new_v4(), "Test Container 3".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container4 = Container::new(Uuid::new_v4(), "Test Container 4".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container5 = Container::new(Uuid::new_v4(), "Test Container 5".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container6 = Container::new(Uuid::new_v4(), "Test Container 6".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        chest.push(vec![container1.clone(), container2.clone(), container3.clone(),  container4.clone(),  container5.clone(), container6.clone()], );
        assert_eq!(6, chest.get_total_count());

        let container_pos =  Position { x: 1, y: 1};
        let mut level = build_test_level(container_pos, chest.clone());

        let player =level.get_player_mut();
        let original_inventory = player.unwrap().get_inventory().clone();
        validate_player_inventory(&original_inventory);
        assert_eq!(62, original_inventory.get_contents().len());
        assert_eq!("Bag", original_inventory.get_contents().get(0).unwrap().get_self_item().get_name());

        let test_item_1 = original_inventory.get_contents().get(1).unwrap().clone();
        let test_item_2 = original_inventory.get_contents().get(2).unwrap().clone();
        assert_eq!("Test Item 1", test_item_1.get_name());
        assert_eq!("Test Item 2", test_item_2.get_name());

        // WHEN we call to move Test Item 1 and 2 into the Chest (moving from the Player's Inventory into the nearby Chest)

        let request = MoveItemsRequestV2 {
            source: SourceContainerScope::PlayerInventory(
                PlayerInventoryContainer {
                    container: original_inventory.clone()
                }
            ),
            target: TargetContainerScope::WorldContainer(
                WorldContainer {
                    container: chest.clone(),
                    position: container_pos.clone()
                }
            ),
            to_move: vec![
                test_item_1.get_self_item().clone(),
                test_item_2.get_self_item().clone()
            ]
        };

        let result = move_items(request.clone(), &mut level);

        // THEN we expect a successful result to return
        if let Ok(response) = result {
            assert!(response.all_items_moved());

            // -- Validate response source
            let response_source = response.updated_scopes.source.get_container();
            let response_source_contents = response_source.get_contents();
            assert_eq!("Test Player's Inventory", response_source.get_name());
            assert_eq!(60, response_source_contents.len());

            // -- Validate response target
            let response_target = response.updated_scopes.target.get_container().unwrap();
            let response_target_contents = response_target.get_contents();
            assert_eq!("Chest", response_target.get_name());
            assert_eq!(8, response_target_contents.len());
            assert_eq!("Test Container 1", response_target_contents.get(0).unwrap().get_name());
            assert_eq!("Test Container 2", response_target_contents.get(1).unwrap().get_name());
            assert_eq!("Test Container 3", response_target_contents.get(2).unwrap().get_name());
            assert_eq!("Test Container 4", response_target_contents.get(3).unwrap().get_name());
            assert_eq!("Test Container 5", response_target_contents.get(4).unwrap().get_name());
            assert_eq!("Test Container 6", response_target_contents.get(5).unwrap().get_name());
            assert_eq!("Test Item 1", response_target_contents.get(6).unwrap().get_name());
            assert_eq!("Test Item 2", response_target_contents.get(7).unwrap().get_name());

            // -- Validate real source (Player's inventory)
            let real_player_inventory = level.get_player().unwrap().get_inventory();
            assert_eq!("Test Player's Inventory", real_player_inventory.get_name());
            assert_eq!(60, real_player_inventory.get_contents().len());

            // -- Validate real target (map Chest)
            let real_target = level.get_map_mut().unwrap().find_container(&response.updated_scopes.target.get_container().unwrap(), container_pos).unwrap();
            assert_eq!("Chest", real_target.get_name());
            let real_target_contents = real_target.get_contents();
            assert_eq!(8, real_target_contents.len());
            assert_eq!("Test Container 1", real_target_contents.get(0).unwrap().get_name());
            assert_eq!("Test Container 2", real_target_contents.get(1).unwrap().get_name());
            assert_eq!("Test Container 3", real_target_contents.get(2).unwrap().get_name());
            assert_eq!("Test Container 4", real_target_contents.get(3).unwrap().get_name());
            assert_eq!("Test Container 5", response_target_contents.get(4).unwrap().get_name());
            assert_eq!("Test Container 6", real_target_contents.get(5).unwrap().get_name());
            assert_eq!("Test Item 1", real_target_contents.get(6).unwrap().get_name());
            assert_eq!("Test Item 2", real_target_contents.get(7).unwrap().get_name());

            return; // pass

        }

        assert!(false)
    }

    #[test]
    #[allow(non_snake_case)]
    // C2. Moving items/container from a World Container to the Player Inventory (should error as this is just TakeItems)
    fn MoveItems_C2_MoveItemsFromWorldContainerToPlayerInventory() {
        // GIVEN a valid map
        // that holds a Chest containing 6 containers (Each with a unique name)
        // AND this also has a player inventory
        let mut chest = Container::new(Uuid::new_v4(), "Chest".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container1 = Container::new(Uuid::new_v4(), "Test Container 1".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container2 = Container::new(Uuid::new_v4(), "Test Container 2".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container3 = Container::new(Uuid::new_v4(), "Test Container 3".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container4 = Container::new(Uuid::new_v4(), "Test Container 4".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container5 = Container::new(Uuid::new_v4(), "Test Container 5".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container6 = Container::new(Uuid::new_v4(), "Test Container 6".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        chest.push(vec![container1.clone(), container2.clone(), container3.clone(),  container4.clone(),  container5.clone(), container6.clone()], );
        assert_eq!(6, chest.get_total_count());

        let container_pos =  Position { x: 1, y: 1};
        let mut level = build_test_level(container_pos, chest.clone());

        let player =level.get_player_mut();
        let original_inventory = player.unwrap().get_inventory().clone();
        validate_player_inventory(&original_inventory);
        assert_eq!(62, original_inventory.get_contents().len());
        assert_eq!("Bag", original_inventory.get_contents().get(0).unwrap().get_self_item().get_name());

        // WHEN we call to move Test Container 1 and 2 from the Chest into the Player's Inventory
        let request = MoveItemsRequestV2 {
            source: SourceContainerScope::WorldContainer(
                WorldContainer {
                    container: chest.clone(),
                    position: container_pos.clone()
                }
            ),
            target: TargetContainerScope::PlayerInventory(
                PlayerInventoryContainer {
                    container: original_inventory.clone()
                }
            ),
            to_move: vec![
                container1.get_self_item().clone(),
                container2.get_self_item().clone()
            ]
        };

        let result = move_items(request.clone(), &mut level);

        // THEN we expect a failure result to return (as this is unsupported as it's covered by Take Items and not Move Items)
        if let Err(e) = result {
            assert_eq!(ErrorType::INTERNAL, e.error_type);
            assert!(e.io_error.is_none());
            assert!(e.displayable_message.is_none());
            assert_eq!(
                "Unsupported operation. Moving from world container to Player Inventory is supported by TakeItems instead.",
                e.internal_message.unwrap()
            );
            
            return // pass;
        }

        // TODO Fail if we don't hit our logic
        assert!(false)
    }
}