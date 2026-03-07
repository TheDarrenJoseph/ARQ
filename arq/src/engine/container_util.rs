use std::fmt::format;
use crate::map::Map;
use crate::engine::event::container::*;
use log::{error, info};
use std::io;

use crate::character::Character;
use crate::engine::level::Level;
use crate::error::errors::ErrorWrapper;
use crate::map::objects::container::Container;
use crate::map::objects::items::Item;
use crate::map::position::Position;
use crate::map::tile::Colour::Red;
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
            match target.add(container_item.clone()) {
                Ok(()) => {
                    copied.push(container_item.clone());
                },
                Err(e) => {
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
                let map_container = level.get_map_mut().unwrap().find_container(&data.source, pos);
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
            if let Some(target_container) = level.get_map_mut().unwrap().find_container_mut(target_position) {
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
        SourceContainerScope::PlayerInventory(_) => {
            let player = level.get_player_mut().unwrap();
            source_container = player.get_inventory_mut();
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
        return if (pos_container.id_equals(&container)) {
            Ok(pos_container)
        } else {
          let child_container = pos_container.get_contents_mut().iter_mut().find(|c| c.id_equals(&container));
          if let Some(c) = child_container {
              Ok(c)
          } else {
              Err(ErrorWrapper::new_internal(String::from("Failed to find container on the map after searching position and children")))
          }
        };
    } else {
        Err(ErrorWrapper::new_internal(String::from("Failed to find any container on the map at position")))
    }
}


/*
    Moves items within root, from the source container
 */
// fn move_items_within(root : &mut Container, source : Item, request: MoveItemsWithinSourceRequest) -> Option<MoveItemsResponse> {
//     // Find a mutable reference to the target container if provided
//     let root_target_result =  request.target_container
//         .map_or_else(
//             || { None },
//             // Find the target container within the root container
//             |target| { root.find_mut(target.get_self_item()) }
//         );
//
//     if let Some(target) = root_target_result {
//         let to_move_items = request.to_move.clone();
//
//         // We can't move items in-place, so first we copy..
//         // Copy items from the source into the target
//
//         let to_copy = find_copying_items(request.source_container, to_move_items);
//         let copy_result = copy_container_items(to_copy, target);
//         if !copy_result.copied.is_empty() || !copy_result.uncopied.is_empty() {
//
//             // Find the source container within the root container
//             if let Some(ref mut source_container) = root.find_mut(&source) {
//                 source_container.remove_matching_items(copy_result.copied.clone());
//                 // If the target contains our source, we need to replace the source there too
//                 let mut updated_target = copy_result.updated_target;
//                 if let Some(ut) = &mut updated_target {
//                     if let Some(found) = ut.find_mut(source_container.get_self_item()) {
//                        found.remove_matching_items(copy_result.copied.clone());
//                     }
//                 }
//
//                 // Now both target and source are updated, return a response to indicate what's moved
//                 let moved = copy_result.copied;
//                 let unmoved = copy_result.uncopied;
//                 log::info!("Returning MoveItems response with {} moved, {} unmoved items", moved.len(), unmoved.len());
//
//                 // Return a response to communicate the changes made
//                 // As the real upstream source will not have been changed here due to only a single mutable ref
//                 let data = MoveItemsResponse {
//                     source: source_container.clone(),
//                     unmoved,
//                     target_container: updated_target,
//                     position: request.source_position,
//                     message: format!("Moved {} of {} items", moved.len(), request.to_move.len())
//                 };
//                 return Some(data);
//             } else {
//                 log::error!("Failed to move items. Failed to find source container.");
//             }
//         } else {
//             log::error!("Failed to move items. {} moved, {} unmoved items", copy_result.copied.len(), copy_result.uncopied.len());
//         }
//     } else {
//         log::error!("Failed to move items. Couldn't find target container in source container.");
//         return None;
//     }
//     None
// }

// Moving items to a specific position within the source_container
fn move_items_to_source_position(mut request: MoveItemsRequestV2) -> Option<MoveItemsResponseV2> {
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
        if let Some(pos) = source_container.item_position(&piip.target_position_item) {
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
            let target_pos = if pos >= moving.len() { pos - moving.len() } else { pos };
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
        if let Some(pos) = source_container.item_position(&wcip.target_position_item) {
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
            let target_pos = if pos >= moving.len() { pos - moving.len() } else { pos };
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

// Moves items between player inventory containers / into world container
// pub fn move_player_items(data: MoveItemsRequest, level : &mut Level) -> Result<MoveItemsResponse, ErrorWrapper> {
//     return if let Some(_) = data.source_position {
//         Err(ErrorWrapper::new_internal(String::from("[container_util::move_player_items] Cannot move player items to a specific position / world container combo (Not implemented).")))
//     } else {
//         log::info!("[move_player_items] Attempting to move player items to a target container (inside inventory)...");
//         let player: &mut Character = level.characters.get_player_mut().unwrap();
//         let inventory: &mut Container = player.get_inventory_mut();
//         let source_container;
//         let source_item;
//         let target_inventory = data.target_container.as_ref().map_or_else(|| false, |c| inventory.id_equals(&c));
//         let target_in_source = data.target_container.as_ref().map_or_else(|| false, |c| data.source_container.find(c.get_self_item()).is_some());
//         if inventory.id_equals(&data.source_container) || target_inventory || !target_in_source {
//             source_container = Some(inventory);
//             source_item = Some(data.source_container.get_self_item().clone());
//         } else {
//             source_container = inventory.find_mut(data.source_container.get_self_item());
//             source_item = source_container.as_ref().map(|s| s.get_self_item().clone());
//         }
//
//         if let Some(source) = source_container {
//             if let Some(_) = data.target_container {
//                 if let Some(source_item) = source_item {
//                     log::info!("Attempting move to container..");
//                     return move_items_within(source, source_item, data).ok_or(
//                         ErrorWrapper::new_internal(String::from("[container_util::move_player_items] Failed to move items to container"))
//                     )
//                 } else {
//                     return Err(ErrorWrapper::new_internal(String::from("[container_util::move_player_items] Failed to move items to container. Failed to find source item")));
//                 }
//             } else if let Some(_) = data.target_position_item {
//                 log::info!("Attempting move to item spot..");
//                 return move_to_item_spot(source, data).ok_or(
//                     ErrorWrapper::new_internal(String::from("[container_util::move_player_items] Failed to move items to item spot in container"))
//                 )
//             } else {
//                 return Err(ErrorWrapper::new_internal(String::from("[container_util::move_items] Cannot move items. No target item provided")));
//             }
//         } else {
//             Err(ErrorWrapper::new_internal(String::from("[container_util::move_items] Cannot move items. Failed to find source container")))
//         }
//     }
// }


pub fn move_items(request: MoveItemsRequestV2, level: &mut Level) -> Result<MoveItemsResponseV2, ErrorWrapper> {
    // Copy our request data so we're not moving it around too much
    let request_result_copy = request.clone();

    if let Some(map) = &mut level.map {
        // Clone out the request fields so we can use them safely without moving the request
        let source = request.source.clone();
        let target = request.target.clone();
        let to_mvoe = request.to_move.clone();


        if (target.is_container_target()) {
            if (source.get_container().id_equals(&target.get_container().unwrap())) {
                return Err(ErrorWrapper::new_internal(String::from("Cannot move items. Source cannot be the target")));
            }
        }

        let source_container = source.get_container().clone();

        if (target.is_container_target()) {
            let live_target: &mut Container;
            match target {
                TargetContainerScope::WorldContainer(target) => {
                    live_target = level
                        .get_map_mut().unwrap()
                        .find_container(&target.container, target.position)
                        .expect("Failed to find target container on the map");
                },
                TargetContainerScope::PlayerInventory(target) => {
                    live_target =
                        level.get_player_mut().unwrap().get_inventory_mut()
                },
                _ => {
                    return Err(ErrorWrapper::new_internal(String::from("Unsupported operation")));
                }
            }

            let to_copy = find_copying_items(source_container, request.to_move.clone());
            let copy_result = copy_container_items(to_copy, live_target);
            let updated_target = live_target.clone();

            let moved = copy_result.copied.clone();
            let unmoved = copy_result.uncopied.clone();
            let updated_source = update_source_container(level, request.clone(), copy_result);

            let updated_source_scope;
            if (request.source.is_world_scope()) {
                if let SourceContainerScope::WorldContainer(wc) = request.source {
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
                if let SourceContainerScope::PlayerInventory(pic) = request.source {
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
            if (request.target.is_world_scope()) {
                if let TargetContainerScope::WorldContainer(wc) = request.target {
                    updated_target_scope = TargetContainerScope::WorldContainer(
                        WorldContainer {
                            container: updated_target,
                            position: wc.position.clone()
                        }
                    )
                } else {
                    return Err(ErrorWrapper::new_internal(String::from("Couldn't convert target")));
                }
            } else {
                if let TargetContainerScope::PlayerInventory(pic) = request.target {
                    updated_target_scope = TargetContainerScope::PlayerInventory(
                        PlayerInventoryContainer {
                            container: updated_target.clone()
                        }
                    )
                } else {
                    return Err(ErrorWrapper::new_internal(String::from("Couldn't convert target")));
                }
            }

            log::info!("Returning MoveItemBetweenResponse with {} moved, {} unmoved items", moved.len(), unmoved.len());
            return Ok(MoveItemsResponseV2 {
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
            let move_response = move_items_to_source_position(request);
            if (move_response.is_none()) {
                return Err(ErrorWrapper::new_internal(String::from("Failed to move items to source position")));
            } else {
                return Ok(move_response.unwrap());
            }
        }
    } else {
        return Err(ErrorWrapper::new_internal(String::from("Cannot move items. No map available.")));
    }
}

pub fn build_container_choices(source: &Container, parent_scope: TargetContainerScope, parent_position: Position) -> Result<Vec<ContainerChoice>, io::Error> {
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

#[cfg(test)]
mod tests {
    use crate::engine::event::container::{SourceContainerScope, MoveItemsWithinSourceRequest, MoveItemsRequestV2, WorldContainer, TargetContainerScope};
use std::collections::HashMap;

    use uuid::Uuid;

    use crate::character::builder::character_builder::{CharacterBuilder, CharacterPattern};
    use crate::character::characters::Characters;
    use crate::engine::container_util::{move_items};
    use crate::engine::level::Level;
    use crate::map::objects::container::{Container, ContainerType};
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

    #[test]
    fn test_move_items_within_container() {
        // GIVEN a source container containing 3 containers
        let mut source_container = Container::new(Uuid::new_v4(), "Source Container".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container1 = Container::new(Uuid::new_v4(), "Test Container 1".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container2 = Container::new(Uuid::new_v4(), "Test Container 2".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let container3 = Container::new(Uuid::new_v4(), "Test Container 3".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        source_container.push(vec![container1.clone(), container2.clone(), container3.clone()]);
        assert_eq!(3, source_container.get_total_count());

        // And we want to move Container 1 into Container 3
        let to_move = vec![container1.get_self_item().clone()];
        let source = source_container.clone();
        let container_pos =  Position { x: 1, y: 1};
        let target_item = container3.get_self_item().clone();

        // AND all of this is present in a level
        let mut level = build_test_level(container_pos.clone(), source_container.clone());

        // WHEN we call to move container 1 into container 3
        let request = MoveItemsRequestV2 {
            source: SourceContainerScope::WorldContainer(
                WorldContainer {
                    container: source_container.clone(),
                    position: container_pos
                }
            ),
            target: TargetContainerScope::WorldContainer(
                WorldContainer {
                    container: container3.clone(),
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

            // And the 'target' (Container 3) container will have the item now
            let target_container = &request.target.get_container().unwrap();
            let updated_target = level.get_map_mut().unwrap().find_container(target_container, container_pos);
            if let Some(c) = updated_target {
                // There should be 1 item in container
                assert_eq!(1, c.get_total_count());
                // And it should be our Container 1
                let top_item = &c.get_contents()[0];
                assert_eq!(top_item.get_self_item_id(), container1.get_self_item_id());
                return; // pass
            }

            // AND The map 'source' container will have the item removed
            let updated_source = level.get_map_mut().unwrap().find_container(&request.source.get_container(), container_pos);
            if let Some(c) = updated_source {
                // There should be 2 items in to root container's top level
                assert_eq!(2, c.get_top_level_count());
                // AND The 'target' container will contain the new items
                if let Some(container_item) = c.find(&target_item) {
                    assert_eq!(1, container_item.get_total_count());
                }
                return; // pass
            }
        }
        assert!(false);
    }


    #[test]
    fn test_move_items_within_from_lower_to_parent() {
        // GIVEN a valid map

        // AND a chest that contains a nested bag and carton
        let mut chest = Container::new(Uuid::new_v4(), "Chest".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
        let mut bag = Container::new(Uuid::new_v4(), "Bag".to_owned(), 'X', 5.0, 1, ContainerType::OBJECT, 100);
        let mut carton = Container::new(Uuid::new_v4(), "Carton".to_owned(), 'X', 5.0, 1, ContainerType::OBJECT, 100);
        let carton_id = carton.get_self_item().get_id();

        // AND each of them contain some other items
        let item1 = Container::new(Uuid::new_v4(), "Test Item 1".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
        let item2 = Container::new(Uuid::new_v4(), "Test Item 2".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
        let item3 = Container::new(Uuid::new_v4(), "Test Item 3".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
        chest.push(vec![item1.clone(), item2.clone(), item3.clone()]);

        let item4 = Container::new(Uuid::new_v4(), "Test Item 4".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
        let item5 = Container::new(Uuid::new_v4(), "Test Item 5".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
        let item6 = Container::new(Uuid::new_v4(), "Test Item 6".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
        bag.push(vec![item4.clone(), item5.clone(), item6.clone()]);

        let item7 = Container::new(Uuid::new_v4(), "Test Item 7".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
        let item8 = Container::new(Uuid::new_v4(), "Test Item 8".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
        let item9 = Container::new(Uuid::new_v4(), "Test Item 9".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
        carton.push(vec![item7.clone(), item8.clone(), item9.clone()]);

        bag.push(vec![carton.clone()]);
        assert_eq!(4, bag.get_top_level_count());

        chest.push(vec![bag.clone()]);
        assert_eq!(4, chest.get_top_level_count());


        // Our container Hierarchy should now be
        //  Chest -> Bag -> Carton

        // AND all of this is stored in a test level
        let container_pos = Position { x: 1, y: 1 };
        let mut level = build_test_level(container_pos, chest);

        // WHEN we call to move items from the lowest container (Carton's item 7 and 8)
        // into the parent container (Bag)
        let to_move = vec![item7.get_self_item().clone(), item8.get_self_item().clone()];
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

            let source = request.source.clone();
            let target = request.target.clone();

            // AND the map will be updated to reflect this
            // Find the Carton
            let source_updated = level.get_map_mut().unwrap().find_container(source.get_container(), container_pos);
            assert_eq!(1, source_updated.unwrap().get_top_level_count());

            // Find the bag
            let target_container = &request.target.get_container().unwrap();
            let mut target_updated = level.get_map_mut().unwrap().find_container(target_container, container_pos);
            assert_eq!(6, target_updated.as_ref().unwrap().get_top_level_count());

            let target_contents = target_updated.as_mut().unwrap().get_contents();
            assert_eq!(item4.get_self_item_id(), target_contents.get(0).unwrap().get_self_item_id());
            assert_eq!(item5.get_self_item_id(), target_contents.get(1).unwrap().get_self_item_id());
            assert_eq!(item6.get_self_item_id(), target_contents.get(2).unwrap().get_self_item_id());
            assert_eq!(carton.get_self_item_id(), target_contents.get(3).unwrap().get_self_item_id());
            assert_eq!(item7.get_self_item_id(), target_contents.get(4).unwrap().get_self_item_id());
            assert_eq!(item8.get_self_item_id(), target_contents.get(5).unwrap().get_self_item_id());
            return;
        }
        assert!(false);
    }
    //
    // #[test]
    // fn test_move_items_bottom() {
    //     // GIVEN a valid map
    //     // that holds a source container containing 6 containers (Each with a unique name)
    //     let mut source_container = Container::new(Uuid::new_v4(), "Source Container".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let container1 = Container::new(Uuid::new_v4(), "Test Container 1".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let container2 = Container::new(Uuid::new_v4(), "Test Container 2".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let container3 = Container::new(Uuid::new_v4(), "Test Container 3".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let container4 = Container::new(Uuid::new_v4(), "Test Container 4".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let container5 = Container::new(Uuid::new_v4(), "Test Container 5".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let container6 = Container::new(Uuid::new_v4(), "Test Container 6".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //
    //     // Clone everything before moving
    //     let to_move = vec![container1.get_self_item().clone(), container2.get_self_item().clone()];
    //     source_container.push(vec![container1, container2, container3,  container4,  container5, container6], );
    //     let source_copy = source_container.clone();
    //     assert_eq!(6, source_container.get_total_count());
    //
    //     let source = source_container.clone();
    //     let container_pos =  Position { x: 1, y: 1};
    //     let target_item = source_container.get(5).get_self_item().clone();
    //     let _expected_target = target_item.clone();
    //     let mut level = build_test_level(container_pos, source_container);
    //
    //     // WHEN we call to move container 1 and 2 to the bottom of the list (Container 6's location)
    //     let data = MoveItemsRequest {
    //         source_container: source,
    //         to_move,
    //         target_container: None,
    //         target_position_item: Some(target_item),
    //         source_position: Some(container_pos),
    //         target_position: None
    //     };
    //     let data_expected = data.clone();
    //     let result = move_items(data, &mut level);
    //
    //     // THEN we expect a result to return
    //     assert!(result.is_ok());
    //
    //     // THEN we expect a valid result
    //     if let Ok(response) = result {
    //         // AND the source/targets should be returned with no outstanding to_move data
    //         assert!(data_expected.source_container.id_equals(&response.source));
    //         assert_eq!(0, response.unmoved.len());
    //
    //         // AND The map 'source' container will have it's items reshuffled
    //         let map_container = level.get_map_mut().unwrap().find_container(&data_expected.source_container, container_pos);
    //         if let Some(c) = map_container {
    //             assert_eq!(6, c.get_total_count());
    //             let contents = c.get_contents();
    //             assert_eq!(source_copy.get(2).get_self_item().get_name(), contents[0].get_self_item().get_name());
    //             assert_eq!(source_copy.get(3).get_self_item().get_name(), contents[1].get_self_item().get_name());
    //             assert_eq!(source_copy.get(4).get_self_item().get_name(), contents[2].get_self_item().get_name());
    //             assert_eq!(source_copy.get(0).get_self_item().get_name(), contents[3].get_self_item().get_name());
    //             assert_eq!(source_copy.get(1).get_self_item().get_name(), contents[4].get_self_item().get_name());
    //             assert_eq!(source_copy.get(5).get_self_item().get_name(), contents[5].get_self_item().get_name());
    //             return; // pass
    //         }
    //     }
    //     assert!(false);
    //
    // }
    //
    // #[test]
    // fn test_move_items_top() {
    //     // GIVEN a valid map
    //     // that holds a source container containing 6 containers (Each with a unique name)
    //     let mut source_container = Container::new(Uuid::new_v4(), "Source Container".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let container1 = Container::new(Uuid::new_v4(), "Test Container 1".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let container2 = Container::new(Uuid::new_v4(), "Test Container 2".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let container3 = Container::new(Uuid::new_v4(), "Test Container 3".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let container4 = Container::new(Uuid::new_v4(), "Test Container 4".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let container5 = Container::new(Uuid::new_v4(), "Test Container 5".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let container6 = Container::new(Uuid::new_v4(), "Test Container 6".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //
    //     // Clone everything before moving
    //     let to_move = vec![container5.get_self_item().clone(), container6.get_self_item().clone()];
    //     source_container.push(vec![container1, container2, container3,  container4,  container5, container6], );
    //     let source_copy = source_container.clone();
    //     assert_eq!(6, source_container.get_total_count());
    //
    //     let source = source_container.clone();
    //     let container_pos =  Position { x: 1, y: 1};
    //     let target_item = source_container.get(0).get_self_item().clone();
    //     let _expected_target = target_item.clone();
    //     let mut level = build_test_level(container_pos, source_container);
    //
    //     // WHEN we call to move container 5 and 6 to the top of the list (Container 1's location)
    //     let data = MoveItemsRequest {
    //         source_container: source,
    //         to_move,
    //         target_container: None,
    //         target_position_item: Some(target_item),
    //         source_position: Some(container_pos),
    //         target_position: None
    //     };
    //     let data_expected = data.clone();
    //     let result = move_items(data, &mut level);
    //
    //     // THEN we expect a result to return
    //     assert!(result.is_ok());
    //
    //     // THEN we expect a valid result
    //     if let Ok(response) = result {
    //         // AND the source/targets should be returned with no outstanding to_move data
    //         assert!(data_expected.source_container.id_equals(&response.source));
    //         assert_eq!(0, response.unmoved.len());
    //
    //         // AND The map 'source' container will have it's items reshuffled
    //         let map_container = level.get_map_mut().unwrap().find_container(&data_expected.source_container, container_pos);
    //         if let Some(c) = map_container {
    //             assert_eq!(6, c.get_total_count());
    //             let contents = c.get_contents();
    //             assert_eq!(source_copy.get(4).get_self_item().get_name(), contents[0].get_self_item().get_name());
    //             assert_eq!(source_copy.get(5).get_self_item().get_name(), contents[1].get_self_item().get_name());
    //             assert_eq!(source_copy.get(0).get_self_item().get_name(), contents[2].get_self_item().get_name());
    //             assert_eq!(source_copy.get(1).get_self_item().get_name(), contents[3].get_self_item().get_name());
    //             assert_eq!(source_copy.get(2).get_self_item().get_name(), contents[4].get_self_item().get_name());
    //             assert_eq!(source_copy.get(3).get_self_item().get_name(), contents[5].get_self_item().get_name());
    //             return; // pass
    //         }
    //     }
    //     assert!(false);
    // }
    //
    // #[test]
    // fn test_move_item_middle() {
    //     // GIVEN a valid map
    //     // that holds a source container containing 6 containers (Each with a unique name)
    //     let mut source_container = Container::new(Uuid::new_v4(), "Source Container".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let container1 = Container::new(Uuid::new_v4(), "Test Container 1".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let container2 = Container::new(Uuid::new_v4(), "Test Container 2".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let container3 = Container::new(Uuid::new_v4(), "Test Container 3".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let container4 = Container::new(Uuid::new_v4(), "Test Container 4".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let container5 = Container::new(Uuid::new_v4(), "Test Container 5".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let container6 = Container::new(Uuid::new_v4(), "Test Container 6".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //
    //     // Clone everything before moving
    //     let to_move = vec![container1.get_self_item().clone(), container2.get_self_item().clone()];
    //     source_container.push(vec![container1, container2, container3,  container4,  container5, container6], );
    //     let source_copy = source_container.clone();
    //     assert_eq!(6, source_container.get_total_count());
    //
    //     // WHEN we call to move container 1 and 2 to the middle of the list (Container 5's location)
    //     let source = source_container.clone();
    //     let container_pos =  Position { x: 1, y: 1};
    //     let target_item = source_container.get(4).get_self_item().clone();
    //     let _expected_target = target_item.clone();
    //     let mut level = build_test_level(container_pos, source_container);
    //     let data = MoveItemsRequest {
    //         source_container: source,
    //         to_move,
    //         target_container: None,
    //         target_position_item: Some(target_item),
    //         source_position: Some(container_pos),
    //         target_position: None
    //     };
    //     let data_expected = data.clone();
    //     let result = move_items(data, &mut level);
    //
    //     // THEN we expect a result to return
    //     assert!(result.is_ok());
    //
    //     // THEN we expect a valid result
    //     if let Ok(response) = result {
    //         // AND the source/targets should be returned with no outstanding to_move data
    //         assert!(data_expected.source_container.id_equals(&response.source));
    //         assert_eq!(0, response.unmoved.len());
    //
    //         // AND The map 'source' container will have it's items reshuffled
    //         let map_container = level.get_map_mut().unwrap().find_container(&data_expected.source_container, container_pos);
    //         if let Some(c) = map_container {
    //             assert_eq!(6, c.get_total_count());
    //             let contents = c.get_contents();
    //             assert_eq!(source_copy.get(2).get_self_item().get_name(), contents[0].get_self_item().get_name());
    //             assert_eq!(source_copy.get(3).get_self_item().get_name(), contents[1].get_self_item().get_name());
    //             assert_eq!(source_copy.get(0).get_self_item().get_name(), contents[2].get_self_item().get_name());
    //             assert_eq!(source_copy.get(1).get_self_item().get_name(), contents[3].get_self_item().get_name());
    //             assert_eq!(source_copy.get(4).get_self_item().get_name(), contents[4].get_self_item().get_name());
    //             assert_eq!(source_copy.get(5).get_self_item().get_name(), contents[5].get_self_item().get_name());
    //             return; // pass
    //         }
    //     }
    //     assert!(false);
    // }
    //
    // #[test]
    // fn test_move_split_items() {
    //     // GIVEN a valid map
    //     // that holds a source container containing 6 containers (Each with a unique name)
    //     let mut source_container = Container::new(Uuid::new_v4(), "Source Container".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let container1 = Container::new(Uuid::new_v4(), "Test Container 1".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let container2 = Container::new(Uuid::new_v4(), "Test Container 2".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let container3 = Container::new(Uuid::new_v4(), "Test Container 3".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let container4 = Container::new(Uuid::new_v4(), "Test Container 4".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let container5 = Container::new(Uuid::new_v4(), "Test Container 5".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let container6 = Container::new(Uuid::new_v4(), "Test Container 6".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //
    //     // Clone everything before moving
    //     let to_move = vec![container1.get_self_item().clone(), container6.get_self_item().clone()];
    //     source_container.push(vec![container1.clone(), container2.clone(), container3.clone(), container4.clone(), container5.clone(), container6.clone()], );
    //     //let source_container_copy = source_container.clone();
    //     assert_eq!(6, source_container.get_total_count());
    //
    //     // WHEN we call to move "Test Container 1" and "Test Container 6" to container 2's location (index 1)
    //     let source = source_container.clone();
    //     let container_pos =  Position { x: 1, y: 1};
    //     // Target is "Test Container 2"
    //     let target_item = source_container.get(1).get_self_item().clone();
    //     let _expected_target = target_item.clone();
    //     let mut level = build_test_level(container_pos, source_container.clone());
    //     let data = MoveItemsRequest {
    //         source_container: source,
    //         to_move,
    //         target_container: None,
    //         target_position_item: Some(target_item),
    //         source_position: Some(container_pos),
    //         target_position: None
    //     };
    //     let data_expected = data.clone();
    //     let result = move_items(data, &mut level);
    //
    //     // THEN we expect a result to return
    //     assert!(result.is_ok());
    //
    //     if let Ok(response) = result {
    //         // AND the source/targets should be returned with no outstanding to_move data
    //         assert!(data_expected.source_container.id_equals(&response.source));
    //         assert_eq!(0, response.unmoved.len());
    //
    //         // AND The map 'source' container will have it's items reshuffled
    //         let map_container = level.get_map_mut().unwrap().find_container(&data_expected.source_container, container_pos);
    //         if let Some(c) = map_container {
    //             assert_eq!(6, c.get_total_count());
    //             let contents = c.get_contents();
    //             // AND the order should now match our expectations
    //             // Test Container 2
    //             assert_eq!(container2.get_self_item().get_name(), contents[0].get_self_item().get_name());
    //             // Test Container 1
    //             assert_eq!(container1.get_self_item().get_name(), contents[1].get_self_item().get_name());
    //             // Test Container 6
    //             assert_eq!(container6.get_self_item().get_name(), contents[2].get_self_item().get_name());
    //             // Test Container 3
    //             assert_eq!(container3.get_self_item().get_name(), contents[3].get_self_item().get_name());
    //             // Test Container 4
    //             assert_eq!(container4.get_self_item().get_name(), contents[4].get_self_item().get_name());
    //             // Test Container 5
    //             assert_eq!(container5.get_self_item().get_name(), contents[5].get_self_item().get_name());
    //             return; // pass
    //         }
    //     }
    //     assert!(false)
    // }
    //
    // #[test]
    // fn test_move_items_no_position() {
    //     // GIVEN a valid map
    //     // that holds a source container containing 3 containers
    //     let mut source_container = Container::new(Uuid::new_v4(), "Source Container".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let container1 = Container::new(Uuid::new_v4(), "Test Container 1".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let container2 = Container::new(Uuid::new_v4(), "Test Container 2".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let container3 = Container::new(Uuid::new_v4(), "Test Container 3".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let to_move = vec![container1.get_self_item().clone()];
    //     source_container.push(vec![container1, container2, container3]);
    //     assert_eq!(3, source_container.get_total_count());
    //
    //     let source = source_container.clone();
    //     let container_pos =  Position { x: 1, y: 1};
    //     let target = source_container.get(2).clone();
    //     let _target_item = target.get_self_item().clone();
    //     let mut level = build_test_level(container_pos, source_container);
    //
    //     // WHEN we call to move container 1 into container 3 without a position for the container
    //     let data = MoveItemsRequest {
    //         source_container: source,
    //         to_move,
    //         target_container: Some(target),
    //         target_position_item: None,
    //         source_position: None,
    //         target_position: None
    //     };
    //     let _data_expected = data.clone();
    //     let result = move_items(data, &mut level);
    //     // THEN we expect an Error to return
    //     assert!(result.is_err());
    //     result.expect_err("[container_util::move_items] Cannot move items. No map position provided");
    // }
    //
    // #[test]
    // fn test_move_player_items_from_parent_to_lower() {
    //     // GIVEN a player inventory containing a nested container (Bag)
    //     // AND the Bag contains a Carton
    //     let mut inventory = Container::new(Uuid::new_v4(), "Player Inventory".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let mut bag = Container::new(Uuid::new_v4(), "Bag".to_owned(), 'X', 5.0, 1, ContainerType::OBJECT, 100);
    //     let mut carton = Container::new(Uuid::new_v4(), "Carton".to_owned(), 'X', 5.0, 1, ContainerType::OBJECT, 100);
    //
    //     // AND all of them contain some other items
    //     let item1 = Container::new(Uuid::new_v4(), "Test Item 1".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
    //     let item2 = Container::new(Uuid::new_v4(), "Test Item 2".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
    //     let item3 = Container::new(Uuid::new_v4(), "Test Item 3".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
    //
    //     let item4 = Container::new(Uuid::new_v4(), "Test Item 4".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
    //     let item5 = Container::new(Uuid::new_v4(), "Test Item 5".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
    //     let item6 = Container::new(Uuid::new_v4(), "Test Item 6".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
    //
    //     let item7 = Container::new(Uuid::new_v4(), "Test Item 7".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
    //     let item8 = Container::new(Uuid::new_v4(), "Test Item 8".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
    //     let item9 = Container::new(Uuid::new_v4(), "Test Item 9".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
    //
    //     // AND we're moving items 2 and 3 from the parent into the Bag (first child)
    //     let to_move = vec![item2.get_self_item().clone(), item3.get_self_item().clone()];
    //     carton.push(vec![item7, item8, item9]);
    //     bag.push(vec![item4, item5, item6, carton.clone()]);
    //
    //     inventory.push(vec![item1, item2, item3, bag.clone()]);
    //     let source = inventory.clone();
    //
    //     let target = bag.clone();
    //
    //     // 11 total contents (including the Bag contents)
    //     assert_eq!(11, inventory.get_total_count());
    //     // Root container has items 1-3 and the bag at the top level
    //     assert_eq!(4, inventory.get_top_level_count());
    //     assert_eq!(4, bag.get_top_level_count());
    //     assert_eq!(3, carton.get_top_level_count());
    //
    //     // AND the level has been setup with the player inventory
    //     let mut level = build_player_test_level();
    //     level.characters.get_player_mut().unwrap().set_inventory(inventory);
    //
    //     // WHEN we try to move these
    //     let data = MoveItemsRequest {
    //         source_container: source, to_move,
    //         target_container: Some(target),
    //         target_position_item: None,
    //         source_position: None,
    //         target_position: None
    //     };
    //     let result = move_player_items(data, &mut level);
    //
    //     // THEN we expect a result to return
    //     assert!(result.is_ok());
    //
    //     if let Ok(response) = result {
    //         // with 0 unmoved items
    //         assert_eq!(0, response.unmoved.len());
    //
    //         let updated_inventory = level.characters.get_player_mut().unwrap().get_inventory_mut();
    //         // AND the player's inventory should now have 2 items in it's top level count
    //         assert_eq!(2, updated_inventory.get_top_level_count());
    //
    //         // AND The Bag should have 5 items now
    //         let bag_item = bag.get_self_item().clone();
    //         if let Some(c) = updated_inventory.find(&bag_item) {
    //             assert_eq!(6, c.get_top_level_count());
    //         } else {
    //             assert!(false, "Couldn't find Bag in the updated inventory!");
    //         }
    //
    //         // AND The Carton should have 3 items still
    //         let carton_item = carton.get_self_item().clone();
    //         if let Some(b) = updated_inventory.find(&carton_item) {
    //             assert_eq!(3, b.get_top_level_count());
    //         } else {
    //             assert!(false, "Couldn't find Carton in the updated inventory!");
    //         }
    //
    //     } else {
    //         assert!(false, "Unexpected data type returned");
    //     }
    // }
    //
    // #[test]
    // fn test_move_player_items_from_lower_to_parent() {
    //     // GIVEN a player inventory containing a nested container (Bag)
    //     // AND the Bag contains a Carton
    //     let mut inventory = Container::new(Uuid::new_v4(), "Player Inventory".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let mut bag = Container::new(Uuid::new_v4(), "Bag".to_owned(), 'X', 5.0, 1, ContainerType::OBJECT, 100);
    //     let mut carton = Container::new(Uuid::new_v4(), "Carton".to_owned(), 'X', 5.0, 1, ContainerType::OBJECT, 100);
    //
    //     // AND all of them contain some other items
    //     let item1 = Container::new(Uuid::new_v4(), "Test Item 1".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
    //     let item2 = Container::new(Uuid::new_v4(), "Test Item 2".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
    //     let item3 = Container::new(Uuid::new_v4(), "Test Item 3".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
    //
    //     let item4 = Container::new(Uuid::new_v4(), "Test Item 4".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
    //     let item5 = Container::new(Uuid::new_v4(), "Test Item 5".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
    //     let item6 = Container::new(Uuid::new_v4(), "Test Item 6".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
    //
    //     let item7 = Container::new(Uuid::new_v4(), "Test Item 7".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
    //     let item8 = Container::new(Uuid::new_v4(), "Test Item 8".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
    //     let item9 = Container::new(Uuid::new_v4(), "Test Item 9".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
    //
    //     // AND we're moving items from the underlying Carton into the parent (Bag)
    //     let to_move = vec![item8.get_self_item().clone()];
    //     let carton_item = carton.get_self_item().clone();
    //     let bag_item = bag.get_self_item().clone();
    //
    //     carton.push(vec![item7, item8, item9]);
    //     bag.push(vec![item4, item5, item6, carton.clone()]);
    //
    //     let source = carton.clone();
    //     inventory.push(vec![item1, item2, item3, bag.clone()]);
    //
    //     let target = bag.clone();
    //
    //     // 11 total contents (including the Bag contents)
    //     assert_eq!(11, inventory.get_total_count());
    //     // Root container has items 1-3 and the bag at the top level
    //     assert_eq!(4, inventory.get_top_level_count());
    //
    //     // AND the level has been setup with the player inventory
    //     let mut level = build_player_test_level();
    //     level.characters.get_player_mut().unwrap().set_inventory(inventory);
    //
    //     // WHEN we try to move an item from the bag into the root container
    //     let data = MoveItemsRequest {
    //         source_container: source,
    //         to_move,
    //         target_container: Some(target),
    //         target_position_item: None,
    //         source_position: None,
    //         target_position: None
    //     };
    //     let result = move_player_items(data, &mut level);
    //
    //     // THEN we expect a result to return
    //     assert!(result.is_ok());
    //
    //     if let Ok(response) = result {
    //         // with 0 unmoved items
    //         assert_eq!(0, response.unmoved.len());
    //
    //         let updated_inventory = level.characters.get_player_mut().unwrap().get_inventory_mut();
    //         // AND the player's inventory should not have 4 items in it's content count
    //         assert_eq!(4, updated_inventory.get_top_level_count());
    //
    //         // AND The Bag should have 5 items now
    //         if let Some(c) = updated_inventory.find(&bag_item) {
    //             assert_eq!(5, c.get_top_level_count());
    //         } else {
    //             assert!(false, "Couldn't find Bag in the updated inventory!");
    //         }
    //
    //         // AND The Carton should have only 2 items now
    //         if let Some(b) = updated_inventory.find(&carton_item) {
    //             assert_eq!(2, b.get_top_level_count());
    //         } else {
    //             assert!(false, "Couldn't find Carton in the updated inventory!");
    //         }
    //     } else {
    //         assert!(false, "Unexpected data type returned");
    //     }
    //
    // }
    //
    // #[test]
    // fn test_move_player_items_from_lower_to_root() {
    //     // GIVEN a player inventory containing a nested container (Bag)
    //     let mut inventory = Container::new(Uuid::new_v4(), "Player Inventory".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);
    //     let mut bag = Container::new(Uuid::new_v4(), "Bag".to_owned(), 'X', 5.0, 1, ContainerType::OBJECT, 100);
    //
    //     // AND both the parent container and bag contains some other items
    //     let item1 = Container::new(Uuid::new_v4(), "Test Item 1".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
    //     let item2 = Container::new(Uuid::new_v4(), "Test Item 2".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
    //     let item3 = Container::new(Uuid::new_v4(), "Test Item 3".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
    //
    //     let item4 = Container::new(Uuid::new_v4(), "Test Item 4".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
    //     let item5 = Container::new(Uuid::new_v4(), "Test Item 5".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
    //     let item6 = Container::new(Uuid::new_v4(), "Test Item 6".to_owned(), 'X', 1.0, 1, ContainerType::ITEM, 0);
    //     let to_move = vec![item6.get_self_item().clone()];
    //
    //     // AND we're moving items from the underlying bag into the main inventory (root)
    //     let bag_item = bag.get_self_item().clone();
    //
    //     bag.push(vec![item4, item5, item6]);
    //     let source = bag.clone();
    //     inventory.push(vec![item1, item2, item3, bag]);
    //
    //     let target = inventory.clone();
    //
    //     // 7 total contents (including the Bag contents)
    //     assert_eq!(7, inventory.get_total_count());
    //     // Root container has items 1-3 and the bag at the top level
    //     assert_eq!(4, inventory.get_top_level_count());
    //
    //     // AND the level has been setup with the player inventory
    //     let mut level = build_player_test_level();
    //     level.characters.get_player_mut().unwrap().set_inventory(inventory);
    //
    //     // WHEN we try to move an item from the bag into the root container
    //     let data = MoveItemsRequest {
    //         source_container: source,
    //         to_move,
    //         target_container: Some(target),
    //         target_position_item: None,
    //         source_position: None,
    //         target_position: None
    //     };
    //     let result = move_player_items(data, &mut level);
    //
    //     // THEN we expect a result to return
    //     assert!(result.is_ok());
    //
    //     if let Ok(response) = result {
    //         // with 0 unmoved items
    //         assert_eq!(0, response.unmoved.len());
    //     } else {
    //         assert!(false, "Unexpected data type returned");
    //     }
    //
    //     let updated_inventory = level.characters.get_player_mut().unwrap().get_inventory_mut();
    //     // AND the player's inventory should not have 5 items in it's content count
    //     assert_eq!(5, updated_inventory.get_top_level_count());
    //     // AND The bag should have only 2 items now
    //     if let Some(b) = updated_inventory.find(&bag_item) {
    //         assert_eq!(2, b.get_top_level_count());
    //     } else {
    //         assert!(false, "Couldn't find Bag in the updated inventory!");
    //     }
    //
    // }
    //
    // #[test]
    // fn test_move_items_between() {
    //
    // }
}