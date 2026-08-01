use crate::engine::event::ui::UIEvent;

pub trait UIEventHandler {
    async fn handle_event(&mut self, event: UIEvent);
}