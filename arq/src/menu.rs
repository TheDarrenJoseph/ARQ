use crate::global_flags::ENTER_KEY;
use ratatui::style::{Color, Style};
use ratatui::widgets::{Block, Borders, List, ListItem};
use termion::event::Key;

pub struct Menu {
    pub menu_titles: Vec<String>,
    pub highlight_text: Option<String>,
    pub selection: usize,
    pub selected: bool,
    pub exit: bool
}

pub trait ToList {
    fn to_list(&self) -> List;
}

pub trait Selection {
    fn select_up(&mut self);
    fn select_down(&mut self);
    fn handle_input(&mut self, key: termion::event::Key);
}

impl ToList for Menu {
    fn to_list(&self) -> List {
        let menu_items: Vec<ListItem> = self.menu_titles.iter().cloned().map(ListItem::new).collect();
        let mut list = List::new(menu_items)
            .block(Block::default()
                .borders(Borders::NONE)
            )
            .style(Style::default().fg(Color::White))
            .highlight_style(Style::default().fg(Color::Red));

        if self.highlight_text != None {
            list = list.highlight_symbol(self.highlight_text.as_ref().unwrap());
        }
        list
    }
}

impl Selection for Menu {
    fn select_up(&mut self) {
        if self.selection > 0 {
            self.selection -= 1;
        }
    }

    fn select_down(&mut self) {
        if self.selection < self.menu_titles.len() - 1 {
            self.selection += 1;
        }
    }

    fn handle_input(&mut self, key: termion::event::Key) {
        self.selected = false;
        match key {
            //Key::Esc => {
            //    self.exit = true;
            //}
            ENTER_KEY => {
                self.selected = true;
            }
            Key::Char(c) => {
                log::info!("Inputted: '{}'", c);
            }
            Key::Up => {
                self.select_up();
            }
            Key::Down => {
                self.select_down();
            }
            Key::Right => {
                self.selected = true;
            }
            _ => {
                log::info!("{}", format!("Unknown input: {:?}", key));
            }
        }
    }
}

pub fn build_start_menu(game_started: bool) -> Menu {

    let play_or_resume = if game_started { "Resume".to_owned() } else { "Play".to_owned() };
    let titles = vec![play_or_resume, "Settings".to_owned(), "Info".to_owned(), "Quit".to_owned()];
    let prompt = Some("-> ".to_owned());
    let menu = Menu { menu_titles : titles,  highlight_text : prompt, selection : 0, selected: false, exit: false};
    menu
}

#[cfg(test)]
mod tests {
    use crate::menu::{Menu, Selection};

    fn build_test_menu() ->  Menu {
        let titles = vec!["A".to_owned(), "B".to_owned(),  "C".to_owned()];
        Menu { menu_titles : titles,  highlight_text : None, selection : 0, selected: false, exit: false}
    }

    #[test]
    fn test_menu_select_up() {
        // GIVEN a menu of 3 choices
        let mut menu = build_test_menu();
        // AND the initial selection is index 1
        menu.selection = 1;

        // WHEN we call to select_up
        menu.select_up();

        // THEN we expect the selection to be at the lowest index
        assert_eq!(0, menu.selection);
    }

    #[test]
    fn test_menu_select_up_upper_bound() {
        // GIVEN a menu of 3 choices
        let mut menu = build_test_menu();
        // AND the initial selection is index 0
        assert_eq!(0, menu.selection);

        // WHEN we call to select_up
        menu.select_up();

        // THEN we expect the selection to remain unchanged
        assert_eq!(0, menu.selection);
    }

    #[test]
    fn test_menu_select_down() {
        // GIVEN a menu of 3 choices
        let mut menu = build_test_menu();
        // AND the initial selection is index 0
        assert_eq!(0, menu.selection);

        // WHEN we call to select_down
        menu.select_down();

        // THEN we expect the selection to increment by 1
        assert_eq!(1, menu.selection);
    }

    #[test]
    fn test_menu_select_down_lower_bound() {
        // GIVEN a menu of 3 choices
        let mut menu = build_test_menu();
        // AND the initial selection is index 0
        assert_eq!(0, menu.selection);

        // WHEN we call to select_down
        for _ in 0..3 {
            menu.select_down();
        }

        // THEN we expect the selection to be at the lowest possible index of 2
        assert_eq!(2, menu.selection);
    }
}
