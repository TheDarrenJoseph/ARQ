use crate::engine::command::util::CurrentContainersData;
use crate::engine::event::container::{OpenedContainerEventData, OpenedContainerEventType, SourceContainerScope, TargetContainerScope};
use crate::engine::event::ui::AppEventType::OpenedContainerEvent;
use crate::engine::event::ui::UIEvent;
use crate::item_list_selection::{ItemListSelection, ListSelection};
use crate::map::objects::container::Container;
use crate::map::position::Position;
use crate::ui::ui_areas::{UIAreas, UI_AREA_NAME_MAIN};
use crate::widget::stateful::container_widget::ContainerWidget;
use ratatui::buffer::Buffer;
use ratatui::layout::Rect;
use ratatui::prelude::{Line, Modifier, StatefulWidget, Style};
use ratatui::symbols::line::VERTICAL;
use ratatui::widgets::{Block, Borders, Tabs, Widget};
use tokio::sync::mpsc;
use tokio::sync::mpsc::UnboundedSender;
use uuid::Uuid;

#[derive(PartialEq, Clone, Debug)]
pub enum TabChoice {
    INVENTORY,
    EQUIPMENT,
    CHARACTER
}

#[derive(Clone)]
pub struct Tab {
    tab_choice: TabChoice,
    title: String
}

impl Tab {
    // Returns the 1st tab
    pub fn first() -> Tab {
        Tab { tab_choice: TabChoice::INVENTORY, title: String::from("Inventory") }
    }

    // Returns all possible tabs in order
    pub fn values() -> Vec<Tab> {
        let inventory_tab = Tab { tab_choice: TabChoice::INVENTORY, title: String::from("Inventory") };
        let equipment_tab = Tab { tab_choice: TabChoice::EQUIPMENT, title: String::from("Equipment") };
        let character_tab = Tab { tab_choice: TabChoice::CHARACTER, title: String::from("Character") };
        vec![inventory_tab, equipment_tab, character_tab]
    }
}


#[derive(Debug, Clone)]
pub struct CharacterInfoWidget {
    pub container_widget : ContainerWidget
}

impl CharacterInfoWidget {
    pub fn new(
        inventory_container_id: Uuid
    ) -> CharacterInfoWidget {
        let container_widget = ContainerWidget::new(inventory_container_id);
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
    pub event_sender: mpsc::UnboundedSender<UIEvent>,
    pub current_containers_data: CurrentContainersData // Tracks the currently open containers / relevant widget data
}

impl CharacterInfoWidgetData {
    pub fn new(
        inventory_container: Container, 
        ui_areas: UIAreas,
        container_event_sender: UnboundedSender<UIEvent>
    ) -> CharacterInfoWidgetData {
        let items = inventory_container.to_cloned_item_list();
        
        // Total area height - 4 for:
        // title, heading, and stat line
        // Plus the tabs for each section of the character info view
        let main_area = ui_areas.get_area(UI_AREA_NAME_MAIN).unwrap();
        let line_count = main_area.area.height - 4;
        let _item_list_selection =  ItemListSelection::new(items.clone(), line_count.into());
        
        CharacterInfoWidgetData {
            tab_choice: TabChoice::INVENTORY,
            container: inventory_container.clone(),
            ui_areas: ui_areas.clone(),
            event_sender: container_event_sender.clone(),
            current_containers_data: CurrentContainersData::new()
        }
    }

    pub async fn handle_event(&mut self, event: UIEvent) {
        log::debug!("[container_info_widget] Handling event: {:?}", event.name());

        match event {
            // Moving items between different containers
            UIEvent::AppEvent(
                OpenedContainerEvent(
                    OpenedContainerEventType::MoveToContainerChoiceResult,
                    Some(OpenedContainerEventData::MoveItemsToContainerChoiceResult(ref response))
                )
            ) => {
                // If the target is in the current containers data (in Inventory) then make sure to handle it here first
                match &response.request.target {
                    TargetContainerScope::PlayerInventory(pic_target) => {
                        let target_container_id = pic_target.container.get_self_item().get_id();
                        if let Some(target_data) =self.current_containers_data.get_data_mut(target_container_id) {
                            let target_event = event.clone();
                            target_data.handle_event(target_event).await;
                        }
                    },
                    _ => {}
                }

                match &response.request.source {
                    SourceContainerScope::PlayerInventory(pic_source) => {
                        let target_container_id = pic_source.container.get_self_item().get_id();
                        if let Some(target_data) =self.current_containers_data.get_data_mut(target_container_id) {
                            let target_event = event.clone();
                            target_data.handle_event(target_event).await;
                        }
                    },
                    _ => {}
                }

                // We're done if we've handled both target and source
                return;

            },
            _ => {}
        }

        if let Some(current_container_data) =  self.current_containers_data.get_current_data_mut() {
            current_container_data.handle_event(event).await;
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

        if let Some(current_container_data) =  widget_data.current_containers_data.get_current_data_mut() {
            self.container_widget.render(
                area, buf, current_container_data
            );
        }
    }
}