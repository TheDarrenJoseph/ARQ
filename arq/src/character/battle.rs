use crate::character::characters::Characters;

#[derive(Default, Clone, Debug)]
pub struct Battle {
    pub characters: Characters,
    pub in_progress : bool
}

impl Battle {
    fn begin(&mut self) {
        self.in_progress = true;
    }

    fn end(&mut self) {
        self.in_progress = false;
    }
}