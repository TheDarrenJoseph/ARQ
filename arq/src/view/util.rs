use crate::engine::event::container::MoveToContainerChoiceData;
use crate::error::errors::ErrorWrapper;
use crate::view::framehandler::container_choice::ContainerChoiceFrameHandler;

pub mod widget_menu;
pub mod callback;
pub mod progress_display;
pub mod cell_builder;

// Validates the MoveToContainerChoiceData, and then returns a container choice frame handler if it's OK
pub fn try_build_container_choice_frame_handler(data : &MoveToContainerChoiceData) -> Result<ContainerChoiceFrameHandler, ErrorWrapper> {
    if data.to_move.is_empty() {
        return ErrorWrapper::internal_result(String::from("Must select items before attempting to move them to a nearby container."));
    }

    if data.choices.is_empty() {
        return ErrorWrapper::internal_result(String::from("No nearby containers found."));
    }

    let choices = data.choices.clone();
    let mut items = Vec::new();
    for c in & choices {
        items.push(c.get_self_item().clone());
    }
    ContainerChoiceFrameHandler::build(&choices)
}
