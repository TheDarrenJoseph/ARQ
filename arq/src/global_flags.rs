use termion::event::Key;

pub struct GlobalTestFlags {
    pub debugging_map_symbols: bool
}

pub const GLOBALS: GlobalTestFlags = GlobalTestFlags {
    // This forces the use of the block character (█) in bright green
    // For any explicit map view blanking, and for map cells outside of the actual map range
    debugging_map_symbols: false
};

pub const ENTER_KEY: Key = Key::Char('\n');