use std::convert::TryInto;

use ratatui::layout::Rect;
use ratatui::widgets::Paragraph;

use crate::item_list_selection::{ItemListSelection, ListSelection};
use crate::map::objects::container::Container;
use crate::ui::ui_util::build_paragraph;

pub fn build_page_count<'a>(item_list_selection : &ItemListSelection, area: Rect) -> (Paragraph<'a>, Rect, usize) {
    let page_number = item_list_selection.get_page_number();
    let total_pages = item_list_selection.get_total_pages();
    let item_count = item_list_selection.get_items().len();
    let page_count_text = format!("Page {}/{} ({})", page_number, total_pages, item_count);
    let page_count_text_length = page_count_text.len();
    let width = page_count_text.len().try_into().unwrap();
    let page_count_paragraph = build_paragraph(page_count_text);
    let x_position = area.width.clone() - page_count_text_length as u16;
    let y_position = area.y.clone() + area.height.clone() - 1;
    let page_count_area = Rect::new( x_position, y_position, width, 1);
    (page_count_paragraph, page_count_area, page_count_text_length)
}

pub fn build_weight_limit<'a>(container : &Container, area: Rect, x_offset: usize) -> (Paragraph<'a>, Rect) {
    let weight_limit = container.get_weight_limit();
    let container_item_weight_total = container.get_contents_weight_total();
    let weight_limit_text = format!("{}/{}Kg", container_item_weight_total, weight_limit);
    let weight_limit_text_length = weight_limit_text.len();
    let weight_limit_text_width = weight_limit_text.len().try_into().unwrap();
    let weight_limit_paragraph = build_paragraph(weight_limit_text);
    let weight_limit_area = Rect::new( area.width.clone() - x_offset as u16 - weight_limit_text_length as u16 - 1, area.y.clone() + area.height.clone() - 1, weight_limit_text_width, 1);
    (weight_limit_paragraph, weight_limit_area)
}

#[cfg(test)]
mod tests {
    use ratatui::buffer::Buffer;
    use ratatui::widgets::Widget;
    use uuid::Uuid;

    use crate::item_list_selection::ItemListSelection;
    use crate::map::objects::container::{Container, ContainerType};
    use crate::map::objects::items::{Item, ItemForm, MaterialType};
    use crate::map::position::{Area, Position};
    use crate::view::util::paging::{build_page_count, build_weight_limit};

    fn extract_buffer_line(buffer: &Buffer, line_index: u16) -> String {
        let mut line = String::new();
        for x in 0..buffer.area.width {
            let cell = buffer.get(x as u16, line_index);
            line.push(cell.symbol().parse().unwrap());
        }
        line
    }

    #[test]
    fn test_build_page_count() {
        // GIVEN a view area of
        const WIDTH: i32 = 20;
        const HEIGHT: i32 = 20;
        let area: Area = Area::new(Position::new(0, 0), WIDTH as u16, HEIGHT as u16);
        // AND we have a enough items to fill 2 pages of content
        let bronze_bar = Item::new_with_form(Uuid::new_v4(), "".to_owned(), MaterialType::BRONZE, ItemForm::BAR, 'X', 1.0, 50);
        let mut items = Vec::new();
        for _i in 0..40 {
            items.push(bronze_bar.clone());
        }
        let item_list_selection = ItemListSelection::new(items, HEIGHT);

        // WHEN we call to build the page count Paragraph widget
        let page_count_paragraph = build_page_count(&item_list_selection, area.to_rect());
        // AND we then render it to a buffer
        let mut buffer = Buffer::empty(area.to_rect());
        let paragraph_widget = page_count_paragraph.0;
        // AND we then extract all the cells of the last line
        let paragraph_rect = page_count_paragraph.1;
        // THEN we expect it to render correctly, being at the end of the line
        paragraph_widget.render(paragraph_rect, &mut buffer);
        // Sanity check sizes first
        assert_eq!(buffer.area.height, 20);
        assert_eq!(paragraph_rect.width, 13);
        assert_eq!(paragraph_rect.height, 1);
        // Finally, check the actual buffer
        let last_line = extract_buffer_line(&buffer, 19);
        assert_eq!(last_line, "       Page 1/2 (40)")
    }

    #[test]
    fn test_build_weight_limit_empty() {
        // GIVEN a view area of
        const WIDTH: i32 = 20;
        const HEIGHT: i32 = 20;
        let area: Area = Area::new(Position::new(0, 0), WIDTH as u16, HEIGHT as u16);
        // AND a valid container with no items and a 100kg weight limit
        let inventory = Container::new(Uuid::new_v4(), "Test Person's Inventory".to_owned(), 'X', 1.0, 1, ContainerType::OBJECT, 100);

        // WHEN we call to build the weight limit description
        let weight_limit = build_weight_limit(&inventory, area.to_rect(), 0);
        // AND we then render it to a buffer
        let mut buffer = Buffer::empty(area.to_rect());
        let paragraph_widget = weight_limit.0;
        // AND we then extract all the cells of the last line
        let paragraph_rect = weight_limit.1;
        // THEN we expect it to render correctly, being at the end of the line
        paragraph_widget.render(paragraph_rect, &mut buffer);
        // Sanity check sizes first
        assert_eq!(buffer.area.height, 20);
        assert_eq!(paragraph_rect.width, 7);
        assert_eq!(paragraph_rect.height, 1);
        // Finally, check the actual buffer
        let last_line = extract_buffer_line(&buffer, 19);
        assert_eq!(last_line, "            0/100Kg ")
    }

}