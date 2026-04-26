use log::error;

#[derive(Clone)]
#[derive(Debug)]
#[derive(Eq)]
#[derive(PartialEq)]
pub struct MultiStepProgress {
    current_step_index: usize,
    steps: Vec<Step>
}

#[derive(Clone)]
#[derive(Debug)]
#[derive(Eq)]
#[derive(PartialEq)]
pub struct Step {
    pub id: String,
    pub description: String
}

impl MultiStepProgress {
    pub fn for_steps_not_started(steps: Vec<Step>) -> MultiStepProgress {
        MultiStepProgress { current_step_index: 0, steps }
    }

    pub fn get_current_step_value(&self) -> Option<&Step> {
        self.steps.get(self.current_step_index)
    }

    /*
    * Returns the current step, 1-indexed for human readability
    */
    pub fn get_current_step_number(&self) -> usize {
        self.current_step_index + 1 as usize
    }

    pub fn next_step(&mut self) {
        let next_index = self.current_step_index+1;
        let step_count = self.steps.len();
        if step_count > next_index {
            self.current_step_index = next_index;
        } else {
            error!("Cannot set next step, next stop would be: {} which is beyond the size of the steps: {}", next_index, step_count);
        }
    }

    pub fn step_count(&self) -> usize {
        self.steps.len()
    }

    pub fn steps(&self) -> &Vec<Step> {
        &self.steps
    }

    pub fn is_done(&self) -> bool {
        self.current_step_index == self.step_count() - 1
    }

    pub fn get_progress_percentage(&self) -> usize {
        (100 / self.steps.len()) * (self.current_step_index+1)
    }
}