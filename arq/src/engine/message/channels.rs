use tokio::sync::mpsc::{Receiver, Sender};
/*
    A channel that allows sending and receiving messages
 */
// pub struct MessageChannels<RQ,RS> {
//     pub request_channel: Channel<RQ>,
//     pub response_channel: Channel<RS>   
// }
// 
// impl<RQ, RS> MessageChannels<RQ, RS> {
//     pub fn new() -> MessageChannels<RQ,RS> {
//         let request_channel = tokio::sync::mpsc::channel(2);
//         let response_channel =  tokio::sync::mpsc::channel(2);
//         
//         MessageChannels {
//             request_channel: Channel {
//                 sender: request_channel.0,
//                 receiver: request_channel.1
//             },
//             response_channel: Channel {
//                 sender: response_channel.0,
//                 receiver: response_channel.1
//             },
//         }
//         
//     }
// }

pub struct Channel<T> {
    pub sender: Sender<T>,
    pub receiver: Receiver<T>
}
