use uuid::Uuid;
use crate::map::objects::container::Container;
use crate::map::objects::items::Item;
use crate::map::position::Position;

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
    MoveItemsToContainerChoice(MoveItemsToContainerRequest),
    SelectedContainer(ContainerTarget),
    MoveItemsToContainerChoiceResult(MoveItemsResponse),
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
pub struct MoveItemsBetweenRequest {
    pub source_container: Container,
    pub source_position: Position,
    pub target_container: Container,
    pub target_position: Position,
    pub to_move: Vec<Item>,
}

#[derive(Clone, Debug)]
pub struct MoveItemBetweenResponse {
    pub request: MoveItemsBetweenRequest,
    pub success: bool,
    pub unmoved: Vec<Item>,
    pub message: String
}

#[derive(Clone, Debug)]
pub struct MoveItemsToContainerRequest {
    pub source: Container,
    pub to_move: Vec<Item>,
    pub position: Option<Position>
}

#[derive(Clone, Debug)]
pub struct ContainerTarget {
    pub target_container_id: Uuid
}