pub mod audio;
pub mod data;
pub mod logging;
pub mod sockets;

#[cfg(not(target_os = "android"))]
pub mod commands;
#[cfg(not(target_os = "android"))]
pub mod ffmpeg;
#[cfg(not(target_os = "android"))]
pub mod graphics;

pub mod prelude {
    pub use crate::{
        fmt_e,
        logging::{log_event, Event, StrResult},
        trace_err, trace_none, trace_str,
    };
    pub use log::{debug, error, info, warn};
}

////////////////////////////////////////////////////////

use prelude::*;
use std::future::Future;
use tokio::{sync::oneshot, task};

// Tokio tasks are not cancelable. This function awaits a cancelable task.
pub async fn spawn_cancelable(
    future: impl Future<Output = StrResult> + Send + 'static,
) -> StrResult {
    // this channel is actually never used. cancel_receiver will be notified when _cancel_sender is
    // dropped
    let (_cancel_sender, cancel_receiver) = oneshot::channel::<()>();

    trace_err!(
        task::spawn(async {
            tokio::select! {
                res = future => res,
                _ = cancel_receiver => Ok(()),
            }
        })
        .await
    )?
}
