use std::sync::mpsc::Receiver;
use crate::ui::ui_areas::UI_AREA_NAME_MAIN;
use crate::map::position::Area;
use crate::ui::ui_layout::LayoutType::SingleMainWindowCentered;
use crate::ui::ui_areas_builder::UIAreasBuilder;
use std::convert::TryInto;
use ratatui::widgets::{Block, Gauge, Widget};
use crate::progress::{MultiStepProgress, Step};
use ratatui::buffer::Buffer;
use ratatui::layout::Rect;
use ratatui::prelude::{Color, Style};
use crate::ui::ui::UIViewMode::LoadingScreen;
// TODO Convert these
// ProgressDisplay

#[derive(Clone)]
#[derive(Debug)]
#[derive(Eq)]
#[derive(PartialEq)]
pub struct LoadingScreenWidget {
    pub progress: MultiStepProgress
}

impl LoadingScreenWidget {
    fn handle_progress() {
        // self.terminal_manager.terminal.draw(|frame| {
        //     //  let current_step_number = progress.get_current_step_number();
        //     //         if let Some(step_number) = current_step_number {
        //     //             let step_count = progress.step_count();
        //     //             log::info!("Showing progress: {}/{}", step_number, step_count);
        //     //             let fh = &mut self.frame_handler;
        //     //             self.terminal_manager.terminal.draw(|frame| {
        //     //                 let ui_areas= UIAreasBuilder::new(Area::from_rect(frame.size()))
        //     //                     .layout_type(SingleMainWindowCentered)
        //     //                     .build().1;
        //     //
        //     //                 let main_area = ui_areas.get_area(UI_AREA_NAME_MAIN).unwrap();
        //     //                 fh.handle_frame(frame, FrameData { data: progress.clone(), ui_areas: ui_areas.clone(), frame_area: main_area.area })
        //     //             }).expect("The progress display should have been drawn.");
        //     //         }
        //     ui.render(
        //         None,
        //         LoadingScreen(),
        //         frame
        //     );
        // });
    }

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