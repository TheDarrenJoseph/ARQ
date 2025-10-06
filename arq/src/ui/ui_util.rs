use std::io::{Error, ErrorKind};

use ratatui::layout::{Rect, Size};
use ratatui::style::Style;
use ratatui::text::{Line, Span};
use ratatui::widgets::Paragraph;

use crate::map::position::{Area, Position};
use crate::ui::resolution::Resolution;
pub use crate::ui::resolution::MIN_RESOLUTION;

enum Alignment {
    LEFT,
    RIGHT
}

pub fn build_paragraph_multi<'a>(messages: Vec<String>) -> Paragraph<'a> {
    let mut spans = Vec::new();
    for line in messages {
        spans.push(Line::from(Span::raw(line.clone())));
    }
    let paragraph = Paragraph::new(spans)
        .style(Style::default())
        .alignment(ratatui::layout::Alignment::Left);
    paragraph
}

pub fn build_paragraph<'a>(text: String) -> Paragraph<'a> {
    let spans = vec![Line::from(Span::raw(text.clone()))];
    let paragraph = Paragraph::new(spans)
        .style(Style::default())
        .alignment(ratatui::layout::Alignment::Left);
    paragraph
}

/*
    target - the expected size e.g height or width
    available - the space available
    alignment - the fallback alignment for any scenario where available does not divide evenly
 */
fn center(target: u16, available: u16, alignment: Alignment)  -> Result<u16, Error> {
    // We've already checked > target height so we only need to check these 2 scenarios
    if target == available {
        Ok(target)
    } else if target > available {
        Err(Error::new(ErrorKind::Other,
                   format!("Target size: {} is above the available range: {}", target, available)))
    } else {
        let diff = available - target;
        // Divides by 2 so we can use the same margin each side
        if diff % 2 == 0 {
            Ok(diff / 2)
        } else {
            // Otherwise top / left align
            let half_diff = diff / 2;
            let remainder = diff % 2;

            return  match alignment {
                Alignment::LEFT => {
                    Ok(half_diff)
                },
                Alignment::RIGHT => {
                    Ok(half_diff + remainder)
                }
            }
        }
    }
}

pub fn center_area(target: Rect, frame_size: Rect, min_resolution: Resolution) -> Result<Area, Error> {
    if target.height < min_resolution.height || target.width < min_resolution.width {
        return Err(Error::new(ErrorKind::Other,
                       format!("Target size: {}, {} is below the minimum supported range: {}, {}", target.width, target.height, min_resolution.width, min_resolution.height)))
    }

    if target.height > frame_size.height && target.width > frame_size.width {
        Err(Error::new(ErrorKind::Other,
                       format!("Target size: {}, {} is above the available range: {}, {}", target.width, target.height, min_resolution.width, min_resolution.height)))
    } else if target.height < frame_size.height || target.width < frame_size.width {
        let target_width = target.width;
        let mut x = target.x;
        if target.width < frame_size.width {
            let frame_width = frame_size.width;
            x = center(target_width, frame_width, Alignment::LEFT)?;
            x += frame_size.x;
        }

        let target_height = target.height;
        let mut y = target.y;
        if target.height < frame_size.height {
            let frame_height = frame_size.height;
            y = center(target_height, frame_height, Alignment::LEFT)?;
            y += frame_size.y;
        }

        let start_position = Position::new(x,y);
        return Ok(Area::new(start_position, target_width, target_height))
    } else {
        // height and width match the target exactly
        return Ok(Area::from_rect(target));
    }
}

pub(crate) fn check_display_size(frame_size_result: Option<Size>) -> Result<(), std::io::Error> {
    if let Some(frame_size) = frame_size_result {
        // Check for 80x24 minimum resolution
        if frame_size.height < MIN_RESOLUTION.height || frame_size.width < MIN_RESOLUTION.width {
            return Err(Error::new(ErrorKind::Other, format!("Sorry, your terminal is below the minimum supported resolution of {}x{}.", MIN_RESOLUTION.height, MIN_RESOLUTION.width)));
        }
        Ok(())
    } else {
        return Err(Error::new(ErrorKind::Other, "Something went wrong; no frame size has been set!"));
    }
}

#[cfg(test)]
mod tests {
    use ratatui::layout::Rect;

    use crate::ui::resolution::MIN_RESOLUTION;
    use crate::ui::ui_util::center_area;

    #[test]
    fn test_center_area_div2_leftalign() {
        // GIVEN a square screen area larger than the target of 80 columns and 24
        let available = Rect::new(0,0, 100, 100);
        let target = Rect::new(0,0, 80,24);

        // WHEN we call to center for the target
        let result = center_area(target, available, MIN_RESOLUTION);
        // THEN we expect a perfectly centered result
        assert!(result.is_ok());
        let result_area = result.unwrap();

        assert_eq!(10, result_area.start_position.x);
        assert_eq!(38, result_area.start_position.y);
        assert_eq!(24, result_area.height);
        assert_eq!(80, result_area.width);
    }

    #[test]
    fn test_center_area_1over_leftalign() {
        // GIVEN a screen area larger than the target of 80 columns and 24
        // AND the screen area has 1 character extra space either side
        let available = Rect::new(0,0, 101, 101);
        let target = Rect::new(0,0, 80,24);

        // WHEN we call to center for the target
        let result = center_area(target, available, MIN_RESOLUTION);
        // THEN we expect a left-aligned result
        assert!(result.is_ok());
        let result_area = result.unwrap();

        assert_eq!(10, result_area.start_position.x);
        assert_eq!(38, result_area.start_position.y);
        assert_eq!(24, result_area.height);
        assert_eq!(80, result_area.width);
    }
}