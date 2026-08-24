use ratatui::layout::Rect;

#[derive(Clone, Debug)]
pub struct Resolution {
    pub width : u16,
    pub height : u16
}

pub const MIN_RESOLUTION: Resolution = Resolution::new(80, 24);
pub const MAX_RESOLUTION: Resolution = Resolution::new(200, 51);

impl Resolution {
    pub const fn new(width: u16, height: u16) -> Resolution {
        Resolution { width, height }
    }
    pub const fn from_rect(rect: Rect) -> Resolution {
        Resolution { width: rect.width, height: rect.height }
    }

    pub fn to_rect(&self) -> Rect {
        Rect {
            x: 0,
            y: 0,
            width: self.width,
            height: self.height
        }
    }
}