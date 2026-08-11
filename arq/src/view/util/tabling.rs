use crate::map::position::Area;
use ratatui::layout::Alignment;
use ratatui::style::{Color, Style};
use ratatui::text::{Line, Span};
use ratatui::widgets::{Block, Borders, Paragraph, Wrap};

#[derive(Debug, Clone)]
pub struct Column {
    pub name : String,
    pub size : i8
}

pub struct Cell {
    pub column : Column,
    pub area: Area
}

pub fn build_padding(length : i8) -> String {
    let mut s = String::new();
    for _i in 1..length {
        s.push(' ');
    }
    s
}

pub fn build_headings<'a>(columns : Vec<Column>) -> Paragraph<'a> {
    let mut heading_spans = Vec::new();
    let mut spans = Vec::new();
    for column in columns {
        let name = column.name.clone();
        let padding = build_padding(column.size - name.len() as i8 + 1);
        spans.push(Span::raw(column.name.clone()));
        spans.push(Span::raw(padding));
    }
    heading_spans.push(Line::from(spans));
    Paragraph::new(heading_spans)
        .block(Block::default()
            .borders(Borders::NONE))
        .style(Style::default().bg(Color::Black).fg(Color::White))
        .alignment(Alignment::Left)
        .wrap(Wrap { trim: false })
}


#[test]
fn test_build_headings() {
    use ratatui::buffer::{Buffer, Cell};
    use ratatui::layout::Rect;
    use ratatui::widgets::Widget;

    // GIVEN a view with a series of columns configured
    let columns = vec![
        Column {name : "NAME".to_string(), size: 12 },
        Column {name : "WEIGHT (Kg)".to_string(), size: 12 },
        Column {name : "VALUE".to_string(), size: 12 }
    ];
    let total_colum_width = columns.iter().fold(0, |acc, c| acc + c.size);
    assert_eq!(36, total_colum_width);
    
    // WHEN we call to build the headings Paragraph
    let headings = build_headings(columns);

    // THEN we expect it to render to the buffer as expected
    let area = Rect { x: 0, y: 0, height: 1, width: total_colum_width as u16 };
    let _cell_buffer : Vec<Cell> = Vec::new();
    let mut buffer = Buffer::empty(area.clone());
    headings.render(area, &mut buffer);
    let expected_header_text = "NAME        WEIGHT (Kg) VALUE       ";
    // Make sure we're accounting for the total column width in our expected buffer
    assert_eq!(total_colum_width as usize, expected_header_text.len());
    let expected_buffer = Buffer::with_lines(vec![expected_header_text]);
    
    assert_eq!(buffer, expected_buffer);
}