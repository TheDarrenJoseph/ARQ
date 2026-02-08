/*
Based on example code from https://github.com/ratatui/templates

The MIT License (MIT)

Copyright (c) 2021-2022 Orhun Parmaksiz
Copyright (c) 2023 The Ratatui Developers

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */
use crate::engine::event::container::OpenedContainerEventData;
use crate::engine::event::container::OpenedContainerEventType;
use log::{debug, error, info};
use std::time::Duration;
use termion::input::TermRead;
use tokio::sync::mpsc;
use tokio::task::JoinHandle;
use tokio_util::sync::CancellationToken;

#[derive(Debug)]
pub struct TerminalEventHandler {
    pub(crate) sender: mpsc::UnboundedSender<UIEvent>,
    pub(crate) receiver: mpsc::UnboundedReceiver<UIEvent>,
}

pub struct EventTask {
    cancellation_token: CancellationToken,
    sender: mpsc::UnboundedSender<UIEvent>,
}

#[derive(Debug, Clone)]
pub enum UIEvent {
    /// An event that is emitted on a regular schedule.
    ///
    /// Use this event to run any code which has to run outside of being a direct response to a user
    /// event. e.g. polling exernal systems, updating animations, or rendering the UI based on a
    /// fixed frame rate.
    Tick,
    // TODO migrate to crossterm events
    Termion(termion::event::Event),
    AppEvent(AppEventType)
}

#[derive(Debug, Clone)]
pub enum AppEventType {
    OpenedContainerEvent(OpenedContainerEventType, Option<OpenedContainerEventData>)
}

impl TerminalEventHandler {
    pub fn new() -> TerminalEventHandler {
        let (sender, receiver) = mpsc::unbounded_channel();
        TerminalEventHandler { sender, receiver }
    }

    pub fn spawn_thread(&self) -> TerminalEventThreadData {
        let cancellation_token = CancellationToken::new();

        info!("Spawning event handler thread");
        let task = EventTask::new(
            cancellation_token.clone(),
            self.sender.clone()
        );
        let join_handle = tokio::spawn(async { task.run().await });

        TerminalEventThreadData {
            cancellation_token,
            join_handle
        }
    }
}

pub struct TerminalEventThreadData {
    pub cancellation_token: CancellationToken,
    pub join_handle: JoinHandle<()>
}

impl EventTask {
    pub fn new(cancellation_token: CancellationToken, sender: mpsc::UnboundedSender<UIEvent>) -> Self {
        Self {
            cancellation_token,
            sender
        }
    }

    pub(crate) async fn run(self) {
        let tick_rate = Duration::from_secs_f64(0.1f64);
        let mut tick = tokio::time::interval(tick_rate);
        let mut events = termion::async_stdin().events();

        loop {
            if self.cancellation_token.is_cancelled() {
                info!("Event Task cancelled");
                return;
            }

            if let Some(e) = events.next() {
                match e {
                    Ok(e) => {
                        if self.sender.is_closed() {
                            break;
                        }
                        debug!("Sending Termion event: {:?}", e);
                        self.send(UIEvent::Termion(e));
                    },
                    Err(e) => {
                        error!("Error reading from stdin: {}", e);
                        break;
                    }
                }
            }
            // wait for the next tick before trying to read events
            tick.tick().await;
            //debug!("TICK");
            // self.send(Event::Tick);
            // debug!("TOCK");
        }
    }

    fn send(&self, event: UIEvent) {
        let _ = self.sender.send(event);
    }
}
