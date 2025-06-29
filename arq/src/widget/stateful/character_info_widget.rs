use ratatui::buffer::Buffer;
use ratatui::layout::Rect;
use ratatui::prelude::{Line, Modifier, StatefulWidget, Style};
use ratatui::symbols::line::VERTICAL;
use ratatui::widgets::{Block, Borders, Tabs, Widget};
use tokio::sync::mpsc;
use tokio::sync::mpsc::UnboundedSender;
use crate::item_list_selection::ItemListSelection;
use crate::map::objects::container::Container;
use crate::map::position::{Area, Position};
use crate::ui::event::Event;
use crate::ui::ui_areas::{UIAreas, UI_AREA_NAME_MAIN};
use crate::ui::ui_layout::LayoutType;
use crate::view::character_info_view::Tab;
use crate::widget::stateful::container_widget::{ContainerWidget, ContainerWidgetData};

#[derive(PartialEq, Clone, Debug)]
pub enum TabChoice {
    INVENTORY,
    EQUIPMENT,
    CHARACTER
}

#[derive(Debug)]
pub struct CharacterInfoWidget {
    pub container_widget : ContainerWidget
}

impl CharacterInfoWidget {
    pub fn new(
        container_widget : ContainerWidget
    ) -> CharacterInfoWidget {
        CharacterInfoWidget {
            container_widget
        }
    }
}

#[derive(Debug, Clone)]
pub struct CharacterInfoWidgetData {
    pub tab_choice: TabChoice,
    pub container : Container,
    pub ui_areas: UIAreas,
    pub item_list_selection : ItemListSelection,
    pub event_sender: mpsc::UnboundedSender<Event>,
    pub container_widget_data: ContainerWidgetData,
}

impl CharacterInfoWidgetData {
    pub fn new(
        inventory_container: Container, 
        ui_areas: UIAreas,
        container_widget_data: ContainerWidgetData,
        container_event_sender: UnboundedSender<Event>
    ) -> CharacterInfoWidgetData {
        let items = inventory_container.to_cloned_item_list();
        // Total area height - 3 for title, heading, and stat line
        let main_area = ui_areas.get_area(UI_AREA_NAME_MAIN).unwrap();
        let line_count = main_area.area.height - 3;
        let item_list_selection =  ItemListSelection::new(items.clone(), line_count.into());
        
        CharacterInfoWidgetData {
            tab_choice: TabChoice::INVENTORY,
            container: inventory_container.clone(),
            ui_areas: ui_areas.clone(),
            item_list_selection,
            event_sender: container_event_sender.clone(),
            container_widget_data: container_widget_data,
        }
    }
}

impl StatefulWidget for CharacterInfoWidget {
    type State = CharacterInfoWidgetData;

    fn render(self, area: Rect, buf: &mut Buffer, state: &mut Self::State) {
        let main_area = state.ui_areas.get_area(UI_AREA_NAME_MAIN).unwrap();
        let frame_size = main_area.area.to_rect();
        
        let widget_data = state;
        
        let tabs = Tab::values();
        let titles: Vec<_> = tabs.iter().map(|t| t.title.clone()).map(Line::from).collect();
        let selection_index = widget_data.tab_choice.clone() as i32;
        let tabs = Tabs::new(titles)
            .block(Block::default().title("Character Info").borders(Borders::NONE))
            .style(Style::default())
            .highlight_style(Style::default().add_modifier(Modifier::REVERSED))
            .divider(VERTICAL)
            .select(selection_index as usize);

        let heading_pos = Position::new(frame_size.x + 1, frame_size.y);
        let heading_area = Rect::new(
            heading_pos.x,
            heading_pos.y,
            frame_size.width - 2,
            3
        );

        tabs.render(heading_area, buf);
        
        let mut container_widget_data = &mut widget_data.container_widget_data.clone();
        self.container_widget.render(
            area, buf, &mut container_widget_data
        );
        
        todo!()
    }
}