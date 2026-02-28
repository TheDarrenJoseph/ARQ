use uuid::Uuid;
use crate::map::objects::container::Container;
use crate::map::objects::items::Item;
use crate::map::position::Position;
use crate::view::framehandler::util::tabling::Column;
use crate::widget::stateful::container_choice_widget::ContainerChoice;

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
    MoveItems,
    MoveItemsResult,
    // Part of the moving items to container event chain
    MoveItemsToContainerChoice,
    // Having selected a specific container to move items into
    MoveItemsToContainerChoiceSelection,
    MoveItemsToContainerChoiceResult,
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
    MoveItems(MoveItemsRequest),
    MoveItemsResult(MoveItemsResponse),
    // Request to start Container Choices
    MoveItemsToContainerChoice(ContainerChoicesRequest),
    MoveItemsToContainerChoiceSelection(ContainerScope),
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
pub struct MoveItemsRequest {
    pub source_container: Container,
    pub to_move: Vec<Item>,
    pub target_container: Option<Container>,
    // for moving items to a specific position
    pub target_position_item: Option<Item>,
    pub source_position: Option<Position>,
    pub target_position: Option<Position>
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

#[derive(Clone, Debug)]
pub enum ContainerScope {
    PlayerInventory(PlayerInventoryContainer),
    WorldContainer(WorldContainer)
}

impl ContainerScope {

    pub fn get_self_item(&self) -> &Item {
        match self {
            ContainerScope::PlayerInventory(other_pic) => {
                &other_pic.container.get_self_item()
            },
            ContainerScope::WorldContainer(other_wc) => {
                &other_wc.container.get_self_item()
            }
        }
    }

    pub fn get_container(&self) -> &Container {
        match self {
            ContainerScope::PlayerInventory(other_pic) => {
                &other_pic.container
            },
            ContainerScope::WorldContainer(other_wc) => {
                &other_wc.container
            }
        }
    }

    pub fn is_player_scope(&self) -> bool {
        match self {
            ContainerScope::PlayerInventory(pic) => {
               true
            },
            ContainerScope::WorldContainer(wc) => {
               false
            }
        }
    }

    pub fn is_world_scope(&self) -> bool {
        match self {
            ContainerScope::PlayerInventory(pic) => {
                false
            },
            ContainerScope::WorldContainer(wc) => {
                true
            }
        }
    }

    pub fn matches(&self, other: &ContainerScope) -> bool {
        match other {
            ContainerScope::PlayerInventory(other_pic) => {
                self.matches_item(other_pic.container.get_self_item())
            },
            ContainerScope::WorldContainer(other_wc) => {
                self.matches_item(other_wc.container.get_self_item())
            }
        }
    }
    pub fn matches_item(&self, item: &Item) -> bool {
        match self {
            ContainerScope::PlayerInventory(pic) => {
                pic.container.id_equals_uuid(item.get_id())
            },
            ContainerScope::WorldContainer(wc) => {
                wc.container.id_equals_uuid(item.get_id())
            }
        }
    }

    pub fn build_container_choice_column_text(&self, column: &Column) -> String {
        let container = match self {
            ContainerScope::PlayerInventory(pic) => {
                &pic.container
            },
            ContainerScope::WorldContainer(wc) => {
                &wc.container
            }
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
    pub source: ContainerScope,
    pub target: ContainerScope,
    pub to_move: Vec<Item>,
}

#[derive(Clone, Debug)]
pub struct MoveItemsResponseV2 {
    pub request: MoveItemsRequestV2,
    pub success: bool,
    pub unmoved: Vec<Item>,
    pub message: String
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