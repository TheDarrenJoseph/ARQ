use std::io::Error;

use log::info;
use termion::event::Key;
use ratatui::buffer::Cell;
use ratatui::CompletedFrame;

use crate::engine::level::Level;
use crate::error::errors::ErrorWrapper;
use crate::map::map_view_areas::MapViewAreas;
use crate::map::position::Area;
use crate::terminal::terminal_manager::TerminalManager;
use crate::ui::ui::UI;
use crate::ui::ui::UIViewMode::Map;
use crate::view::util::cell_builder::CellBuilder;
use crate::view::{verify_display_size, GenericInputResult, InputHandler, InputResult, View};
use crate::widget::stateful::map_widget::MapWidget;
use crate::widget::StatefulWidgetType;
/*
    This view draws the following to the screen:
    1. Individual tiles of the map
    2. Characters
    3. Containers

    There are 3 map areas to consider:
    1. Map Area (Map co-ords): the position/size of the actual map e.g tiles, this should currently always start at 0,0
    2. Map view area (View co-ords): The position/size of the map view relative to the entire terminal frame, this could start at 1,1 for example (accounting for borders)
    3. Map display area (Map co-ords) The position/size of the map 'viewfinder', this is essentially the portion of the map area that's visible within the map view area
      3.1 The map display area is what will move with the character, and is what allows the map view to pan around
 */
pub struct MapView<'a, B : ratatui::backend::Backend> {
    pub ui : &'a mut UI,
    pub level : &'a mut Level,
    pub terminal_manager : &'a mut TerminalManager<B>,
    pub map_view_areas: MapViewAreas
}

impl<B : ratatui::backend::Backend> MapView<'_, B> {

    fn blank_map_view(&mut self) -> Result<(), Error> {
        let view_area = self.map_view_areas.map_display_area;
        let blanking_cell: Cell = CellBuilder::for_blank();
        // Clear everything in the view area (entire internal window area)
        for view_area_x in view_area.start_position.x..view_area.end_position.x {
            for view_area_y in view_area.start_position.y..view_area.end_position.y {
                let cell_tup: (u16, u16, &Cell) = (view_area_x, view_area_y, &blanking_cell);
                let updates: Vec<(u16, u16, &Cell)> = vec![cell_tup];
                self.terminal_manager.terminal.backend_mut().draw(updates.into_iter())?;
            }
        }
        self.terminal_manager.terminal.backend_mut().flush().expect("The terminal should have been flushed.");
        info!("Map view cleared");
        Ok(())
    }
    
    fn build_widget(&mut self) -> MapWidget {
        let map_view_areas = self.map_view_areas;
        let map_widget: MapWidget = MapWidget::new(map_view_areas);
        return map_widget;
    }
}

impl<B : ratatui::backend::Backend> View<bool> for MapView<'_, B> {

    fn begin(&mut self) -> Result<InputResult<bool>, ErrorWrapper> {
        let _level = self.level.clone();
        let _map_view_areas = self.map_view_areas.clone();
        
        let map_widget = self.build_widget();
        let stateful_widgets = self.ui.get_stateful_widgets_mut();
        stateful_widgets.push(StatefulWidgetType::Map(map_widget));
        
        self.draw(None)?;
        return Ok(InputResult { generic_input_result: GenericInputResult { done: true, requires_view_refresh: true }, view_specific_result: None});
    }

    // There are 3 map areas to consider:
    // 1. Map Area - Map co-ords (the position/size of the actual map e.g tiles), this should currently always start at 0,0
    // 2. Map view area - View co-ords (The position/size of the map view relative to the entire terminal frame), this could start at 1,1 for example (accounting for borders)
    // 3. Map display area - Map co-ords (The position/size of the map 'viewfinder', the area that you can actually see the map through)
    // 3.1 The map display area is what will move with the character throughout larger maps
    fn draw(&mut self, _area: Option<Area>) -> Result<CompletedFrame, ErrorWrapper> {
        let map_display_area = self.map_view_areas.map_display_area;
        let frame_size = map_display_area.to_rect();
        
        let ui = &mut self.ui;

        // Frame handler data
        verify_display_size::<B>(self.terminal_manager);

        let level = &mut self.level;
        let terminal = &mut self.terminal_manager.terminal;
        let map_view_areas = self.map_view_areas.clone();
        
        return Ok(terminal.draw(|frame| {
            // First let the UI draw everything else
            ui.render(None, Map(), frame);

            // Then render the map widget
            let map_widget = ui.get_stateful_widgets_mut().iter_mut().find(|w| match w {
                StatefulWidgetType::Map(_) => true,
                _ => false
            });
            if let Some(widget_type) = map_widget {
                match widget_type {
                    StatefulWidgetType::Map(map_widget) => {
                        // Update the map widget with the latest state
                        map_widget.map_view_areas = map_view_areas;
                        frame.render_stateful_widget(map_widget.clone(), frame_size, level);
                    }
                    _ => {}
                }
            }
        })?);
    }


}

impl <COM: ratatui::backend::Backend> InputHandler<bool> for MapView<'_, COM> {
    fn handle_input(&mut self, _input: Option<Key>) -> Result<InputResult<bool>, ErrorWrapper> {
        return Ok(InputResult { generic_input_result: GenericInputResult { done: true, requires_view_refresh: false }, view_specific_result: None});
    }
}