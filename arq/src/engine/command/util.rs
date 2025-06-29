use std::collections::HashMap;
use uuid::Uuid;
use crate::widget::stateful::container_widget::ContainerWidgetData;

// Tracks the currently open containers / relevant widget data
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
}