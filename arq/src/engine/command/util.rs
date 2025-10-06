use crate::widget::stateful::container_widget::ContainerWidgetData;
use std::collections::HashMap;
use uuid::Uuid;

// Tracks the currently open containers / relevant widget data
#[derive(Clone, Debug)]
pub struct CurrentContainersData {
    pub(crate) current_container_id: Option<Uuid>,
    pub(crate) container_ids : Vec<Uuid>,
    pub(crate) widget_data_by_id : HashMap<Uuid, ContainerWidgetData>
}

impl CurrentContainersData {
    pub fn new() -> CurrentContainersData {
        CurrentContainersData {
            current_container_id: None,
            container_ids : vec![],
            widget_data_by_id : HashMap::new()
        }
    }

    pub fn add_container_data(&mut self, container_id: Uuid, widget_data: ContainerWidgetData) {
        self.current_container_id = Some(container_id);
        self.container_ids.push(container_id);
        self.widget_data_by_id.insert(container_id, widget_data);
    }
    
    pub fn get_current_data_mut(&mut self) -> Option<&mut ContainerWidgetData> {
        if let Some(current_container_id) = self.current_container_id {
            self.widget_data_by_id.get_mut(&current_container_id)
        } else {
            None
        }
    }
}