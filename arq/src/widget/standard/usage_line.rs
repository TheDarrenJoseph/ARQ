use ratatui::buffer::Buffer;
use ratatui::layout::Rect;
use ratatui::prelude::{Style, Widget};

#[derive(Eq, Hash, PartialEq)]
#[derive(Debug)]
#[derive(Clone)]
pub struct UsageCommand  {
    pub key : char,
    pub description : String
}

impl UsageCommand {
    pub const fn new(key: char, description: String) -> Self {
        UsageCommand { key, description}
    }

    pub fn get_key(self) -> char {
        self.key
    }
    pub fn get_description(self) -> String {
        self.description
    }
    fn describe_usage(&self) -> String {
        format!("{} - {}", self.key, self.description)
    }
}

#[derive(Clone)]
#[derive(Debug)]
#[derive(PartialEq, Eq)]
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
            UsageCommand::new('i', String::from("Inventory") ),
            UsageCommand::new('o', String::from("Open") ),
            UsageCommand::new('k', String::from("Look") )
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
