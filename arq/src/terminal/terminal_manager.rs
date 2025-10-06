use std::io;

use ratatui::backend::{Backend, CrosstermBackend, TestBackend};
use ratatui::Terminal;
use termion::raw::{IntoRawMode, RawTerminal};

use crate::ui::resolution::Resolution;

pub struct TerminalManager<B : Backend> {
    pub terminal : ratatui::Terminal<B>,
}

impl <B : Backend>  TerminalManager<B> {
    pub fn clear_screen(&mut self) -> Result<(), io::Error> {
        self.terminal.clear()
    }
}

pub fn init() -> Result<TerminalManager<CrosstermBackend<RawTerminal<io::Stdout>>>, io::Error> {
    let stdout = io::stdout().into_raw_mode()?;
    let backend = CrosstermBackend::new(stdout);
    let terminal = Terminal::new(backend)?;
    let manager = TerminalManager::<CrosstermBackend<RawTerminal<io::Stdout>>> { terminal };

    log::info!("Terminal initialised.");
    return Ok(manager);
}

pub fn init_test(resolution : Resolution) -> Result<TerminalManager<TestBackend>, io::Error> {
    let backend = TestBackend::new(resolution.width,resolution.height);
    let terminal = Terminal::new(backend)?;
    let manager = TerminalManager::<TestBackend> { terminal };

    log::info!("Terminal initialised.");
    return Ok(manager);
}
