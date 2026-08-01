
#[derive(Clone)]
#[derive(Debug)]

pub struct MappedOption<T> {
    pub mapped : T,
    pub name : String,
    pub size : i8
}

// Selection state for a series of text / Column options
#[derive(Clone)]
#[derive(Debug)]
pub struct OptionListSelection<T> {
    pub options: Vec<MappedOption<T>>,
    pub index: u16
}

impl<T> OptionListSelection<T> {
    pub fn new() -> OptionListSelection<T> {
        OptionListSelection {
            options: vec![],
            index: 0
        }
    }

    pub fn get_chosen(&self) -> Option<&MappedOption<T>> {
        self.options.get(self.index as usize)
    }

    pub fn move_up(&mut self) {
        if self.index > 0 {
            self.index -= 1;
        }
    }

    pub fn move_down(&mut self) {
        if self.index < self.options.len() as u16 {
            self.index += 1;
        }
    }
}