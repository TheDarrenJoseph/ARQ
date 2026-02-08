use ratatui::buffer::Cell;
use ratatui::style::Color;

use crate::character::Character;
use crate::global_flags;
use crate::map::objects::container::Container;
use crate::map::tile::TileDetails;
use crate::terminal::colour_mapper;

#[derive(Clone)]
#[derive(Debug)]
pub struct CellBuilder {
}

impl CellBuilder {
    pub fn from_tile(tile_details: &TileDetails) -> Cell {
        let symbol = tile_details.symbol.character.to_string();
        let fg = colour_mapper::map_colour(tile_details.symbol.colour);
        let bg = ratatui::style::Color::Black;
        let _modifier = ratatui::style::Modifier::empty();
        
        let mut cell = Cell::default();
        cell.set_symbol(&*symbol);
        cell.set_bg(bg);
        cell.set_fg(fg);
        cell
    }

    pub fn from_character(character: &Character) -> Cell{
        let character_colour = character.get_colour();
        let symbol = character.get_symbol().to_string();
        let fg = colour_mapper::map_colour(character_colour);
        let bg = ratatui::style::Color::Black;
        let mut cell = Cell::default();
        cell.set_symbol(&*symbol);
        cell.set_bg(bg);
        cell.set_fg(fg);
        cell
    }

    pub fn from_container(container: &Container) -> Cell{
        let container_item = container.get_self_item();
        let symbol = container_item.symbol.character.to_string();
        let colour = container_item.symbol.colour;
        let fg = colour_mapper::map_colour(colour);
        let bg = ratatui::style::Color::Black;
        let mut cell = Cell::default();
        cell.set_symbol(&*symbol);
        cell.set_bg(bg);
        cell.set_fg(fg);
        cell
    }

    pub fn for_blank() -> Cell {
        if global_flags::GLOBALS.debugging_map_symbols {
            let debugging_symbol = String::from('\u{2588}');
            let mut debugging_blank = Cell::default();
            debugging_blank.set_symbol(&*debugging_symbol);
            debugging_blank.set_bg(Color::Black);
            debugging_blank.set_fg(Color::Green);
            debugging_blank
        } else {
            let mut blank = Cell::new(" ");
            blank.set_bg(Color::Black);
            blank.set_fg(Color::Black);
            blank
        }
    }
}