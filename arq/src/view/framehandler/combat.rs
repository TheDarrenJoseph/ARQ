use ratatui::layout::{Alignment, Rect};
use ratatui::style::{Modifier, Style};
use ratatui::text::{Line, Span, Text};
use ratatui::widgets::{Block, Borders, Paragraph};
use ratatui::Frame;

use crate::character::battle::Battle;
use crate::character::equipment::{Equipment, EquipmentSlot, WeaponSlot};
use crate::engine::combat::CombatTurnChoiceEventType;
use crate::engine::level::Level;
use crate::map::map_view_areas::MapViewAreas;
use crate::map::position::{build_rectangular_area, Area, Position};
use crate::option_list_selection::{MappedOption, OptionListSelection};
use crate::ui::ui_areas::{BorderedArea, UIAreas, UI_AREA_NAME_CONSOLE, UI_AREA_NAME_MAIN, UI_AREA_NAME_MINIMAP};
use crate::view::framehandler::{FrameData, FrameHandler};
use crate::widget::stateful::map_widget::MapWidget;

pub struct CombatFrameHandler {
    pub selection: OptionListSelection<CombatTurnChoiceEventType>,
    pub level: Level
}



impl CombatFrameHandler {
    pub fn new(level: Level) -> CombatFrameHandler {
        CombatFrameHandler { selection: OptionListSelection::new(), level }
    }


}


impl FrameHandler<Battle> for CombatFrameHandler {
    fn handle_frame(&mut self, frame: &mut Frame, data: FrameData<Battle>) {

    }
}