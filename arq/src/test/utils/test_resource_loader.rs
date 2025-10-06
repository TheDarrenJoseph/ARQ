use std::fs::File;
use std::io::Read;

use crate::map::position::Area;
use ratatui::buffer::Buffer;

pub fn read_expected_buffer_file(path: String, buffer_area: Area) -> Buffer {
    let mut input_string = String::new();
    File::open(path.clone()).unwrap().read_to_string(&mut input_string).expect(format!("The file '{}' should have been read to String", path).as_str());

    let mut lines = Vec::new();
    input_string.lines().for_each(|l| lines.push(l));
    let mut buffer_lines: Vec<String> = Vec::new();
    for y in 0..buffer_area.height as usize {
        let line = lines.get(y).expect(format!("File lines should contain an index of: {}", y).as_str());
        let line_string = String::from(*line);
        buffer_lines.push(String::from(line_string))
    }

    Buffer::with_lines(buffer_lines)
}