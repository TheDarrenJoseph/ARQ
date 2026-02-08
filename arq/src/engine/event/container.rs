use crate::view::framehandler::container::{ContainerTarget, DropItemsRequest, DropItemsResponse, MoveItemsRequest, MoveItemsResponse, MoveItemsToContainerRequest, OpenContainerRequest, TakeItemsRequest, TakeItemsResponse};

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
#[derive(Debug)]
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