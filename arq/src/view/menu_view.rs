use std::convert::TryInto;
use std::io;

use crate::error::errors::ErrorWrapper;
use log::{debug, info};
use termion::input::TermRead;
use ratatui::layout::Rect;
use ratatui::CompletedFrame;
use ratatui::widgets::ListState;

use crate::map::position::Area;
use crate::menu::{Menu, Selection, ToList};
use crate::terminal::terminal_manager::TerminalManager;
use crate::ui::ui::{StartMenuChoice, UI};
use crate::ui::ui_areas::UI_AREA_NAME_MAIN;
use crate::ui::ui_layout::LayoutType;
use crate::view::{verify_display_size, GenericInputResult, InputResult, View};

pub struct MenuView<'a, B : ratatui::backend::Backend> {
    pub ui : &'a mut UI,
    pub terminal_manager : &'a mut TerminalManager<B>,
    pub(crate) menu: Menu
}

impl<B : ratatui::backend::Backend> MenuView<'_, B> {
    fn handle_start_menu_selection(&mut self) -> Result<StartMenuChoice, ErrorWrapper> {
        loop {
            let start_menu_mut = &mut self.menu;

            let last_selection = start_menu_mut.selection;
            let key = io::stdin().keys().next().unwrap().unwrap();
            start_menu_mut.handle_input(key);
            let selection = start_menu_mut.selection;
            //debug!("Selected menu item: {}", selection);

            if start_menu_mut.selected {
                match start_menu_mut.selection.try_into() {
                    Ok(x) => {
                        return Ok(x);
                    },
                    Err(_) => {}
                }
            }

            if last_selection != selection {
                //debug!("Selection changed to: {}", selection);
                self.draw(None)?;
            }
        }
    }
}

impl<B : ratatui::backend::Backend> View<StartMenuChoice> for MenuView<'_, B> {
    fn begin(&mut self) -> Result<InputResult<StartMenuChoice>, ErrorWrapper> {
        self.draw(None)?;
        let mut choice : Option<StartMenuChoice> = None;
        loop {
            if choice.is_some() {
                return Ok(InputResult {
                    generic_input_result: GenericInputResult { done: false, requires_view_refresh: true },
                    view_specific_result: Some(choice.clone().unwrap())
                });
            } else {
                choice = Some(self.handle_start_menu_selection()?);
            }
        }
    }

    fn draw(&mut self, _area: Option<Area>) -> Result<CompletedFrame, ErrorWrapper> {
        let ui = &mut self.ui;
        verify_display_size::<B>(&mut self.terminal_manager);

        // TODO hookup a full-screen area for this
        let ui_areas = ui.ui_layout.as_mut().unwrap().get_ui_areas(LayoutType::SingleMainWindow);
        let main_area_result = ui_areas.get_area(UI_AREA_NAME_MAIN).unwrap();

        let menu = &self.menu;
        let menu_selection = self.menu.selection;
        Ok(self.terminal_manager.terminal.draw(move |frame| {
            let mut menu_list_state = ListState::default();
            menu_list_state.select(Some(menu_selection.try_into().unwrap()));
            let area = main_area_result.area;
            let menu_size = Rect::new(4, 4, area.width / 2, menu.menu_titles.len().try_into().unwrap());
            let menu_list = menu.to_list();
            frame.render_stateful_widget(menu_list, menu_size, &mut menu_list_state);
        })?)
    }
}