use anyhow::Result;
use pulldown_cmark::Parser;

use crate::{font::ResolvedFont, metadata::Metadata};

// simple struct to add more pdf rendering engines in a future
pub trait Convert {
    fn convert(events: Parser, font: ResolvedFont, metadata: Metadata) -> Result<Vec<u8>>;
}
