use crate::map::position::Area;
use crate::progress::{MultiStepProgress, Step};
use crate::ui::ui_areas::UI_AREA_NAME_MAIN;
use crate::ui::ui_areas_builder::UIAreasBuilder;
use crate::ui::ui_layout::LayoutType::SingleMainWindowCentered;
use ratatui::buffer::Buffer;
use ratatui::layout::Rect;
use ratatui::prelude::{Color, Style};
use ratatui::widgets::{Block, Gauge, Widget};
use std::convert::TryInto;

#[derive(Clone)]
#[derive(Debug)]
#[derive(Eq)]
#[derive(PartialEq)]
pub struct LoadingScreenWidget {
    pub progress: MultiStepProgress
}

impl LoadingScreenWidget {

    fn build_gauge(&self) -> Gauge<'static> {
        let progress = &self.progress;
        let step_number =  progress.get_current_step_number();
        let current_step: &Step = progress.get_current_step_value().unwrap();
        let step_name = current_step.description.clone();
        let step_count = progress.step_count();
        let progress_percentage = progress.get_progress_percentage();
        let label = format!("{}/{} : {}", step_number, step_count, step_name);

        return Gauge::default()
            .block(Block::default()
                .title("Map Generation"))
            .label(label)
            .style(Style::default().fg(Color::White).bg(Color::Black))
            .gauge_style(Style::default().fg(Color::White).bg(Color::Black))
            // No support for usize? Not ideal.
            .percent(progress_percentage.try_into().unwrap())
    }
}

impl Widget for LoadingScreenWidget {
    fn render(self, area: Rect, buf: &mut Buffer) {
        let ui_areas= UIAreasBuilder::new(Area::from_rect(area))
         .layout_type(SingleMainWindowCentered)
         .build().1;

        let main_area = ui_areas.get_area(UI_AREA_NAME_MAIN).unwrap();
        let gauge = self.build_gauge();
        gauge.render(main_area.area.to_rect(), buf);
    }
}