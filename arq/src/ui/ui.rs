use std::io;
use std::slice::IterMut;
use crate::engine::level::Level;
use crate::map::position::Area;
use crate::ui::resolution::Resolution;
use crate::ui::ui::UIViewMode::Map;
use crate::ui::ui_areas::{UI_AREA_NAME_CONSOLE, UI_AREA_NAME_MAIN};
use crate::ui::ui_layout::{LayoutType, UILayout};
use crate::view::framehandler::console::{ConsoleBuffer, ConsoleFrameHandler};
use crate::view::framehandler::{FrameData, FrameHandler};
use crate::widget::stateful::character_info_widget::CharacterInfoWidgetData;
use crate::widget::stateful::container_widget::ContainerWidgetData;
use crate::widget::{StandardWidgetType, StatefulWidgetType};
use ratatui::layout::{Alignment, Rect};
use ratatui::style::{Color, Style};
use ratatui::text::{Line, Span};
use ratatui::widgets::{Block, Borders, Paragraph, Wrap};
use termion::event::Key;
use termion::input::TermRead;
use crate::widget::stateful::container_choice_widget::ContainerChoiceWidget;

pub struct UI {
    pub render_additional: bool,
    console_visible: bool,
    stateful_widgets: Vec<StatefulWidgetType>,
    additional_widgets: Vec<StandardWidgetType>,
    frame_size : Option<Area>,
    frame_handler: ConsoleFrameHandler,
    pub ui_layout: Option<UILayout>
}

#[derive(Clone)]
pub enum StartMenuChoice {
    Play,
    Settings,
    Info,
    Quit
}

pub fn build_ui() -> UI {
    let frame_handler = ConsoleFrameHandler { buffer: ConsoleBuffer { content: String::from("") } };
    UI {
        frame_size : None,
        render_additional: false,
        console_visible: false,
        stateful_widgets: Vec::new(),
        additional_widgets: Vec::new(),
        frame_handler,
        ui_layout: None
    }
}

impl std::convert::TryFrom<usize> for StartMenuChoice {
    type Error = String;

    fn try_from(val: usize) -> Result<Self, Self::Error> {
        match val {
            0 => Ok(StartMenuChoice::Play),
            1 => Ok(StartMenuChoice::Settings),
            2 => Ok(StartMenuChoice::Info),
            3 =>  Ok(StartMenuChoice::Quit),
            _ => Err("Failed to convert to StartMenuChoice".to_string())
        }
    }
}

pub enum SettingsMenuChoice {
    FogOfWar,
    Quit
}

pub enum UIViewMode {
    Map(), // The default view mode
    Container(ContainerWidgetData),
    CharacterInfo(CharacterInfoWidgetData)
}

impl std::convert::TryFrom<usize> for SettingsMenuChoice {
    type Error = String;

    fn try_from(val: usize) -> Result<Self, Self::Error> {
        match val {
            0 => Ok(SettingsMenuChoice::FogOfWar),
            1 => Ok(SettingsMenuChoice::Quit),
            _ => Err("Failed to convert to StartMenuChoice".to_string())
        }
    }
}


pub trait Draw {
    fn draw_info(&mut self, frame : &mut ratatui::Frame<'_>);
    fn draw_console(&mut self, frame : &mut ratatui::Frame<'_>);
    fn draw_additional_widgets(&mut self, frame: &mut ratatui::Frame);
    fn draw_level_widgets(&mut self, level: &mut Level, frame: &mut ratatui::Frame);
    fn draw_container_widgets(&mut self, container: &mut ContainerWidgetData, frame: &mut ratatui::Frame);
    fn draw_character_info(&mut self, widget_data: &mut CharacterInfoWidgetData, frame: &mut ratatui::Frame);
}

fn build_main_block<'a>() -> Block<'a> {
    Block::default()
        .borders(Borders::ALL)
        //.title("||ARQ -- ASCII Roguelike Quester||")
        .style(Style::default().bg(Color::Black))
}

impl UI {

    pub fn init(&mut self, frame_area: Area) {
        let resolution = Resolution::new(frame_area.width, frame_area.height);
        self.frame_size = Some(frame_area);
        if self.ui_layout.is_none() {
          self.ui_layout = Some(UILayout::new(resolution));
        }
    }

    pub fn re_init(&mut self, frame_area: Area) {
        let resolution = Resolution::new(frame_area.width, frame_area.height);
        self.frame_size = Some(frame_area);
        let mut ui_layout = UILayout::new(resolution);
        ui_layout.rebuild_areas(frame_area.to_rect());
        self.ui_layout = Some(ui_layout);
    }

    /*
        Draws the "base" UI that's visible by default:
        1. The "main" window / main block (This does not put content inside this block, it's just an empty bordered window)
        2. The console window at the bottom of the screen
        3. Stat bars and command usage hints (additional widgets)
     */
    pub fn render<'a>(
        &mut self,
        level: Option<Level>,
        view_mode: UIViewMode,
        frame: &mut ratatui::Frame<'_>
    ) {
        let ui_layout =  self.ui_layout.as_mut().ok_or("Failed to get ui_layout, has it been initialised?").unwrap();
        let areas = ui_layout.get_or_build_areas(frame.size(), LayoutType::StandardSplit);
        if let Some(main) = areas.get_area(UI_AREA_NAME_MAIN) {
            let main_area = main.area;
            let main_block = build_main_block();
            {
                frame.render_widget(main_block, main_area.to_rect());
            }
        }

        // Render any major stateful widgets
        if let Some(mut level) = level {
            self.draw_level_widgets(&mut level, frame);
        }
        match view_mode {
            UIViewMode::Map() => {
                // TODO Map widget support
            },
            UIViewMode::Container(mut widget_data) => {
                self.draw_container_widgets(&mut widget_data, frame);
            },
            UIViewMode::CharacterInfo(mut widget_data) => {
                self.draw_character_info(&mut widget_data, frame);
            }
        }
        
        if self.console_visible {
            self.draw_console(frame);
        }

        if self.render_additional {
            self.draw_additional_widgets(frame);
        }
    }

    pub fn show_console(&mut self) {
        self.console_visible = true;
    }

    pub fn hide_console(&mut self) {
        self.console_visible = false;
    }

    /*
        Updates the buffer that is written to console on each rendering of the UI
     */
    pub fn set_console_buffer(&mut self, input: String) {
        self.frame_handler.buffer.content = input;
    }

    pub fn clear_console_buffer(&mut self) {
        self.frame_handler.buffer.content = String::new();
    }

    pub fn get_additional_widgets(&self) -> &Vec<StandardWidgetType> {
        &self.additional_widgets
    }

    pub fn get_additional_widgets_mut(&mut self) -> &mut Vec<StandardWidgetType> {
        &mut self.additional_widgets
    }

    pub fn get_stateful_widgets_mut(&mut self) -> &mut Vec<StatefulWidgetType> {
        &mut self.stateful_widgets
    }
}

pub fn get_input_key() -> Result<Key, io::Error> {
    Ok(io::stdin().keys().next().unwrap()?)
}

impl Draw for UI {

    fn draw_info(&mut self, frame: &mut ratatui::Frame<'_>) {
        self.render(None, Map(), frame);

        let dev_spans = vec!(Span::raw("Made by Darren Joseph. Written in Rust."));
        let spans = vec![Line::from(dev_spans),
                         Line::from(Span::raw("--- Credits ---")),
                         Line::from(Span::raw("Background music:")),
                         Line::from(Span::raw("Tavern Loop One by Alexander Nakarada | https://www.serpentsoundstudios.com")),
                         Line::from(Span::raw("Music promoted by https://www.free-stock-music.com")),
                         Line::from(Span::raw("Attribution 4.0 International (CC BY 4.0)")),
                         Line::from(Span::raw("https://creativecommons.org/licenses/by/4.0/")),
                         Line::from(Span::raw("")),
                         Line::from(Span::raw("Celtic Ambiance by Alexander Nakarada (www.creatorchords.com)")),
                         Line::from(Span::raw("Licensed under Creative Commons BY Attribution 4.0 License")),
                         Line::from(Span::raw("https://creativecommons.org/licenses/by/4.0/")),
        ];

        let spans_len = spans.len() as u16;
        let paragraph = Paragraph::new(spans)
            .block(Block::default().borders(Borders::NONE))
            .style(Style::default().fg(Color::White).bg(Color::Black))
            .alignment(Alignment::Left)
            .wrap(Wrap { trim: true });

        let frame_size = frame.size();
        let paragraph_size = Rect::new(4, 4, frame_size.width - 4, spans_len + 4);
        frame.render_widget(paragraph, paragraph_size);
    }

    fn draw_console(&mut self, frame: &mut ratatui::Frame) {
        let ui_areas = self.ui_layout.as_ref().unwrap().get_ui_areas(LayoutType::StandardSplit);
        let console_area = ui_areas.get_area(UI_AREA_NAME_CONSOLE).unwrap().area;
        let frame_data = FrameData { frame_area: console_area, ui_areas: ui_areas.clone(), data: ConsoleBuffer { content: self.frame_handler.buffer.content.clone() } };
        self.frame_handler.handle_frame(frame, frame_data);
    }

    fn draw_additional_widgets(&mut self, frame: &mut ratatui::Frame) {
        let widget_count = self.additional_widgets.len();
        if let Some(main_area) = self.ui_layout.as_ref().unwrap().get_ui_areas(LayoutType::StandardSplit).get_area(UI_AREA_NAME_MAIN) {
            let area = main_area.area;
            let rect = area.to_rect();
            let max_width = area.width / 2;
            if widget_count > 0 {
                let mut _offset = 0;
                for widget in self.additional_widgets.iter_mut() {
                    match widget {
                        StandardWidgetType::StatLine(w) => {
                            frame.render_widget(w.clone(),
                                                Rect::new(rect.x + 1, rect.y, max_width, 1));
                        },
                        StandardWidgetType::UsageLine(w) => {
                            frame.render_widget(w.clone(),
                                                Rect::new(rect.x + 1, rect.height - 1, rect.width, 1));
                        }
                    }
                    _offset += 1;
                }
            }
        }
    }

    fn draw_level_widgets(&mut self, level: &mut Level, frame: &mut ratatui::Frame) {
        // TODO do we need to consider UI Layouts here?
        let widget_count = self.stateful_widgets.len();
        if widget_count > 0 {
            let mut _offset = 0;
            for widget in self.stateful_widgets.iter_mut() {
                match widget {
                    StatefulWidgetType::Map(map_widget) => {
                        frame.render_stateful_widget(map_widget.clone(), frame.area(), level);
                    }
                    _ => {}
                }
                _offset += 1;
            }
        }
    }

    fn draw_container_widgets(&mut self, widget_data: &mut ContainerWidgetData, frame: &mut ratatui::Frame) {
        let widget_data_container_id = widget_data.container.get_self_item().get_id();
        let widget_count = self.stateful_widgets.len();
        if widget_count > 0 {
            let ui_layout = self.ui_layout.as_mut().unwrap();
            let ui_areas = ui_layout.get_or_build_areas(frame.size(), LayoutType::StandardSplit);
            for widget in self.stateful_widgets.iter_mut() {
                match widget {
                    StatefulWidgetType::Container(container_widget) => {
                        // If the data belongs to this widget, render it
                        if container_widget.container_id == widget_data_container_id {
                            frame.render_stateful_widget(container_widget.clone(), frame.size(), widget_data);
                            // Stop here so we don't waste time drawing more widgets that will be hidden
                            return;
                        }
                    }
                    _ => {}
                }
            }
        }
    }

    fn draw_character_info(&mut self, widget_data: &mut CharacterInfoWidgetData, frame: &mut ratatui::Frame) {
        let widget_count = self.stateful_widgets.len();
        if widget_count > 0 {
            let ui_layout = self.ui_layout.as_mut().unwrap();
            let ui_areas = ui_layout.get_or_build_areas(frame.size(), LayoutType::StandardSplit);
            for widget in self.stateful_widgets.iter_mut() {
                match widget {
                    StatefulWidgetType::CharacterInfo(charcter_info_widget) => {
                        frame.render_stateful_widget(charcter_info_widget.clone(), frame.size(), widget_data);
                    }
                    _ => {
                        continue;
                    }
                }
            }

            for widget in self.stateful_widgets.iter_mut() {
                match widget {
                    StatefulWidgetType::ContainerChoice(container_choice_widget) => {
                        if let Some(ccd) = &mut widget_data.containers_data.container_choice_data {
                            frame.render_stateful_widget(container_choice_widget.clone(), frame.size(), ccd);
                        }
                    }
                    _ => {
                        continue;
                    }
                }
            }
        }
    }
}


