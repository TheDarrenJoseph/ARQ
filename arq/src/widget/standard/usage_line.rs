use ratatui::buffer::Buffer;
use ratatui::layout::Rect;
use ratatui::prelude::{Style, Widget};
use termion::event::Key;
use crate::engine::command::open_command::OpenedContainerEventType;
use crate::util::describe_key;

#[derive(Eq)]
#[derive(PartialEq)]
#[derive(Debug)]
#[derive(Clone)]
pub struct UsageCommand  {
    pub key : Key,
    pub description : String,
    // If needed, we can indicate the opened container event type to use for this
    pub opened_container_event_type: Option<OpenedContainerEventType>
}

impl UsageCommand {
    pub const fn new(key: Key, description: String) -> Self {
        UsageCommand { key, description, opened_container_event_type: None }
    }

    pub const fn for_container_event(key: Key, description: String, opened_container_event_type: OpenedContainerEventType) -> Self {
        UsageCommand { key, description, opened_container_event_type: Some(opened_container_event_type) }
    }


    pub fn get_key(self) -> Key {
        self.key
    }
    pub fn get_description(self) -> String {
        self.description
    }
    fn describe_usage(&self) -> String {
        format!("{} - {}", describe_key(self.key), self.description)
    }
}

#[derive(Clone)]
#[derive(Debug)]
#[derive(Eq)]
#[derive(PartialEq)]
pub struct UsageLineWidget {
    pub commands : Vec<UsageCommand>
}

impl Widget for UsageLineWidget {
    fn render(self, area: Rect, buf: &mut Buffer) where Self: Sized {
        let line = self.describe();
        buf.set_string(area.x, area.y, line.as_str(), Style::default());
    }
}


impl UsageLineWidget {
    pub fn new() -> Self {
        UsageLineWidget { commands: Self::default_commands() }
    }
    
    pub fn reset_commands(&mut self) {
        self.commands = Self::default_commands();
    }
    
    pub fn for_commands(commands : Vec<UsageCommand>) -> Self {
        UsageLineWidget { commands }
    }

    fn default_commands() -> Vec<UsageCommand> {
        vec![
            UsageCommand::new(Key::Char('i'), String::from("Inventory")),
            UsageCommand::new(Key::Char('o'), String::from("Open")),
            UsageCommand::new(Key::Char('k'), String::from("Look"))
        ]
    }
    
    pub fn describe(&self) -> String {
        let mut description = String::from("");
        let len = self.commands.len();
        let mut i = 0;
        for c in self.commands.iter() {
            description += c.describe_usage().as_str();
            if i < len - 1 {
                description += ", ";
            }
            i += 1;
        }
        description
    }
}
