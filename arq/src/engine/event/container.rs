use crate::map::objects::container::Container;
use crate::map::objects::items::Item;
use crate::map::position::Position;
use crate::view::framehandler::util::tabling::Column;
use uuid::Uuid;

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum OpenedContainerEventType {
    // Generic Container Events
    Close,
    OpenContainer,
    // World Container Events
    TakeItems,
    TakeItemsResult,
    // Character Inventory Specific Events
    DropItems,
    DropItemsResult,
    // This move items only contains the selected items and target container
    // And is further handled by the command layer
    // 1. ContainerWidget -> CharacterInfoCommand
    MoveItems,
    MoveItemsResult,
    // Part of the moving items to container event chain
    // This is used to call from the widget to the command only:
    // 1. ContainerWidget -> CharacterInfoCommand
    ShowMoveContainerChoices,
    // Having selected a specific container to move items into
    MoveToContainerChoiceSelection,
    MoveToContainerChoiceResult,
    // Part of item equipment selection
    EquipItems,
    EquipItemsResult
}

// Specifying the request types used for a specific OpenedContainerEventType
#[derive(Debug, Clone)]
pub enum OpenedContainerEventData {
    OpenContainer(OpenContainerRequest),
    TakeItems(TakeItemsRequest),
    TakeItemsResult(TakeItemsResponse),
    DropItems(DropItemsRequest),
    DropItemsResult(DropItemsResponse),
    // For moving items around in a single source container
    MoveItemsWithinSource(MoveItemsWithinSourceRequest),
    MoveItemsWithSourceResult(MoveItemsResponseV2),
    // Request to start Container Choices
    MoveItemsToContainerChoice(ContainerChoicesRequest),
    MoveItemsToContainerChoiceSelection(TargetContainerScope),
    MoveItemsToContainerChoiceResult(MoveItemsResponseV2),
}


// For opening a container while browsing a map container
#[derive(Clone, Debug, PartialEq)]
pub struct OpenContainerRequest {
    pub source_container_id: Uuid,
    pub target: Container
}

#[derive(Clone, Debug, PartialEq)]
pub struct TakeItemsRequest {
    pub source: Container,
    pub to_take: Vec<Item>,
    pub position: Option<Position>
}

#[derive(Clone, Debug, PartialEq)]
pub struct TakeItemsResponse {
    pub container_id: Uuid,
    pub untaken: Vec<Item>,
    pub message: String
}

#[derive(Clone, Debug, PartialEq)]
pub struct DropItemsRequest {
    pub source: Container,
    pub to_drop: Vec<Item>,
    pub position: Option<Position>
}

#[derive(Clone, Debug, PartialEq)]
pub struct DropItemsResponse {
    pub container_id: Uuid,
    pub undropped: Vec<Item>,
    pub message: String
}

#[derive(Clone, Debug)]
pub struct MoveToContainerChoiceData {
    pub source: Container,
    pub to_move: Vec<Item>,
    pub position: Option<Position>,
    pub choices: Vec<Container>,
    pub target_container: Option<Container>
}

#[derive(Clone, Debug)]
pub struct MoveItemsWithinSourceRequest {
    pub source_container: Container,
    pub to_move: Vec<Item>,
    pub target_container: Option<Container>,
    // for moving items to a specific position
    pub target_position_item: Option<Item>
}

#[derive(Clone, Debug)]
pub struct MoveItemsResponse {
    pub source: Container,
    pub unmoved: Vec<Item>,
    pub target_container: Option<Container>,
    pub position: Option<Position>,
    pub message: String
}

#[derive(Clone, Debug)]
pub struct PlayerInventoryContainer {
    pub container: Container
}

#[derive(Clone, Debug)]
pub struct WorldContainer {
    pub container: Container,
    pub position: Position,
}

// For moving items to a particular spot in a container
#[derive(Clone, Debug)]
pub struct PlayerInventoryItemPosition {
    pub target_position_item: Item
}

// For moving items to a particular spot in a container
#[derive(Clone, Debug)]
pub struct WorldContainerItemPosition {
    pub target_position_item: Item,
    pub position: Position,
}

#[derive(Clone, Debug)]
pub enum SourceContainerScope {
    PlayerInventory(PlayerInventoryContainer),
    WorldContainer(WorldContainer)
}

#[derive(Clone, Debug)]
pub enum TargetContainerScope {
    PlayerInventory(PlayerInventoryContainer),
    PlayerInventoryItemPosition(PlayerInventoryItemPosition),
    WorldContainer(WorldContainer),
    WorldContainerItemPosition(WorldContainerItemPosition)
}

impl SourceContainerScope {

    pub fn get_self_item(&self) -> &Item {
        match self {
            SourceContainerScope::PlayerInventory(other_pic) => {
                &other_pic.container.get_self_item()
            },
            SourceContainerScope::WorldContainer(other_wc) => {
                &other_wc.container.get_self_item()
            }
        }
    }

    pub fn get_container(&self) -> &Container {
        match self {
            SourceContainerScope::PlayerInventory(other_pic) => {
                &other_pic.container
            },
            SourceContainerScope::WorldContainer(other_wc) => {
                &other_wc.container
            }
        }
    }

    pub fn is_player_scope(&self) -> bool {
        match self {
            SourceContainerScope::PlayerInventory(_pic) => {
               true
            },
            SourceContainerScope::WorldContainer(_wc) => {
               false
            }
        }
    }

    pub fn is_world_scope(&self) -> bool {
        match self {
            SourceContainerScope::PlayerInventory(_pic) => {
                false
            },
            SourceContainerScope::WorldContainer(_wc) => {
                true
            }
        }
    }

    pub fn matches(&self, other: &SourceContainerScope) -> bool {
        match other {
            SourceContainerScope::PlayerInventory(other_pic) => {
                self.matches_item(other_pic.container.get_self_item())
            },
            SourceContainerScope::WorldContainer(other_wc) => {
                self.matches_item(other_wc.container.get_self_item())
            }
        }
    }

    pub fn matches_target(&self, other: &TargetContainerScope) -> bool {
        match other {
            TargetContainerScope::PlayerInventory(other_pic) => {
                self.matches_item(other_pic.container.get_self_item())
            },
            TargetContainerScope::WorldContainer(other_wc) => {
                self.matches_item(other_wc.container.get_self_item())
            },
            _ => {
                false
            }
        }
    }

    pub fn matches_item(&self, item: &Item) -> bool {
        match self {
            SourceContainerScope::PlayerInventory(pic) => {
                pic.container.id_equals_uuid(item.get_id())
            },
            SourceContainerScope::WorldContainer(wc) => {
                wc.container.id_equals_uuid(item.get_id())
            }
        }
    }
}

impl TargetContainerScope {

    pub fn get_self_item(&self) -> &Item {
        match self {
            TargetContainerScope::PlayerInventory(other_pic) => {
                &other_pic.container.get_self_item()
            },
            TargetContainerScope::PlayerInventoryItemPosition(piip) => {
                &piip.target_position_item
            },
            TargetContainerScope::WorldContainer(other_wc) => {
                &other_wc.container.get_self_item()
            },
            TargetContainerScope::WorldContainerItemPosition(wcip) => {
                &wcip.target_position_item
            },
        }
    }

    pub fn get_container(&self) -> Option<Container> {
        match self {
            TargetContainerScope::PlayerInventory(other_pic) => {
                Some(other_pic.container.clone())
            },
            TargetContainerScope::PlayerInventoryItemPosition(_piip) => {
                None
            },
            TargetContainerScope::WorldContainer(other_wc) => {
                Some(other_wc.container.clone())
            }
            TargetContainerScope::WorldContainerItemPosition(_wcip) => {
                None
            }
        }
    }

    pub fn is_targeting_another_container(&self) -> bool {
        match self {
            TargetContainerScope::PlayerInventory(_other_pic) => {
                true
            },
            TargetContainerScope::PlayerInventoryItemPosition(_piip) => {
                false
            },
            TargetContainerScope::WorldContainer(_other_wc) => {
                true
            }
            TargetContainerScope::WorldContainerItemPosition(_wcip) => {
                false
            }
        }
    }

    pub fn is_targeting_item_position(&self) -> bool {
        match self {
            TargetContainerScope::PlayerInventory(_other_pic) => {
                false
            },
            TargetContainerScope::PlayerInventoryItemPosition(_piip) => {
                true
            },
            TargetContainerScope::WorldContainer(_other_wc) => {
                false
            }
            TargetContainerScope::WorldContainerItemPosition(_wcip) => {
                true
            }
        }
    }

    pub fn is_player_scope(&self) -> bool {
        match self {
            TargetContainerScope::PlayerInventory(_pic) => {
                true
            },
            TargetContainerScope::PlayerInventoryItemPosition(_piip) => {
                true
            },
            TargetContainerScope::WorldContainer(_wc) => {
                false
            }
            TargetContainerScope::WorldContainerItemPosition(_wcip) => {
                false
            }
        }
    }

    pub fn is_world_scope(&self) -> bool {
        match self {
            TargetContainerScope::PlayerInventory(_pic) => {
                false
            },
            TargetContainerScope::PlayerInventoryItemPosition(_piip) => {
                false
            },
            TargetContainerScope::WorldContainer(_wc) => {
                true
            }
            TargetContainerScope::WorldContainerItemPosition(_wcip) => {
                true
            }
        }
    }

    pub fn matches_source(&self, other: &SourceContainerScope) -> bool {
        match other {
            SourceContainerScope::PlayerInventory(other_pic) => {
                self.matches_item(other_pic.container.get_self_item())
            },
            SourceContainerScope::WorldContainer(other_wc) => {
                self.matches_item(other_wc.container.get_self_item())
            }
        }
    }

    pub fn matches_item(&self, item: &Item) -> bool {
        match self {
            TargetContainerScope::PlayerInventory(pic) => {
                pic.container.id_equals_uuid(item.get_id())
            },
            TargetContainerScope::WorldContainer(wc) => {
                wc.container.id_equals_uuid(item.get_id())
            },
            _ => { false }
        }
    }

    pub fn build_container_choice_column_text(&self, column: &Column) -> String {
        let container = match self {
            TargetContainerScope::PlayerInventory(pic) => {
                &pic.container
            },
            TargetContainerScope::WorldContainer(wc) => {
                &wc.container
            },
            _ => { return String::from("N/a"); }
        };

        match column.name.as_str() {
            "NAME" => {
                container.get_self_item().get_name()
            },
            "STORAGE (Kg)" => {
                format!("{}/{}", container.get_weight_total(), container.get_weight_limit())
            },
            _ => { "".to_string() }
        }
    }
}

#[derive(Clone, Debug)]
pub struct MoveItemsRequestV2 {
    pub source: SourceContainerScope,
    pub target: TargetContainerScope,
    pub to_move: Vec<Item>,
}

#[derive(Clone, Debug)]
pub struct UpdatedScopes {
    pub source: SourceContainerScope,
    pub target: TargetContainerScope,
}

#[derive(Clone, Debug)]
pub struct MoveItemsResponseV2 {
    pub request: MoveItemsRequestV2,
    pub success: bool,
    pub updated_scopes: UpdatedScopes,
    pub unmoved: Vec<Item>,
    pub message: String
}

impl MoveItemsResponseV2 {
    pub fn all_items_moved(&self) -> bool {
        self.unmoved.is_empty()
    }
}

#[derive(Clone, Debug)]
pub struct ContainerChoicesRequest {
    pub source: Container,
    pub to_move: Vec<Item>,
    pub position: Option<Position>
}

#[derive(Clone, Debug)]
pub struct ContainerTarget {
    pub target_container_id: Uuid
}