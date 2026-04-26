use crate::map::objects::items::Item;
use std::collections::VecDeque;
use std::convert::TryInto;

#[derive(Debug, Clone)]
pub enum SelectionMode {
    SelectingItems
}

pub trait ListSelection {
    fn get_page_number(&self) -> i32;
    fn get_total_pages(&self) -> i32;
    fn get_start_index(&self) -> i32;
    fn get_container_index(&self) -> Option<i32>;
    fn get_true_index(&self) -> i32;
    fn get_focused_item(&self) -> Option<&Item>;
    fn get_end_of_page_index(&mut self) -> i32;
    fn get_items(&self) -> &Vec<Item>;
    fn get_items_mut(&mut self) -> &mut Vec<Item>;
    fn get_selected_items(&self) -> VecDeque<Item>;
    fn is_selecting(&self) -> bool;
    fn toggle_select(&mut self);
    fn cancel_selection(&mut self);
    fn is_selected(&self, index : i32) -> bool;
    fn is_focused(&self, index : i32) -> bool;
    fn select(&mut self, index : i32);
    fn deselect(&mut self, index : i32);
    fn page_up(&mut self);
    fn page_down(&mut self);
    fn move_up(&mut self);
    fn move_down(&mut self);
}

#[derive(Debug, Clone)]
pub struct ItemListSelection {
    selection_mode : SelectionMode,
    // The index of the topmost item on the screen, allows scrolling
    start_index: i32,
    // The pivot index is the 'initial' selected index
    pivot_index: Option<i32>,
    // --- These are for the focused lines
    // This is relative to the current page
    current_index: i32,
    // This is the current index + start offset to get the 'true' container index currently focused
    true_index: i32,
    // --- Set during selection of items
    previous_container_index: Option<i32>,
    container_index: Option<i32>,
    selecting_items: bool,
    // --- Storage of items in the selection
    selected_indices: VecDeque<i32>,
    selected_items: VecDeque<Item>,
    items : Vec<Item>,
    pub page_line_count: i32,
}

impl ItemListSelection {
    pub fn new(items : Vec<Item>, page_line_count: i32) -> ItemListSelection {
        let selection_mode = SelectionMode::SelectingItems;
        let start_index = 0;
        let current_index = 0;
        let true_index = 0;
        let pivot_index = None;
        let previous_container_index = None;
        let container_index = None;
        let selecting_items = false;
        let selected_indices = VecDeque::new();
        let selected_items = VecDeque::new();
        ItemListSelection { selection_mode, start_index, pivot_index, previous_container_index, current_index, true_index, container_index, selecting_items, selected_indices, selected_items, items, page_line_count }
    }

    fn reset_selection(&mut self) {
        self.start_index = 0;
        self.current_index = 0;
        self.true_index = 0;
        self.pivot_index = None;
        self.previous_container_index = None;
        self.container_index = None;
        self.selecting_items = false;
        self.selected_indices.clear();
        self.selected_items.clear();
    }

    fn get_current_index(&self) -> i32 {
        self.current_index.clone()
    }

    fn get_pivot_index(&self) -> i32 {
        match self.pivot_index {
            Some(idx) => { idx },
            None => { -1 }
        }
    }

    fn determine_max_scroll_index(&mut self) -> i32 {
        let item_count = self.items.len() as i32;
        let mut max_scroll_index = 0;
        if item_count > self.page_line_count {
            max_scroll_index = item_count - self.page_line_count;
        }
        max_scroll_index
    }

    pub fn determine_max_selection_index(&mut self) -> i32 {
        if self.items.is_empty() {
            return 0;
        } else {
            let remaining_item_count = self.items.len() as i32 - 1 - self.start_index;
            let max_selection_index = if remaining_item_count >= self.page_line_count { self.page_line_count.clone() - 1 } else { remaining_item_count };
            max_selection_index
        }
    }

    fn check_reducing_selection_below(&mut self) {
        match self.previous_container_index {
            Some(previous_container_index) => {
                match self.container_index {
                    Some(container_index) => {
                        // Deselect anything we were previously selecting
                        let reducing_selection_below = container_index < previous_container_index && container_index > self.get_pivot_index();
                        if reducing_selection_below || self.selected_start_position() {
                            for i in container_index+1..=previous_container_index {
                                self.deselect(i);
                            }
                        }
                    },
                    None => {}
                }
            },
            None => {}
        }
    }

    fn select_range(&mut self, start : i32, end: i32) {
        for i in start..end {
            self.select(i);
        }
    }

    fn deselect_range(&mut self, start : i32, end: i32) {
        for i in start..end {
            self.deselect(i);
        }
    }

    fn check_selecting_items_above(&mut self) {
        match self.previous_container_index {
            Some(previous_container_index) => {
                match self.container_index {
                    Some(container_index) => {
                        let selecting_items_above = container_index < previous_container_index && container_index < self.get_pivot_index();
                        if selecting_items_above {
                            for i in (container_index..=self.get_pivot_index()).rev() {
                                self.select(i);
                            }
                        }
                    }, None => {}
                }
            }, None => {}
        }
    }

    fn check_selecting_items_below(&mut self) {
        match self.previous_container_index {
            Some(previous_container_index) => {
                match self.container_index {
                    Some(container_index) => {
                        let selecting_items_below = container_index > previous_container_index && container_index > self.get_pivot_index();
                        if selecting_items_below {
                            for i in self.get_pivot_index()..=container_index {
                                self.select(i);
                            }
                        }
                    },
                    None => {}
                }
            },
            None => {}
        }
    }

    fn selected_start_position(&self) -> bool {
        self.container_index == Some(self.get_pivot_index())
    }

    fn check_reducing_selection_above(&mut self) {
        match self.previous_container_index {
            Some(previous_container_index) => {
                match self.container_index {
                    Some(container_index) => {
                        // Deselect anything we were previously selecting
                        let reducing_selection_above = container_index > previous_container_index && container_index < self.get_pivot_index();
                        if reducing_selection_above || self.selected_start_position() {
                            for i in previous_container_index.clone() - 1..container_index {
                                self.deselect(i);
                            }
                        }
                    },
                    None => {}
                }
            },
            None => {}
        }
    }

    fn update_indices(&mut self, index: i32) {
        self.current_index = index.clone();
        self.true_index = self.start_index.clone() + index.clone();

        // Update container indices
        if self.is_selecting() {
            match self.container_index {
                Some(previous_container_index) => {
                    self.previous_container_index = Some(previous_container_index);
                    let current_container_index = self.start_index.clone() + index.clone();
                    self.container_index = Some(current_container_index);
                },
                None => {
                    let previous_current_index = self.current_index.clone();
                    self.previous_container_index = Some(previous_current_index);
                    let current_container_index = self.start_index.clone() + index.clone();
                    self.container_index = Some(current_container_index);
                }
            }

        }
    }

    fn has_pivot_point(&self) -> bool {
        match self.pivot_index {
            Some(_) => {
                true
            },
            None => {
                false
            }
        }
    }

    pub fn update_selection(&mut self, index : i32) {
        self.update_indices(index);
        match self.previous_container_index {
            Some(previous_container_index) => {
                match self.container_index {
                    Some(container_index) => {
                        let selection_changed = container_index != previous_container_index;
                        if selection_changed && self.selecting_items {
                            self.check_selecting_items_above();
                            self.check_reducing_selection_above();
                            self.check_selecting_items_below();
                            self.check_reducing_selection_below();
                        }
                    },
                    None => {}
                }
            },
            None => {}
        }
    }

    fn should_scroll_up(&mut self, selection_index: i32) -> bool {
        let selection_at_start_of_a_page = selection_index % self.page_line_count == 0;
        let can_scroll_up_one = self.start_index >= 1;
        selection_at_start_of_a_page && can_scroll_up_one
    }

    fn should_scroll_down(&mut self, selection_index: i32) -> bool {
        let max_scroll_index = self.determine_max_scroll_index();
        let max_selection_index = self.determine_max_selection_index();
        let end_of_page = selection_index == max_selection_index;
        let can_scroll = self.start_index <= max_scroll_index;
        return end_of_page && can_scroll;
    }

    fn should_turn_to_previous_page(&mut self, selection_index: i32) -> bool {
        let selection_at_start_of_a_page = selection_index % self.page_line_count == 0;
        let can_turn_back = self.start_index >= self.page_line_count;
        selection_at_start_of_a_page && can_turn_back
    }

    fn should_turn_to_next_page(&mut self, selection_index: i32) -> bool {
        let more_pages = self.get_page_number() < self.get_total_pages();
        let max_scroll_index = self.determine_max_scroll_index();
        let max_selection_index = self.determine_max_selection_index();
        let end_of_page = selection_index == max_selection_index;
        let remaining_item_count = self.items.len() as i32 - 1  - self.start_index;
        let can_scroll = self.start_index <= max_scroll_index && remaining_item_count > 0;
        return more_pages && end_of_page && can_scroll;
    }

    fn get_selected_count(&self) -> i32 {
        self.selected_indices.len() as i32
    }

    fn set_current_index(&mut self, index : i32) {
        self.current_index = index;
    }

    fn set_initial_selection(&mut self, index : i32) {
        let container_index = index + self.start_index.clone();
        self.selecting_items = true;
        self.select(container_index);
        self.update_selection(index.clone());
    }

    fn get_item(&self, index: i32) -> Option<&Item> {
        let items_size : i32 = self.items.len().try_into().unwrap();
        if index >= 0 && index < items_size {
            return Some(&self.items[index as usize])
        }
        None
    }

    fn get_item_mut(&mut self, index: i32) -> Option<&mut Item> {
        let items_size : i32 = self.items.len().try_into().unwrap();
        if index >= 0 && index < items_size {
            return Some(&mut self.items[index as usize])
        }
        None
    }
}

impl ListSelection for ItemListSelection {
    fn get_page_number(&self) -> i32 {
        (self.start_index.clone() / self.page_line_count) + 1
    }

    fn get_total_pages(&self) -> i32 {
        if self.items.len() == 0 {
            0
        } else if (self.items.len() as i32) < self.page_line_count.clone() {
             1
        } else {
            let page_count = self.items.len() as f32 / self.page_line_count.clone() as f32;
            page_count.ceil() as i32
        }
    }

    fn get_start_index(&self) -> i32 {
        self.start_index.clone()
    }

    fn get_container_index(&self) -> Option<i32> {
        self.container_index.clone()
    }

    fn get_true_index(&self) -> i32 {
        self.true_index.clone()
    }

    fn get_focused_item(&self) -> Option<&Item> {
        self.get_item(self.true_index.clone())
    }

    fn get_end_of_page_index(&mut self) -> i32 {
        self.start_index.clone() + self.determine_max_selection_index()
    }

    fn get_items(&self) -> &Vec<Item> {
        &self.items
    }

    fn get_items_mut(&mut self) -> &mut Vec<Item> {
        &mut self.items
    }

    fn get_selected_items(&self) -> VecDeque<Item> {
        self.selected_items.clone()
    }

    fn is_selecting(&self) -> bool {
        self.selecting_items.clone() || !self.selected_items.is_empty()
    }

    fn toggle_select(&mut self) {
        self.selecting_items = !self.selecting_items.clone();
        // Select the current item
        if self.selecting_items {
            // Reset the pivot index and select
            self.pivot_index = Some(self.true_index.clone());
            self.select( self.true_index.clone());
            self.update_indices( self.current_index.clone());
        }
    }

    fn cancel_selection(&mut self) {
        self.selecting_items = false;
        self.selected_items.clear();
        self.selected_indices.clear();
        self.pivot_index = None;
        self.container_index = None;
    }

    fn is_selected(&self, index : i32) -> bool {
        self.selected_indices.contains(&index)
    }

    fn is_focused(&self, index : i32) -> bool {
        let true_index = self.start_index + self.current_index;
        true_index == index
    }

    fn select(&mut self, index : i32) {
        // Set the pivot if not set
        if !self.has_pivot_point() {
            self.pivot_index = Some(index.clone());
        }

        if !self.is_selected(index) {
            let item_result = self.items.get(index.clone() as usize);
            match item_result {
                Some(item) => {
                    match self.previous_container_index {
                        Some(previous_container_index) => {
                            if previous_container_index <= index {
                                self.selected_indices.push_back(index.clone());
                                self.selected_items.push_back(item.clone());
                            } else {
                                self.selected_indices.push_front(index.clone());
                                self.selected_items.push_front(item.clone());
                            }
                        },
                        None => {
                            self.selected_indices.push_front(index.clone());
                            self.selected_items.push_front(item.clone());
                        }
                    }
                },
                None => {}
            }
        }
    }

    fn deselect(&mut self, index : i32) {
        if self.is_selected(index) {
            let item_result = self.items.get(index.clone() as usize);
            match item_result {
                Some(_) => {
                    let true_index = self.selected_indices.binary_search(&index).unwrap();
                    self.selected_indices.remove(true_index.clone() as usize);
                    self.selected_items.remove(true_index.clone() as usize);
                    match self.container_index {
                        Some(idx) => {
                            if idx == index {
                                self.container_index = None
                            }
                        },
                        None => {}
                    }
                },
                None => {}
            }
        }
    }

    fn page_up(&mut self) {
        let mut new_index = self.current_index.clone();
        if self.should_turn_to_previous_page(self.current_index.clone()) {
            self.start_index = self.start_index - self.page_line_count;
            if self.selecting_items {
                self.select_range(new_index, self.current_index.clone());
            }
            new_index = self.determine_max_selection_index();
        } else if self.current_index == 0 {
            new_index = 0;
            self.start_index = 0;
        } else {
            new_index = 0;
        }
        self.update_selection(new_index);
    }

    fn page_down(&mut self) {
        let new_index;
        if self.should_turn_to_next_page(self.current_index.clone()) {
            // Reset the selection index to 0 and the start index to begin the new page
            new_index = 0;
            self.start_index += self.page_line_count;
        } else {
            // Select the lowest item
            let max_selection_index = self.determine_max_selection_index();
            new_index = max_selection_index;
        }
        self.update_selection(new_index);
    }

    fn move_up(&mut self) {
        let mut new_index = self.current_index.clone();
        if self.current_index > 0 {
            new_index = self.current_index.clone() - 1;
        } else if self.should_scroll_up(self.current_index.clone()) {
            // Scroll up a line
            self.start_index -= 1;
        } else if self.current_index == 0 && self.start_index > 0 {
            new_index = self.current_index.clone() - 1;
        }
        self.update_selection(new_index);
    }

    fn move_down(&mut self) {
        let mut new_index = self.current_index.clone();
        let max_selection_index = self.determine_max_selection_index();
        let true_index = self.start_index.clone() + new_index;
        let valid_index = true_index < (self.items.len() as i32) as i32;
        let less_than_max = self.current_index < max_selection_index;
        if valid_index && less_than_max {
            new_index = self.current_index.clone() + 1;
        } else if valid_index && self.should_scroll_down(self.current_index.clone()) {
            // Bump the start index forward to scroll
            self.start_index += 1;
        }
        self.update_selection(new_index);
    }

}

#[cfg(test)]
mod tests {
    use crate::item_list_selection::{ItemListSelection, ListSelection};
    use crate::map::objects::items::Item;

    fn build_item_series_4() -> Vec<Item> {
        let item = Item::with_defaults("Test Item 1".to_owned(),  1.0, 1);
        let item2 = Item::with_defaults( "Test Item 2".to_owned(), 1.0, 1);
        let item3 = Item::with_defaults("Test Item 3".to_owned(),  1.0, 1);
        let item4 = Item::with_defaults("Test Item 4".to_owned(), 1.0, 1);
        let items = vec! [ item.clone(), item2.clone(), item3.clone(), item4.clone() ];
        items
    }

    fn build_item_series_8() -> Vec<Item> {
        let item1 = Item::with_defaults("Test Item 1".to_owned(),  1.0, 1);
        let item2 = Item::with_defaults("Test Item 2".to_owned(),  1.0, 1);
        let item3 = Item::with_defaults("Test Item 3".to_owned(), 1.0, 1);
        let item4 = Item::with_defaults("Test Item 4".to_owned(), 1.0, 1);
        let item5 = Item::with_defaults("Test Item 5".to_owned(),  1.0, 1);
        let item6 = Item::with_defaults("Test Item 6".to_owned(),  1.0, 1);
        let item7 = Item::with_defaults("Test Item 7".to_owned(),  1.0, 1);
        let item8 = Item::with_defaults("Test Item 8".to_owned(),  1.0, 1);
        vec! [ item1.clone(), item2.clone(), item3.clone(), item4.clone(), item5.clone(), item6.clone(), item7.clone(), item8.clone()  ]
    }

    #[test]
    fn test_item_list_selection_new(){
        // GIVEN a series of items to select from            let test_item = Item::with_defaults(format!("Test Item {}", i), 1, 100);
        let items = build_item_series_4();

        // WHEN we call to build a list selection of these items
        let list_selection = ItemListSelection::new(items.clone(), 4);
        assert_eq!(1, list_selection.get_page_number());
        assert_eq!(1, list_selection.get_total_pages());

        // THEN we expect it to wrap the items provided
        let wrapped_items = list_selection.get_items();
        assert_eq!(4, wrapped_items.len());
        assert_eq!(items[0], wrapped_items[0]);
        assert_eq!(items[1], wrapped_items[1]);
        assert_eq!(items[2], wrapped_items[2]);
        assert_eq!(items[3], wrapped_items[3]);

        // AND have no currently selected items
        let selected_items = list_selection.get_selected_items();
        assert_eq!(0, selected_items.len());
    }

    #[test]
    fn test_multi_page_count() {
        // GIVEN a series of items to select from
        let item = Item::with_defaults("Test Item 1".to_owned(),  1.0, 1);
        let item2 = Item::with_defaults( "Test Item 2".to_owned(), 1.0, 1);
        let item3 = Item::with_defaults("Test Item 3".to_owned(),  1.0, 1);
        let item4 = Item::with_defaults("Test Item 4".to_owned(), 1.0, 1);
        let items = vec! [ item.clone(), item2.clone(), item3.clone(), item4.clone() ];

        // WHEN we call to build a list selection of these items with a line count of 2 items per page
        let list_selection = ItemListSelection::new(items.clone(), 2);
        // THEN we expect there to be 2 pages
        assert_eq!(1, list_selection.get_page_number());
        assert_eq!(2, list_selection.get_total_pages());
    }

    #[test]
    fn test_multi_page_count_odd() {
        // GIVEN a series of 60 items to select from
        let mut items = Vec::new();
        for _i in 1..=60 {
            let item = Item::with_defaults("Test Item 1".to_owned(),  1.0, 1);
            items.push(item);
        }

        // WHEN we call to build a list selection of these items
        // with a line count of 36 items per page (Which would give us an odd page count of 1.69)
        let list_selection = ItemListSelection::new(items.clone(), 36);
        // THEN we expect there to be 2 pages total
        assert_eq!(1, list_selection.get_page_number());
        assert_eq!(2, list_selection.get_total_pages());
    }

    #[test]
    fn test_move_up() {
        // GIVEN a selection with a series of items to select from
        let items = build_item_series_4();

        let mut list_selection = ItemListSelection::new(items.clone(), 4);

        // AND an initial container index of 2
        list_selection.set_current_index(2);

        // WHEN we call to move up
        list_selection.move_up();

        // THEN we expect the container index to be 1 (2nd item)
        assert_eq!(1, list_selection.get_current_index());
        // AND we should have no currently selected items
        let selected_items = list_selection.get_selected_items();
        assert_eq!(0, selected_items.len());
    }

    #[test]
    fn test_move_down_up_of_list() {
        // GIVEN a selection with a series of items to select from
        let items = build_item_series_4();
        let mut list_selection = ItemListSelection::new(items.clone(), 4);
        // AND an initial container index of 3
        list_selection.set_current_index(3);

        // WHEN we move up 3 times
        list_selection.move_up();
        list_selection.move_up();
        list_selection.move_up();

        // THEN we expect the container index to be 0 (1st item)
        assert_eq!(0, list_selection.get_current_index());
        // AND we should have no currently selected items
        let selected_items = list_selection.get_selected_items();
        assert_eq!(0, selected_items.len());
    }

    #[test]
    fn test_move_down() {
        // GIVEN a selection with a series of items to select from
        let items = build_item_series_4();
        let mut list_selection = ItemListSelection::new(items.clone(), 4);
        list_selection.set_current_index(0);

        // WHEN we call to move down
        list_selection.move_down();

        // THEN we expect the current index to be 1 (2nd item)
        assert_eq!(1, list_selection.get_current_index());
        // AND we should have no currently selected items
        let selected_items = list_selection.get_selected_items();
        assert_eq!(0, selected_items.len());
    }

    #[test]
    fn test_move_down_end_of_list() {
        // GIVEN a selection with a series of items to select from
        let items = build_item_series_4();
        let mut list_selection = ItemListSelection::new(items.clone(), 4);
        list_selection.set_current_index(0);

        // WHEN we move down 3 times
        list_selection.move_down();
        list_selection.move_down();
        list_selection.move_down();

        // THEN we expect the container index to be 3 (4th item)
        assert_eq!(3, list_selection.get_current_index());
        // AND we should have no currently selected items
        let selected_items = list_selection.get_selected_items();
        assert_eq!(0, selected_items.len());
    }

    #[test]
    fn test_toggle_selection() {
        // GIVEN a list selection
        let mut list_selection = ItemListSelection::new(Vec::new(), 0);
        // AND it's currently not selecting anything
        assert!(!list_selection.is_selecting());
        // WHEN we call to toggle selection
        list_selection.toggle_select();
        // THEN it will flip the is selecting state
        assert!(list_selection.is_selecting());
        // AND the opposite
        list_selection.toggle_select();
        assert!(!list_selection.is_selecting());
    }

    #[test]
    fn test_index_select() {
        // GIVEN a series of items to select from
        let items = build_item_series_4();

        // AND a valid list selection
        let mut list_selection = ItemListSelection::new(items.clone(), 4);

        // WHEN we call to select a particular index
        list_selection.select(1);

        // THEN we expect it to be returned
        assert!(list_selection.is_selected(1));
        let selected_items = list_selection.get_selected_items();
        assert_eq!(1, selected_items.len());
        assert_eq!(items[1], selected_items[0]);
    }

    #[test]
    fn test_selecting_above() {
        // GIVEN a series of items to select from
        let items = build_item_series_4();

        // AND a valid list selection
        let mut list_selection = ItemListSelection::new(items.clone(), 4);

        // AND we've begun by selecting an index  and ensuring we're selecting items
        list_selection.set_initial_selection(1);
        // WHEN we call to move up the selection
        list_selection.move_up();

        // THEN we expect multiple items/indices to be returned
        assert_eq!(true, list_selection.is_selecting());
        assert_eq!(true, list_selection.is_selected(0));
        assert_eq!(true, list_selection.is_selected(1));
        let selected_items = list_selection.get_selected_items();
        assert_eq!(2, selected_items.len());
        assert_eq!(items[0], selected_items[0]);
        assert_eq!(items[1], selected_items[1]);
    }

    #[test]
    fn test_selecting_above_multi() {
        // GIVEN a series of items to select from
        let items = build_item_series_4();

        // AND a valid list selection
        let mut list_selection = ItemListSelection::new(items.clone(), 4);

        // AND we've begun by selecting an index  and ensuring we're selecting items
        list_selection.set_initial_selection(2);
        // WHEN we call to move up the selection multiple times
        list_selection.move_up();
        list_selection.move_up();

        // THEN we expect multiple items/indices to be returned
        assert_eq!(true, list_selection.is_selecting());
        assert_eq!(true, list_selection.is_selected(0));
        assert_eq!(true, list_selection.is_selected(1));
        assert_eq!(true, list_selection.is_selected(2));
        let selected_items = list_selection.get_selected_items();
        assert_eq!(3, selected_items.len());
        assert_eq!(items[0], selected_items[0]);
        assert_eq!(items[1], selected_items[1]);
        assert_eq!(items[2], selected_items[2]);
    }

    #[test]
    fn test_reducing_selection_above() {
        // GIVEN a series of items to select from
        let items = build_item_series_4();

        // AND a valid list selection
        let mut list_selection = ItemListSelection::new(items.clone(), 4);

        // AND we've begun by selecting an index (the pivot point)
        // AND then selecting above that twice
        list_selection.set_initial_selection(2);
        list_selection.move_up();
        list_selection.move_up();

        assert_eq!(true, list_selection.is_selecting());
        assert_eq!(true, list_selection.is_selected(0));
        assert_eq!(true, list_selection.is_selected(1));
        assert_eq!(true, list_selection.is_selected(2));
        let selected_items = list_selection.get_selected_items();
        assert_eq!(3, selected_items.len());

        // WHEN we call to select down
        list_selection.move_down();

        // THEN we expect the selection to decrease towards the initial selection/pivot point (2)
        assert_eq!(true, list_selection.is_selecting());
        assert_eq!(true, list_selection.is_selected(1));
        assert_eq!(true, list_selection.is_selected(2));
        let selected_items = list_selection.get_selected_items();
        assert_eq!(2, selected_items.len());
        assert_eq!(items[1], selected_items[0]);
        assert_eq!(items[2], selected_items[1]);
    }

    #[test]
    fn test_selecting_downwards() {
        // GIVEN a series of items to select from
        let items = build_item_series_4();

        // AND a valid list selection
        let mut list_selection = ItemListSelection::new(items.clone(), 4);

        // AND we've begun by selecting an index  and ensuring we're selecting items
        list_selection.set_initial_selection(1);
        // WHEN we call to move down the selection
        list_selection.move_down();

        // THEN we expect multiple items/indices to be returned
        assert_eq!(true, list_selection.is_selecting());
        assert_eq!(true, list_selection.is_selected(1));
        assert_eq!(true, list_selection.is_selected(2));
        let selected_items = list_selection.get_selected_items();
        assert_eq!(2, selected_items.len());
        assert_eq!(items[1], selected_items[0]);
        assert_eq!(items[2], selected_items[1]);
    }

    #[test]
    fn test_selecting_downwards_multi() {
        // GIVEN a series of items to select from
        let items = build_item_series_4();

        // AND a valid list selection
        let mut list_selection = ItemListSelection::new(items.clone(), 4);

        // AND we've begun by selecting an index  and ensuring we're selecting items
        list_selection.set_initial_selection(1);
        // WHEN we call to move down the selection multiple times
        list_selection.move_down();
        list_selection.move_down();

        // THEN we expect multiple items/indices to be returned
        assert_eq!(true, list_selection.is_selecting());
        assert_eq!(true, list_selection.is_selected(1));
        assert_eq!(true, list_selection.is_selected(2));
        assert_eq!(true, list_selection.is_selected(3));
        let selected_items = list_selection.get_selected_items();
        assert_eq!(3, selected_items.len());
        assert_eq!(items[1], selected_items[0]);
        assert_eq!(items[2], selected_items[1]);
        assert_eq!(items[3], selected_items[2]);
    }

    #[test]
    fn test_reducing_selection_below_all_selected() {
        // GIVEN a series of items to select from
        let items = build_item_series_4();

        // AND a valid list selection
        let mut list_selection = ItemListSelection::new(items.clone(), 4);

        // AND we've begun by selecting the first item in the container
        // AND then selecting below that until we reach the bottom of the container
        list_selection.set_initial_selection(0);
        list_selection.move_down();
        list_selection.move_down();
        list_selection.move_down();

        assert_eq!(true, list_selection.is_selecting());
        assert_eq!(true, list_selection.is_selected(0));
        assert_eq!(true, list_selection.is_selected(1));
        assert_eq!(true, list_selection.is_selected(2));
        assert_eq!(true, list_selection.is_selected(3));
        let selected_items = list_selection.get_selected_items();
        assert_eq!(4, selected_items.len());

        // WHEN we call to select up
        list_selection.move_up();

        // THEN we expect the selection to decrease towards the initial selection/pivot point
        assert_eq!(true, list_selection.is_selecting());
        assert_eq!(true, list_selection.is_selected(0));
        assert_eq!(true, list_selection.is_selected(1));
        assert_eq!(true, list_selection.is_selected(2));
        let selected_items = list_selection.get_selected_items();
        assert_eq!(3, selected_items.len());
        assert_eq!(items[0], selected_items[0]);
        assert_eq!(items[1], selected_items[1]);
        assert_eq!(items[2], selected_items[2]);
    }

    #[test]
    fn test_reducing_selection_below() {
        // GIVEN a series of items to select from
        let items = build_item_series_4();

        // AND a valid list selection
        let mut list_selection = ItemListSelection::new(items.clone(), 4);

        // AND we've begun by selecting an index (the pivot point)
        // AND then selecting below that twice
        list_selection.set_initial_selection(1);
        list_selection.move_down();
        list_selection.move_down();

        assert_eq!(true, list_selection.is_selecting());
        assert_eq!(true, list_selection.is_selected(1));
        assert_eq!(true, list_selection.is_selected(2));
        assert_eq!(true, list_selection.is_selected(3));
        let selected_items = list_selection.get_selected_items();
        assert_eq!(3, selected_items.len());

        // WHEN we call to select up
        list_selection.move_up();

        // THEN we expect the selection to decrease towards the initial selection/pivot point
        assert_eq!(true, list_selection.is_selecting());
        assert_eq!(true, list_selection.is_selected(1));
        assert_eq!(true, list_selection.is_selected(2));
        let selected_items = list_selection.get_selected_items();
        assert_eq!(2, selected_items.len());
        assert_eq!(items[1], selected_items[0]);
        assert_eq!(items[2], selected_items[1]);
    }

    #[test]
    fn test_page_down_same_page() {
        // GIVEN a series of items to select from
        let items = build_item_series_4();

        // AND a valid list selection that has a line count matching our items
        let mut list_selection = ItemListSelection::new(items.clone(), 4);

        // AND we've begun by selecting an index  and ensuring we're selecting items
        list_selection.set_initial_selection(1);
        // WHEN we call to go down a page
        list_selection.page_down();

        // THEN we expect everything below this point to be selected
        assert_eq!(true, list_selection.is_selecting());
        assert_eq!(true, list_selection.is_selected(1));
        assert_eq!(true, list_selection.is_selected(2));
        assert_eq!(true, list_selection.is_selected(3));

        // AND the start index remains unchanged
        assert_eq!(0, list_selection.get_start_index());

        let selected_items = list_selection.get_selected_items();
        assert_eq!(3, selected_items.len());
        assert_eq!(items[1], selected_items[0]);
        assert_eq!(items[2], selected_items[1]);
        assert_eq!(items[3], selected_items[2]);
    }

    #[test]
    fn test_page_down_multi_page_same_page() {
        // GIVEN a series of items to select from
        let items = build_item_series_8();

        // AND a valid list selection that has a line count that fits half of these items
        let mut list_selection = ItemListSelection::new(items.clone(), 4);

        // AND we've begun by selecting an index  and ensuring we're selecting items
        list_selection.set_initial_selection(1);
        // WHEN we call to go down a page
        list_selection.page_down();

        // THEN we expect everything below this point to be selected
        // up to the end of the page
        assert_eq!(true, list_selection.is_selecting());
        assert_eq!(true, list_selection.is_selected(1));
        assert_eq!(true, list_selection.is_selected(2));
        assert_eq!(true, list_selection.is_selected(3));

        // AND the start index remains unchanged
        assert_eq!(0, list_selection.get_start_index());

        let selected_items = list_selection.get_selected_items();
        assert_eq!(3, selected_items.len());
        assert_eq!(items[1], selected_items[0]);
        assert_eq!(items[2], selected_items[1]);
        assert_eq!(items[3], selected_items[2]);
    }

    #[test]
    fn test_page_down_multi_page_next_page() {
        // GIVEN a series of items to select from
        let items = build_item_series_8();

        // AND a valid list selection that has a line count that fits half of these items
        let mut list_selection = ItemListSelection::new(items.clone(), 4);

        // AND we've begun by selecting an index
        list_selection.set_initial_selection(1);
        // AND paging down to the end of the page
        list_selection.page_down();

        // WHEN we call to page down again
        list_selection.page_down();

        // THEN we expect the selection to turn the page
        // and select the first item on the next page
        assert_eq!(true, list_selection.is_selecting());
        assert_eq!(4, list_selection.get_selected_count());
        assert_eq!(true, list_selection.is_selected(1));
        assert_eq!(true, list_selection.is_selected(2));
        assert_eq!(true, list_selection.is_selected(3));
        // 4 items per page so index 4 is the first item on the 2nd page
        assert_eq!(true, list_selection.is_selected(4));
        // AND the start index will be the next page start index
        assert_eq!(4, list_selection.get_start_index());

        // AND the items themselves will also be selected
        let selected_items = list_selection.get_selected_items();
        assert_eq!(4, selected_items.len());
        assert_eq!(items[1], selected_items[0]);
        assert_eq!(items[2], selected_items[1]);
        assert_eq!(items[3], selected_items[2]);
        assert_eq!(items[4], selected_items[3]);
    }

    #[test]
    fn test_page_up_multi_page_same_page() {
        // GIVEN a series of items to select from
        let items = build_item_series_8();

        // AND a valid list selection that has a line count that fits half of these items
        let mut list_selection = ItemListSelection::new(items.clone(), 4);

        // AND we've begun by navigating to the correct index and initialising selection
        list_selection.page_down();  // end of first page
        list_selection.page_down();  // 2nd page
        list_selection.set_initial_selection(2); // 3rd item of 2nd page
        assert_eq!(4, list_selection.get_start_index());

        // WHEN we call to go up a page
        list_selection.page_up();

        // THEN we expect everything from the top of the page to the original point to be selected
        assert_eq!(true, list_selection.is_selecting());
        // Items 5,6, and 7
        assert_eq!(true, list_selection.is_selected(4));
        assert_eq!(true, list_selection.is_selected(5));
        assert_eq!(true, list_selection.is_selected(6));

        // AND the start index remains unchanged
        assert_eq!(4, list_selection.get_start_index());

        let selected_items = list_selection.get_selected_items();
        assert_eq!(3, selected_items.len());
        assert_eq!(items[4], selected_items[0]);
        assert_eq!(items[5], selected_items[1]);
        assert_eq!(items[6], selected_items[2]);
    }

    #[test]
    fn test_page_up_multi_page_previous_page() {
        // GIVEN a series of items to select from
        let items = build_item_series_8();

        // AND a valid list selection that has a line count that fits half of these items
        let mut list_selection = ItemListSelection::new(items.clone(), 4);

        // AND we've begun by navigating to the correct index and initialising selection
        list_selection.page_down();  // end of first page
        list_selection.page_down();  // 2nd page
        list_selection.set_initial_selection(2); // 3rd item of 2nd page
        assert_eq!(4, list_selection.get_start_index());

        // WHEN we call to go up a page twice
        list_selection.page_up();
        list_selection.page_up();

        // THEN we expect the selection to turn back the page and select the last item on the previous page
        assert_eq!(true, list_selection.is_selecting());
        assert_eq!(true, list_selection.is_selected(3));
        assert_eq!(true, list_selection.is_selected(4));
        assert_eq!(true, list_selection.is_selected(5));
        assert_eq!(true, list_selection.is_selected(6));

        // AND the start index is now the start of the previous page
        assert_eq!(0, list_selection.get_start_index());

        // AND our selection is the last item of page 1 and the first 3 items of page 2
        let selected_items = list_selection.get_selected_items();
        assert_eq!(4, selected_items.len());
        assert_eq!(items[3], selected_items[0]);
        assert_eq!(items[4], selected_items[1]);
        assert_eq!(items[5], selected_items[2]);
        assert_eq!(items[6], selected_items[3]);
    }


    #[test]
    fn test_select_second_page() {
        // GIVEN a series of items to select from
        let items = build_item_series_8();

        // AND a valid list selection that has a line count that fits half of these items
        let mut list_selection = ItemListSelection::new(items.clone(), 4);

        // AND we've moved to the 2nd page of the view
        list_selection.page_down();  // end of first page
        list_selection.page_down();  // 2nd page

        // WHEN we move down and select an item
        list_selection.move_down();
        list_selection.move_down();
        list_selection.move_down();
        list_selection.toggle_select();

        // THEN we expect only that item to be selected
        assert_eq!(4, list_selection.get_start_index());
        assert_eq!(3, list_selection.get_current_index());
        assert_eq!(7, list_selection.get_true_index());
        let selected_items = list_selection.get_selected_items();
        assert_eq!(1, selected_items.len());
        assert_eq!(true, list_selection.is_selected(7));

        // AND we're in the selection mode
        assert_eq!(true, list_selection.is_selecting());
    }

    #[test]
    fn test_downward_multi_select_second_page() {
        // GIVEN a series of items to select from
        let items = build_item_series_8();

        // AND a valid list selection that has a line count that fits half of these items
        let mut list_selection = ItemListSelection::new(items.clone(), 4);

        // AND we've moved to the 2nd page of the view
        list_selection.page_down();  // end of first page
        list_selection.page_down();  // 2nd page

        // WHEN we move down and select a range of items
        list_selection.move_down();
        list_selection.toggle_select(); // Item 5
        list_selection.move_down(); // Item 6

        // THEN we expect only these items to be selected
        assert_eq!(4, list_selection.get_start_index());
        assert_eq!(2, list_selection.get_current_index());
        let selected_items = list_selection.get_selected_items();
        assert_eq!(2, selected_items.len());
        assert_eq!(true, list_selection.is_selected(5));
        assert_eq!(true, list_selection.is_selected(6));

        // AND we're in the selection mode
        assert_eq!(true, list_selection.is_selecting());
    }
}