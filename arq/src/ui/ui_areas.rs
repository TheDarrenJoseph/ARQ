use std::collections::HashMap;
use std::io::{Error, ErrorKind};

use ratatui::layout::Rect;

use crate::map::position::{build_rectangular_area, Area, Position};

// Total represents the entire available area
pub const UI_AREA_NAME_TOTAL: &str = "total";

// The 'main' area is always present and is the largest area
pub const UI_AREA_NAME_MAIN: &str = "main";

// For the split view, the window is split into 'main' and 'console'
pub const UI_AREA_NAME_CONSOLE: &str = "console";

// For the combat, the window is split into 'main', 'console', and 'minimap'
pub const UI_AREA_NAME_MINIMAP: &str = "minimap";


/*
 UIArea entries keyed by ID for easy reference
 */
#[derive(Clone, Debug)]
pub struct UIAreas {
    areas: HashMap<String, UIArea>
}

#[derive(Clone, Debug)]
pub struct UIArea {
    pub name : String,
    pub area: Area
}

impl UIArea {
    pub fn get_bordered_area(&self) -> BorderedArea {
        BorderedArea::from_area(self.area).unwrap()
    }
}

impl UIAreas {

    pub const fn new(areas: HashMap<String, UIArea>) -> UIAreas {
        UIAreas { areas }
    }

    pub fn get_area(&self, key: &str) -> Option<&UIArea> {
        self.areas.get(key)
    }

    pub fn get_bordered_area(&self, key: &str) -> BorderedArea {
        let area = self.areas.get(key).unwrap().get_bordered_area();
        area
    }

    pub fn len(&self) -> usize {
        self.areas.len()
    }
}

/*
 An area with a 1 character wide border on the outer area
 Inner area is the area inside this border
 */
#[derive(Clone)]
#[derive(Debug)]
pub struct BorderedArea {
    pub outer: Area,
    pub inner: Area
}

impl BorderedArea {
    pub fn new(start_position: Position, width: u16, height: u16) -> Result<BorderedArea, std::io::Error> {
        if width > 2 && height > 2 {
            let outer_area = build_rectangular_area(start_position, width, height);
            let inner_start_position = Position::new(start_position.x +1, start_position.y +1);
            let inner_area = build_rectangular_area(inner_start_position, width - 2, height - 2);
            Ok(BorderedArea { outer: outer_area, inner: inner_area })
        } else {
            Err(Error::new(ErrorKind::Other, String::from("Cannot build a bordered area for anything less than a 3x3 area")))
        }
    }

    pub fn from_area(area: Area) -> Result<BorderedArea, std::io::Error> {
        return BorderedArea::new(area.start_position, area.width, area.height);
    }

    pub fn from_rect(rect: Rect) -> Result<BorderedArea, std::io::Error> {
        return BorderedArea::new(Position::new(rect.x, rect.y), rect.width, rect.height);
    }
}

#[cfg(test)]
mod bordered_area_tests {
    use crate::map::position::Position;
    use crate::ui::ui_areas::BorderedArea;

    #[test]
    fn test_bordered_area_new() {
        let start_position = Position::new(0,0);
        const SIZE: u16 = 3;
        let result = BorderedArea::new(start_position, SIZE, SIZE).unwrap();

        let outer_area = result.outer;
        assert_eq!(0, outer_area.start_position.x);
        assert_eq!(0, outer_area.start_position.y);
        assert_eq!(3, outer_area.width);
        assert_eq!(3, outer_area.height);
        assert_eq!(2, outer_area.end_position.x);
        assert_eq!(2, outer_area.end_position.y);

        let inner_area = result.inner;
        assert_eq!(1, inner_area.start_position.x);
        assert_eq!(1, inner_area.start_position.y);
        assert_eq!(1, inner_area.width);
        assert_eq!(1, inner_area.height);
        assert_eq!(1, inner_area.end_position.x);
        assert_eq!(1, inner_area.end_position.y);
    }
}

