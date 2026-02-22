use rand::distr::{Distribution, StandardUniform};
use rand::Rng;
use ratatui::layout::{Rect, Size};

use crate::ui::resolution::Resolution;

#[derive(Copy, Clone, std::cmp::PartialEq, Hash, Debug)]
pub struct Position {
    pub x : u16,
    pub y : u16
}


impl Position {
    pub fn new(x: u16, y: u16) -> Position {
        Position { x, y }
    }
    pub const fn zero() -> Position {
        Position { x: 0, y: 0 }
    }

    pub fn from_rect(rect: Rect) -> Position {
        Position { x : rect.x, y: rect.y }
    }

    pub fn from_size(size: Size) -> Position {
        Position { x : size.width, y: size.height }
    }

    pub fn get_neighbors(&self) -> Vec<Position> {
        let mut positions = Vec::new();
        // Left
        if self.x > 0 {
            positions.push(Position { x: self.x - 1, y: self.y });
        }
        // Right
        if self.x < u16::MAX {
            positions.push(Position { x: self.x + 1, y: self.y });
        }
        // Top
        if self.y > 0 {
            positions.push(Position { x: self.x, y: self.y - 1 });
        }
        // Bottom
        if self.y < u16::MAX {
            positions.push(Position { x: self.x, y: self.y + 1 });
        }
        positions
    }

    pub fn describe_neighbor(&self, position: Position) -> String {
        if (position.x == self.x - 1 && position.y == self.y) {
            return String::from("Left")
        }

        if (position.x == self.x + 1 && position.y == self.y) {
            return String::from("Right")
        }

        if (position.x == self.x  && position.y == self.y - 1) {
            return String::from("Top")
        }

        if (position.x == self.x  && position.y == self.y + 1) {
            return String::from("Bottom")
        }

        if (position.x == self.x  && position.y == self.y) {
            return String::from("Current")
        }

        return "Unknown".to_string()
    }
    
    pub fn equals(&self, position: Position) -> bool {
        return self.x == position.x && self.y == position.y;
    }

    pub fn equals_option(&self, position: Option<Position>) -> bool {
        return if let Some(pos) = position {
            self.x == pos.x && self.y == pos.y
        } else {
            false
        }
    }

    fn calculate_unsigned_offset_value(&self, actual: u16, offset: i32) -> u16 {
        let mut output = actual;
        if offset < 0 {
            let negative_offset = i32::abs(offset) as u16;
            if output >= negative_offset {
                output -= negative_offset;
            } else {
                output = 0;
            }
        } else if offset > 0 {
            output += offset as u16;
        }
        return output;
    }

    pub fn offset(&mut self, offset_x: i32, offset_y: i32) -> Position {
        self.x = self.calculate_unsigned_offset_value(self.x, offset_x);
        self.y = self.calculate_unsigned_offset_value(self.y, offset_y);
        *self
    }
}

impl Eq for Position {}

#[derive(Copy, Clone, std::cmp::PartialEq, Debug)]
pub struct Area {
    pub start_position : Position,
    pub end_position : Position,
    pub width: u16,
    pub height: u16
}

impl Area {
    pub const fn new(start_position: Position, width: u16, height: u16) -> Area {
        build_rectangular_area(start_position, width,height)
    }

    pub const fn from_resolution(resolution: Resolution) -> Area {
        build_rectangular_area(Position::zero(), resolution.width, resolution.height)
    }

    pub fn from_rect(rect: Rect) -> Area {
        build_rectangular_area(Position::from_rect(rect), rect.width,rect.height)
    }

    pub fn to_rect(&self) -> Rect {
        Rect { x: self.start_position.x, y: self.start_position.y, width: self.width, height: self.height }
    }

    pub fn get_position(&self, x: u16, y: u16) -> Position {
        let result_x = self.start_position.x + x;
        let result_y = self.start_position.y + y;
        return Position::new(result_x, result_y);
    }

    pub fn get_total_area(&self) -> u16 {
        self.width * self.height
    }

    pub fn get_size_x(&self) -> u16 {
        self.width
    }

    pub fn get_size_y(&self) -> u16 {
        self.height
    }

    pub fn get_sides(&self) -> Vec<AreaSide> {
        let start_pos = &self.start_position;
        let mut sides = Vec::new();
        for side in all_sides().iter() {
            if *side == Side::LEFT || *side == Side::RIGHT {
                sides.push(build_line(start_pos.clone(), self.height, side.clone()));
            } else {
                sides.push(build_line(start_pos.clone(), self.width, side.clone()));
            }
        }
        sides
    }

    pub fn intersects(&self, area: Area) -> bool {

        let start_x = self.start_position.x;
        let start_y = self.start_position.y;
        let end_x = self.end_position.x;
        let end_y = self.end_position.y;

        let area_start_x = area.start_position.x;
        let area_start_y = area.start_position.y;

        let area_end_x = area.end_position.x;
        let area_end_y = area.end_position.y;

        let area_start_x_intersect = area_start_x >= start_x && area_start_x <= end_x;
        let area_end_x_intersect = area_end_x >= start_x && area_end_x <= end_x;

        let area_start_y_intersect = area_start_y >= start_y && area_start_y <= end_y;
        let area_end_y_intersect = area_end_y >= start_y && area_end_y <= end_y;

        let intersects = (area_start_x_intersect || area_end_x_intersect) && (area_start_y_intersect || area_end_y_intersect);
        intersects
    }


    pub fn intersects_or_touches(&self, area: Area) -> bool {
        // Adjust boundaries to allow 1 place above or below each range
        let start_x = if self.start_position.x > 0 { self.start_position.x - 1 } else { self.start_position.x };
        let start_y = if self.start_position.y > 0 {  self.start_position.y - 1 } else {  self.start_position.y };
        let end_x = if self.end_position.x < u16::MAX { self.end_position.x + 1 } else { self.end_position.x };
        let end_y = if self.end_position.y < u16::MAX { self.end_position.y + 1 } else { self.end_position.y };

        // Cast to signed and get absolute value to find the diff
        let size_x = (start_x as i32 - end_x as i32).abs() as u16;
        let size_y = (start_y as i32 - end_y as i32).abs() as u16;

        let start_position = Position { x: start_x, y: start_y };
        let end_position = Position { x: end_x, y: end_y};

        let self_adjusted_area = Area { start_position, end_position, width: size_x, height: size_y };
        self_adjusted_area.intersects(area)
    }

    pub fn contains_position(&self, position: Position) -> bool {
        let x= position.x;
        let y = position.y;

        let lower_x_bound = x >= self.start_position.x;
        let lower_y_bound = y >= self.start_position.y;
        let upper_x_bound = x <= self.end_position.x;
        let upper_y_bound = y <= self.end_position.y;
        let in_bounds = lower_x_bound && lower_y_bound && upper_x_bound && upper_y_bound;
        in_bounds
    }

    pub fn contains(&self, x: u16, y: u16) -> bool {
        let lower_x_bound = x >= self.start_position.x;
        let lower_y_bound = y >= self.start_position.y;
        let upper_x_bound = x <= self.end_position.x;
        let upper_y_bound = y <= self.end_position.y;
        let in_bounds = lower_x_bound && lower_y_bound && upper_x_bound && upper_y_bound;
        in_bounds
    }

    pub fn can_fit(&self, position: Position, size: u16) -> bool {
        if size == 0 {
            return false;
        }

        let x= position.x;
        let y = position.y;
        let in_bounds = self.contains_position(position);
        if in_bounds && size <= self.width && size <= self.height {
            let end_x = x + size - 1;
            let end_y = y + size - 1;

            let end_x_bound = end_x <= self.end_position.x;
            let end_y_bound = end_y <= self.end_position.y;
            let end_bound_fit = end_x_bound && end_y_bound;
            return end_bound_fit;
        }
        false
    }

    pub fn get_positions(&self) -> Vec<Position> {
        let mut positions = Vec::new();
        let start_pos = self.start_position;
        let end_pos = self.end_position;
        for x in start_pos.x..end_pos.x+1  {
            for y in start_pos.y..end_pos.y+1 {
                positions.push(Position { x, y });
            }
        }
        positions
    }

    pub fn get_description(&self) -> String {
        format!("{}x{}", self.width, self.height)
    }

}

#[derive(Copy, Clone, Debug, PartialEq)]
pub enum Side {
    LEFT,
    RIGHT,
    TOP,
    BOTTOM
}

impl Distribution<Side> for StandardUniform {
    fn sample<R: Rng + ?Sized>(&self, rng: &mut R) -> Side {
        match rng.gen_range(0..=3) {
            0 => Side::LEFT,
            1 => Side::RIGHT,
            2 => Side::TOP,
            3 => Side::BOTTOM,
            _ => Side::LEFT,
        }
    }
}

#[derive(Copy, Clone)]
pub struct AreaSide {
    pub area: Area,
    pub side : Side
}

impl AreaSide {
    pub fn get_mid_point(&self) -> Position {
       match self.side {
           Side::LEFT => {
               Position { x: self.area.start_position.x, y: self.area.start_position.y + (self.area.height - 1) / 2 }
           },
           Side::RIGHT => {
               Position { x: self.area.end_position.x, y: self.area.start_position.y + (self.area.height - 1) / 2 }
           },
           Side::TOP => {
               Position { x: self.area.start_position.x + (self.area.width - 1) / 2, y: self.area.start_position.y}
           },
           Side::BOTTOM => {
               Position { x: self.area.start_position.x + (self.area.width - 1) / 2, y: self.area.end_position.y }
           }
       }
    }
}

pub fn all_sides() -> [Side; 4] {
    [Side::LEFT,Side::RIGHT,Side::TOP,Side::BOTTOM]
}

pub fn build_line(start_position : Position, size: u16, side: Side) -> AreaSide {
    let start_x = start_position.x;
    let start_y = start_position.y;

    let start_position;
    let end_position;
    let zero_indexed_size = size - 1;
    let size_x;
    let size_y;
    match side {
        Side::LEFT => {
            start_position = Position { x : start_x, y: start_y};
            end_position = Position { x : start_x, y: start_y + zero_indexed_size};
            size_x = 1;
            size_y = size;
        },
        Side::RIGHT => {
            start_position = Position { x : start_x + zero_indexed_size, y: start_y};
            end_position = Position { x : start_x + zero_indexed_size, y: start_y + zero_indexed_size};
            size_x = 1;
            size_y = size;
        },
        Side::TOP => {
            start_position = Position { x : start_x, y: start_y};
            end_position = Position { x : start_x + zero_indexed_size, y: start_y};
            size_x = size;
            size_y = 1;
        },
        Side::BOTTOM => {
            start_position = Position { x : start_x, y: start_y + zero_indexed_size};
            end_position = Position { x : start_x + zero_indexed_size, y: start_y + zero_indexed_size};
            size_x = size;
            size_y = 1;
        }
    }
    let area = Area { start_position, end_position, width: size_x, height: size_y };
    AreaSide { area, side }
}

pub const fn build_square_area(start_position : Position, size: u16) -> Area {
    let start_x = start_position.x;
    let start_y = start_position.y;
    let end_position = Position { x : start_x + (size - 1), y: start_y + (size - 1)};
    Area { start_position, end_position, width: size, height: size }
}

pub const fn build_rectangular_area(start_position : Position, size_x: u16, size_y: u16) -> Area {
    let start_x = start_position.x;
    let start_y = start_position.y;

    // Handle 0 sizing
    let end_x = if size_x > 0 {
        start_x + size_x - 1
    } else {
        0
    };

    // Handle 0 sizing
    let end_y = if size_y > 0 {
        start_y + size_y - 1
    } else {
        0
    };

    let end_position = Position{x: end_x, y: end_y};
    Area { start_position, end_position, width: size_x, height: size_y }
}

#[cfg(test)]
mod tests {
    use ratatui::layout::Rect;

    use crate::map::position::{build_rectangular_area, build_square_area, Area, Position, Side};

    #[test]
    fn test_get_neighbors_top_left() {
        // GIVEN a position in the top left corner
        let start_pos = Position { x: 0, y: 0 };
        // WHEN we call to get it's neighbors
        let neighbors = start_pos.get_neighbors();
        // THEN we expect 2 positions to return. The right-hand and the below neighbor
        assert_eq!(2, neighbors.len());
        assert_eq!(Position{x: 1, y: 0}, *neighbors.get(0).unwrap());
        assert_eq!(Position{x: 0, y: 1}, *neighbors.get(1).unwrap());
    }


    #[test]
    fn test_get_neighbors_top_right() {
        // GIVEN a position in the top right corner
        let start_pos = Position { x: u16::MAX, y: 0 };
        // WHEN we call to get it's neighbors
        let neighbors = start_pos.get_neighbors();
        // THEN we expect 2 positions to return. The left-hand and the below neighbor
        assert_eq!(2, neighbors.len());
        assert_eq!(Position{x: u16::MAX - 1, y: 0}, *neighbors.get(0).unwrap());
        assert_eq!(Position{x: u16::MAX, y: 1}, *neighbors.get(1).unwrap());
    }

    #[test]
    fn test_get_neighbors_bottom_left() {
        // GIVEN a position in the bottom left corner
        let start_pos = Position { x: 0, y: u16::MAX };
        // WHEN we call to get it's neighbors
        let neighbors = start_pos.get_neighbors();
        // THEN we expect 2 positions to return. The right-hand and the above neighbor
        assert_eq!(2, neighbors.len());
        assert_eq!(Position{x: 1, y: u16::MAX }, *neighbors.get(0).unwrap());
        assert_eq!(Position{x: 0, y: u16::MAX - 1 }, *neighbors.get(1).unwrap());
    }

    #[test]
    fn test_get_neighbors_bottom_right() {
        // GIVEN a position in the bottom right corner
        let start_pos = Position { x: u16::MAX, y: u16::MAX };
        // WHEN we call to get it's neighbors
        let neighbors = start_pos.get_neighbors();
        // THEN we expect 2 positions to return. The left-hand and the above neighbor
        assert_eq!(2, neighbors.len());
        assert_eq!(Position{x: u16::MAX - 1, y: u16::MAX }, *neighbors.get(0).unwrap());
        assert_eq!(Position{x: u16::MAX, y: u16::MAX - 1 }, *neighbors.get(1).unwrap());
    }


    #[test]
    fn test_get_neighbors_mid_point() {
        // GIVEN a position in the center of the possible range
        let start_pos = Position { x: u16::MAX/2, y: u16::MAX/2 };
        // WHEN we call to get it's neighbors
        let neighbors = start_pos.get_neighbors();
        // THEN we expect 4 positions to return. The left-hand, right, above, and below neighbor
        assert_eq!(4, neighbors.len());
        assert_eq!(Position{x: u16::MAX/2 - 1, y: u16::MAX/2 }, *neighbors.get(0).unwrap());
        assert_eq!(Position{x: u16::MAX/2 + 1, y: u16::MAX/2 }, *neighbors.get(1).unwrap());
        assert_eq!(Position{x: u16::MAX/2, y: u16::MAX/2 - 1 }, *neighbors.get(2).unwrap());
        assert_eq!(Position{x: u16::MAX/2, y: u16::MAX/2 + 1  }, *neighbors.get(3).unwrap());
    }

    #[test]
    fn test_build_square_area() {
        let start_pos = Position { x: 1, y: 2 };
        let area = build_square_area(start_pos, 3);
        assert_eq!(3, area.width);
        assert_eq!(3, area.height);
        assert_eq!(1, area.start_position.x);
        assert_eq!(2, area.start_position.y);
        assert_eq!(3, area.end_position.x);
        assert_eq!(4, area.end_position.y);
    }

    #[test]
    fn test_build_rectangular_area() {
        let start_pos = Position { x: 0, y: 0 };
        let area = build_rectangular_area(start_pos, 6, 3);
        assert_eq!(6, area.width);
        assert_eq!(3, area.height);
        assert_eq!(0, area.start_position.x);
        assert_eq!(0, area.start_position.y);
        assert_eq!(5, area.end_position.x);
        assert_eq!(2, area.end_position.y);
    }

    #[test]
    fn test_build_rectangular_area_size_0() {
        // Given a Position starting at 0,0
        let start_pos = Position { x: 0, y: 0 };
        // And it's zero sized
        let area = build_rectangular_area(start_pos, 0,0);
        // THEN we expect everything to be 0
        assert_eq!(0, area.width);
        assert_eq!(0, area.height);
        assert_eq!(0, area.start_position.x);
        assert_eq!(0, area.start_position.y);
        assert_eq!(0, area.end_position.x);
        assert_eq!(0, area.end_position.y);
    }

    #[test]
    fn test_get_total_area() {
        // GIVEN a valid area
        let start_pos = Position { x: 0, y: 0 };
        let area = build_square_area(start_pos, 4);
        assert_eq!(4, area.width);
        assert_eq!(4, area.height);
        assert_eq!(0, area.start_position.x);
        assert_eq!(0, area.start_position.y);
        assert_eq!(3, area.end_position.x);
        assert_eq!(3, area.end_position.y);

        // WHEN we call to get the total area
        let total_area = area.get_total_area();

        // THEN we expect it to equal width (x) * height (y) / 4x4
        assert_eq!(16, total_area);
    }

    #[test]
    fn test_get_get_positions() {
        // GIVEN a valid area
        let start_pos = Position { x: 0, y: 0 };
        let area = build_square_area(start_pos, 4);
        assert_eq!(4, area.width);
        assert_eq!(4, area.height);
        assert_eq!(0, area.start_position.x);
        assert_eq!(0, area.start_position.y);
        assert_eq!(3, area.end_position.x);
        assert_eq!(3, area.end_position.y);

        // WHEN we call to get all possible positions
        let positions = area.get_positions();

        // THEN we expect it to equal width (x) * height (y) / 4x4
        assert_eq!(16, positions.len());
       // First column
        assert_eq!(Position { x: 0, y: 0}, *positions.get(0).unwrap());
        assert_eq!(Position { x: 0, y: 1}, *positions.get(1).unwrap());
        assert_eq!(Position { x: 0, y: 2}, *positions.get(2).unwrap());
        assert_eq!(Position { x: 0, y: 3}, *positions.get(3).unwrap());

        // Second column
        assert_eq!(Position { x: 1, y: 0}, *positions.get(4).unwrap());
        assert_eq!(Position { x: 1, y: 1}, *positions.get(5).unwrap());
        assert_eq!(Position { x: 1, y: 2}, *positions.get(6).unwrap());
        assert_eq!(Position { x: 1, y: 3}, *positions.get(7).unwrap());

        // Third column
        assert_eq!(Position { x: 2, y: 0}, *positions.get(8).unwrap());
        assert_eq!(Position { x: 2, y: 1}, *positions.get(9).unwrap());
        assert_eq!(Position { x: 2, y: 2}, *positions.get(10).unwrap());
        assert_eq!(Position { x: 2, y: 3}, *positions.get(11).unwrap());

        // Fourth column
        assert_eq!(Position { x: 3, y: 0}, *positions.get(12).unwrap());
        assert_eq!(Position { x: 3, y: 1}, *positions.get(13).unwrap());
        assert_eq!(Position { x: 3, y: 2}, *positions.get(14).unwrap());
        assert_eq!(Position { x: 3, y: 3}, *positions.get(15).unwrap());
    }

    #[test]
    fn test_get_get_positions_nonzero_start() {
        // GIVEN a valid area starting at index 4, 4
        let start_pos = Position { x: 4, y: 4 };
        let area = build_square_area(start_pos, 4);
        assert_eq!(4, area.width);
        assert_eq!(4, area.height);
        assert_eq!(4, area.start_position.x);
        assert_eq!(4, area.start_position.y);
        assert_eq!(7, area.end_position.x);
        assert_eq!(7, area.end_position.y);

        // WHEN we call to get all possible positions
        let positions = area.get_positions();

        // THEN we expect it to equal width (x) * height (y) / 4x4
        assert_eq!(16, positions.len());
        // First column
        assert_eq!(Position { x: 4, y: 4}, *positions.get(0).unwrap());
        assert_eq!(Position { x: 4, y: 5}, *positions.get(1).unwrap());
        assert_eq!(Position { x: 4, y: 6}, *positions.get(2).unwrap());
        assert_eq!(Position { x: 4, y: 7}, *positions.get(3).unwrap());

        // Second column
        assert_eq!(Position { x: 5, y: 4}, *positions.get(4).unwrap());
        assert_eq!(Position { x: 5, y: 5}, *positions.get(5).unwrap());
        assert_eq!(Position { x: 5, y: 6}, *positions.get(6).unwrap());
        assert_eq!(Position { x: 5, y: 7}, *positions.get(7).unwrap());

        // Third column
        assert_eq!(Position { x: 6, y: 4}, *positions.get(8).unwrap());
        assert_eq!(Position { x: 6, y: 5}, *positions.get(9).unwrap());
        assert_eq!(Position { x: 6, y: 6}, *positions.get(10).unwrap());
        assert_eq!(Position { x: 6, y: 7}, *positions.get(11).unwrap());

        // Fourth column
        assert_eq!(Position { x: 7, y: 4}, *positions.get(12).unwrap());
        assert_eq!(Position { x: 7, y: 5}, *positions.get(13).unwrap());
        assert_eq!(Position { x: 7, y: 6}, *positions.get(14).unwrap());
        assert_eq!(Position { x: 7, y: 7}, *positions.get(15).unwrap());
    }

    #[test]
    fn test_3x3_valid_top_left_intersects() {
        // GIVEN a 3x3 Area
        let area = build_square_area(Position { x: 1, y: 1 }, 3);
        assert_eq!(3, area.width);
        assert_eq!(3, area.height);
        assert_eq!(1, area.start_position.x);
        assert_eq!(1, area.start_position.y);
        assert_eq!(3, area.end_position.x);
        assert_eq!(3, area.end_position.y);

        // WHEN we call to see if an area overlaping the top-left corner intersects
        let overlap_area = build_square_area(Position { x: 0, y: 0 }, 2);
        // THEN we expect the result to be true
        assert!(area.intersects(overlap_area));
    }

    #[test]
    fn test_3x3_valid_top_right_intersects() {
        // GIVEN a 3x3 Area (x)
        let area = build_square_area(Position { x: 1, y: 1 }, 3);
        assert_eq!(3, area.width);
        assert_eq!(3, area.height);
        assert_eq!(1, area.start_position.x);
        assert_eq!(1, area.start_position.y);
        assert_eq!(3, area.end_position.x);
        assert_eq!(3, area.end_position.y);

        // WHEN we call to see if an area (X) overlaping the top-right corner intersects
        // i.e
        // ---XX
        // -xxXX
        // -xxx-
        // -xxx-
        let overlap_area = build_square_area(Position { x: 3, y: 0 }, 2);
        // THEN we expect the result to be true
        assert!(area.intersects(overlap_area));
    }

    #[test]
    fn test_3x3_valid_bottom_right_intersects() {
        // GIVEN a 3x3 Area (x)
        let area = build_square_area(Position { x: 1, y: 1 }, 3);
        assert_eq!(3, area.width);
        assert_eq!(3, area.height);
        assert_eq!(1, area.start_position.x);
        assert_eq!(1, area.start_position.y);
        assert_eq!(3, area.end_position.x);
        assert_eq!(3, area.end_position.y);

        // WHEN we call to see if an area (X) overlaping the top-right corner intersects
        // i.e
        //  01234
        // 0-----
        // 1-xxx-
        // 2-xxx-
        // 3-xxXX
        // 4---XX
        let overlap_area = build_square_area(Position { x: 3, y: 3 }, 2);
        // THEN we expect the result to be true
        assert!(area.intersects(overlap_area));
    }

    #[test]
    fn test_3x3_valid_bottom_left_intersects() {
        // GIVEN a 3x3 Area (x)
        let area = build_square_area(Position { x: 1, y: 1 }, 3);
        assert_eq!(3, area.width);
        assert_eq!(3, area.height);
        assert_eq!(1, area.start_position.x);
        assert_eq!(1, area.start_position.y);
        assert_eq!(3, area.end_position.x);
        assert_eq!(3, area.end_position.y);

        // WHEN we call to see if an area (X) overlaping the top-right corner intersects
        // i.e
        //  01234
        // 0-----
        // 1-xxx-
        // 2-xxx-
        // 3XXxx-
        // 4XX---
        let overlap_area = build_square_area(Position { x: 0, y: 3 }, 2);
        // THEN we expect the result to be true
        assert!(area.intersects(overlap_area));
    }

    #[test]
    fn test_3x3_invalid_ends_before_intersects() {
        // GIVEN a 3x3 Area
        let area = build_square_area(Position { x: 1, y: 1 }, 3);
        assert_eq!(3, area.width);
        assert_eq!(3, area.height);
        assert_eq!(1, area.start_position.x);
        assert_eq!(1, area.start_position.y);
        assert_eq!(3, area.end_position.x);
        assert_eq!(3, area.end_position.y);

        // WHEN we call to see if an area that ends touching the top-left corner intersects
        let overlap_area = build_square_area(Position { x: 0, y: 0 }, 1);
        // THEN we expect the result to be false
        assert!(!area.intersects(overlap_area));
    }

    #[test]
    fn test_3x3_invalid_starts_after_intersects() {
        // GIVEN a 3x3 Area
        let area = build_square_area(Position { x: 1, y: 1 }, 3);
        assert_eq!(3, area.width);
        assert_eq!(3, area.height);
        assert_eq!(1, area.start_position.x);
        assert_eq!(1, area.start_position.y);
        assert_eq!(3, area.end_position.x);
        assert_eq!(3, area.end_position.y);

        // WHEN we call to see if an area that starts after the bottom-right corner intersects
        let overlap_area = build_square_area(Position { x: 4, y: 4 }, 3);
        // THEN we expect the result to be false
        assert!(!area.intersects(overlap_area));
    }

    #[test]
    fn test_3x3_valid_contains() {
        // GIVEN a 3x3 Area
        let area = build_square_area(Position { x: 0, y: 0 }, 3);
        assert_eq!(3, area.width);
        assert_eq!(3, area.height);
        assert_eq!(0, area.start_position.x);
        assert_eq!(0, area.start_position.y);
        assert_eq!(2, area.end_position.x);
        assert_eq!(2, area.end_position.y);

        // WHEN we call to see if points 0,0 - 2,2 are contained
        // THEN we expect the results to be true
        assert!(area.contains_position(Position { x: 0, y: 0 }));
        assert!(area.contains_position(Position { x: 0, y: 1 }));
        assert!(area.contains_position(Position { x: 0, y: 2 }));
        assert!(area.contains_position(Position { x: 1, y: 0 }));
        assert!(area.contains_position(Position { x: 1, y: 1 }));
        assert!(area.contains_position(Position { x: 1, y: 2 }));
        assert!(area.contains_position(Position { x: 2, y: 0 }));
        assert!(area.contains_position(Position { x: 2, y: 1 }));
        assert!(area.contains_position(Position { x: 2, y: 2 }));
    }

    #[test]
    fn test_3x3_invalid_contains() {
        // GIVEN a 3x3 Area
        let area = build_square_area(Position { x: 0, y: 0 }, 3);
        assert_eq!(3, area.width);
        assert_eq!(3, area.height);
        assert_eq!(0, area.start_position.x);
        assert_eq!(0, area.start_position.y);
        assert_eq!(2, area.end_position.x);
        assert_eq!(2, area.end_position.y);

        // WHEN we call to see if points above it's range are contained
        // THEN we expect the results to be false
        assert!(!area.contains_position(Position { x: 3, y: 0 }));
        assert!(!area.contains_position(Position { x: 0, y: 3 }));
        assert!(!area.contains_position(Position { x: 3, y: 3 }));
    }

    #[test]
    fn test_3x3_offset_invalid_contains() {
        // GIVEN a 3x3 Area
        let area = build_square_area(Position { x: 1, y: 1 }, 3);
        assert_eq!(3, area.width);
        assert_eq!(3, area.height);
        // That starts at position 1,1
        assert_eq!(1, area.start_position.x);
        assert_eq!(1, area.start_position.y);
        // And ends at 3,3
        assert_eq!(3, area.end_position.x);
        assert_eq!(3, area.end_position.y);

        // WHEN we call to see if points above it's range are contained
        // THEN we expect the results to be false
        assert!(!area.contains_position(Position { x: 4, y: 0 }));
        assert!(!area.contains_position(Position { x: 0, y: 4 }));
        assert!(!area.contains_position(Position { x: 4, y: 4 }));
    }

    #[test]
    fn test_3x3_can_fit_2x2_at_1_1() {
        // GIVEN a 3x3 Area
        let area = build_square_area(Position { x: 0, y: 0 }, 3);

        // WHEN we call to see if the central point can fit a 2x2 area
        let can_fit = area.can_fit(Position { x: 1, y: 1 }, 2);

        // THEN we expect the result to be true
        assert_eq!(true, can_fit);
    }

    #[test]
    fn test_3x3_can_fit_2x2_at_start() {
        // GIVEN a 3x3 Area
        let area = build_square_area(Position { x: 0, y: 0 }, 3);

        // WHEN we call to see if the start point can fit a 2x2 area
        let can_fit = area.can_fit(Position { x: 0, y: 0 }, 2);

        // THEN we expect the result to be true
        assert_eq!(true, can_fit);
    }

    #[test]
    fn test_3x3_cannot_fit_2x2_at_end() {
        // GIVEN a 3x3 Area
        let area = build_square_area(Position { x: 0, y: 0 }, 3);

        // WHEN we call to see if the end point can fit a 2x2 area
        let can_fit = area.can_fit(Position { x: 2, y: 2 }, 4);

        // THEN we expect the result to be false
        assert_ne!(true, can_fit);
    }

    #[test]
    fn test_3x3_cannot_fit_4x4_at_start() {
        // GIVEN a 3x3 Area
        let area = build_square_area(Position { x: 0, y: 0 }, 3);

        // WHEN we call to see if the start point can fit a 4x4 area
        let can_fit = area.can_fit(Position { x: 0, y: 0 }, 4);

        // THEN we expect the result to be false
        assert_ne!(true, can_fit);
    }

    #[test]
    fn test_3x3_cannot_fit_zero() {
        // GIVEN a 3x3 Area
        let area = build_square_area(Position { x: 0, y: 0 }, 3);

        // WHEN we call to see if the start point can fit a zero sized area
        let can_fit = area.can_fit(Position { x: 0, y: 0 }, 0);

        // THEN we expect the result to be false
        assert_ne!(true, can_fit);
    }

    #[test]
    fn test_get_sides() {
        let start_position = Position { x: 0, y: 0};
        let area = build_square_area(start_position, 3);
        let sides = area.get_sides();

        assert_eq!(4, sides.len());

        let left = sides[0];
        assert_eq!(Side::LEFT, left.side);
        assert_eq!(0, left.area.start_position.x);
        assert_eq!(0, left.area.start_position.y);
        assert_eq!(0, left.area.end_position.x);
        assert_eq!(2, left.area.end_position.y);

        let right = sides[1];
        assert_eq!(Side::RIGHT, right.side);
        assert_eq!(2, right.area.start_position.x);
        assert_eq!(0, right.area.start_position.y);
        assert_eq!(2, right.area.end_position.x);
        assert_eq!(2, right.area.end_position.y);

        let top = sides[2];
        assert_eq!(Side::TOP, top.side);
        assert_eq!(0, top.area.start_position.x);
        assert_eq!(0, top.area.start_position.y);
        assert_eq!(2, top.area.end_position.x);
        assert_eq!(0, top.area.end_position.y);

        let bottom = sides[3];
        assert_eq!(Side::BOTTOM, bottom.side);
        assert_eq!(0, bottom.area.start_position.x);
        assert_eq!(2, bottom.area.start_position.y);
        assert_eq!(2, bottom.area.end_position.x);
        assert_eq!(2, bottom.area.end_position.y);
    }

    #[test]
    fn test_offset_positive() {
        // GIVEN an initial position
        let mut position = Position { x: 3, y: 3};

        // WHEN we try to offset by +1
        position = position.offset(1, 1);
        // THEN we should see x: 4, y: 4
        assert_eq!(4, position.x);
        assert_eq!(4, position.y);
    }

    #[test]
    fn test_offset_negative() {
        // GIVEN an initial position
        let mut position = Position { x: 3, y: 3};

        // WHEN we try to offset by -1
        position = position.offset(-1, -1);
        // THEN we should see x: 2, y: 2
        assert_eq!(2, position.x);
        assert_eq!(2, position.y);
    }

    #[test]
    fn test_offset_negative_lower_bound() {
        // GIVEN an initial position
        let mut position = Position { x: 3, y: 3};

        // WHEN we try to offset by -4 (taking us below 0)
        position = position.offset(-4, -4);
        // THEN we should see x: 0, y: 0 (the lowest possible positive values w/ offset)
        assert_eq!(0, position.x);
        assert_eq!(0, position.y);
    }

    #[test]
    fn test_area_new() {
        let start_position = Position::new(0,0);
        const SIZE: u16 = 3;
        let area = Area::new(start_position, SIZE, SIZE);
        assert_eq!(0, area.start_position.x);
        assert_eq!(0, area.start_position.y);
        assert_eq!(3, area.width);
        assert_eq!(3, area.height);
        assert_eq!(2, area.end_position.x);
        assert_eq!(2, area.end_position.y);
    }

    #[test]
    fn from_rect() {
        // GIVEN a Rectangle starting at Pos x:1, y:1 and of size 3x3
        let rect = Rect::new(1,1,3,3);

        // WHEN we call to build an area from this
        let area = Area::from_rect(rect);

        // THEN we expect an exact match to the Rect provided
        assert_eq!(rect.x, area.start_position.x);
        assert_eq!(rect.y, area.start_position.y);
        assert_eq!(rect.x + rect.width - 1, area.end_position.x);
        assert_eq!(rect.y + rect.height - 1, area.end_position.y);
        assert_eq!(rect.width, area.width);
        assert_eq!(rect.height, area.height);
    }
}