use crate::error::errors::ErrorType::{DISPLAYABLE, INTERNAL, IO};
use std::fmt::{Debug, Display, Formatter};

pub enum ErrorType {
    // For errors that we can display to the player via console, etc
    DISPLAYABLE,
    // For internal error messaging, sometimes these may be used to determine a player-friendly error
    INTERNAL,
    IO
}

// TODO hookup for messages
pub struct ErrorWrapper {
    pub(crate) displayable_message: Option<String>, // For player-friendly errors
    pub(crate) internal_message: Option<String>, // For non-critical / system errors
    pub(crate) io_error: Option<std::io::Error>,
    pub(crate) error_type: ErrorType
}

impl ErrorWrapper {
    pub(crate) const fn new_internal(message: String) -> ErrorWrapper {
        ErrorWrapper { displayable_message: None, internal_message: Some(message), io_error: None,  error_type: INTERNAL }
    }

    pub(crate) const fn new_displayable(player_message: String, internal_message: String) -> ErrorWrapper {
        ErrorWrapper { displayable_message: Some(player_message), internal_message: Some(internal_message), io_error: None,  error_type: DISPLAYABLE }
    }

    pub(crate) const fn internal_result<T>(message: String) -> Result<T, ErrorWrapper>  {
        return Err(ErrorWrapper::new_internal(message));
    }
    
    pub(crate) fn io_error_result<T>(error: std::io::Error) -> Result<T, ErrorWrapper> {
        Err(ErrorWrapper::from(error))
    }
}

impl From<std::io::Error> for ErrorWrapper {
    fn from(io_error: std::io::Error) -> ErrorWrapper {
        ErrorWrapper { displayable_message: None, internal_message: None, io_error: Some(io_error),  error_type: IO }
    }
}

impl Debug for ErrorWrapper {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        match self.error_type {
            ErrorType::INTERNAL => {
                write!(f, "{}", self.internal_message.as_ref().unwrap())
            },
            ErrorType::DISPLAYABLE => {
                write!(f, "{}", self.displayable_message.as_ref().unwrap())
            },
            IO => {
                write!(f, "{}", self.io_error.as_ref().unwrap())
            }
        }
    }
}

impl Display for ErrorWrapper {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        let error_type= &self.error_type;
        match error_type {
            ErrorType::INTERNAL => {
                write!(f, "{}", self.internal_message.as_ref().unwrap())
            },
            ErrorType::DISPLAYABLE => {
                write!(f, "{}", self.displayable_message.as_ref().unwrap())
            },
            IO => {
                write!(f, "{}", self.io_error.as_ref().unwrap())
            }
        }  
    }
}

pub fn error_result<T>(message: String) -> Result<T, ErrorWrapper> {
    return Err(ErrorWrapper::new_internal(message));
}