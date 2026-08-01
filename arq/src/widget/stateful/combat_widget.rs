use ratatui::prelude::StatefulWidget;
use crate::character::battle::Battle;
use crate::engine::level::Level;
use crate::map::map_view_areas::MapViewAreas;
use crate::widget::stateful::map_widget::MapWidget;

#[derive(Clone)]
#[derive(Debug)]
pub struct CombatWidget {
    pub battle: Battle
}


// impl StatefulWidget for CombatWidget {
//     type State = Battle;
// }