#![allow(clippy::missing_safety_doc, clippy::redundant_static_lifetimes)]

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));
pub use root::vr::*;
pub use root::*;

include!(concat!(env!("OUT_DIR"), "/properties_mappings.rs"));
