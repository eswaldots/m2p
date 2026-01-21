use anyhow::Result;
use pulldown_cmark::Parser;

use crate::{font::ResolvedFont, metadata::Metadata};

// simple struct with pulldown_cmark types to add more pdf rendering engines in a future
pub trait PDFBuilder {
    fn build(events: Parser, font: ResolvedFont, metadata: Metadata) -> Result<Vec<u8>>;
}
