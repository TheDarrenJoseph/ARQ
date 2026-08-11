use crate::ui::ui_layout::LayoutType;
use crate::map::position::Area;
use std::io;
use crate::engine::combat::CombatResult;
use ratatui::prelude::Widget;
use crate::character::battle::Battle;
use crate::engine::combat::CombatTurnChoiceEventType;
use crate::engine::command::command::Command;
use crate::engine::command::look_command::LookCommand;
use crate::engine::level::Level;
use crate::error::errors::ErrorWrapper;
use crate::terminal::terminal_manager::TerminalManager;
use crate::ui::bindings::look_bindings::LookKeyBindings;
use crate::ui::ui::UI;
use crate::ui::ui::UIViewMode::Map;
use crate::ui::ui_areas::{BorderedArea, UI_AREA_NAME_MAIN};
use crate::widget::stateful::combat_widget::CombatWidget;
use crate::widget::StatefulWidgetType;

pub struct CombatCommand<'a, B: 'static + ratatui::backend::Backend> {
    pub level: &'a mut Level,
    pub ui: &'a mut UI,
    pub terminal_manager : &'a mut TerminalManager<B>,
    pub battle: Battle
}

#[derive(Clone)]
pub struct CombatCallbackData {
    pub choice: CombatTurnChoiceEventType,
    pub result: Option<CombatResult>
}


impl <B: ratatui::backend::Backend> Command<()> for CombatCommand<'_, B> {
    async fn start(&mut self) -> Result<(), ErrorWrapper> {
        let ui = &mut self.ui;
        ui.show_console();
        self.terminal_manager.clear_screen().expect("Screen should have been cleared");
        //verify_display_size::<B>(self.terminal_manager);

        let frame_area = Area::from_rect(self.terminal_manager.terminal.get_frame().size());

        let mut ui_layout= ui.ui_layout.as_mut().expect("Failed to get UI Layout");
        let ui_areas =ui_layout.get_or_build_areas(frame_area.to_rect(), LayoutType::CombatView);

        let main_area = ui_areas.get_area(UI_AREA_NAME_MAIN).unwrap().area;
        let bordered_main_area = BorderedArea::from_area(main_area).unwrap();

        let combat_widget = CombatWidget {};
        ui.add_stateful_widget(StatefulWidgetType::Combat(combat_widget));

        // Input / Output loop
        while self.battle.in_progress {

            self.re_render().expect("Combat UI should have been drawn.");

            // let input_result = self.handle_input(None).unwrap();
            // if input_result.generic_input_result.done {
            //     return Ok(self.build_done_result());
            // }

            // TODO get battle action
            // callback w/ action to trigger processing
            // Show action results
        }
        // TODO
        //return Ok(self.build_done_result());
        return Ok(())
    }
}

impl <B: ratatui::backend::Backend> CombatCommand<'_, B> {
    // TODO Keep or remove?
    //    fn build_done_result(&self) -> InputResult<Battle> {
    //         InputResult { generic_input_result: GenericInputResult { done: true, requires_view_refresh: false }, view_specific_result: Some(self.battle.clone())}
    //     }
    //
    //     fn build_input_done_result(&self) -> InputResult<bool> {
    //         InputResult { generic_input_result: GenericInputResult { done: true, requires_view_refresh: false }, view_specific_result: None}
    //     }
    //
    //     fn build_input_not_done_result(&self) -> InputResult<bool> {
    //         InputResult { generic_input_result: GenericInputResult { done: false, requires_view_refresh: false }, view_specific_result: None}
    //     }

    pub fn new<'a>(ui: &'a mut UI, terminal_manager: &'a mut TerminalManager<B>, level: &'a mut Level, battle: Battle) -> CombatCommand<'a, B> {
        CombatCommand { level: level, ui, terminal_manager, battle }
    }

    fn re_render(&mut self) -> Result<(), io::Error> {
        let ui = &mut self.ui;
        let level = self.level.clone();
        self.terminal_manager.terminal.draw(|frame| {
            ui.render(Some(level), Map(), frame);
        })?;
        Ok(())
    }
}
