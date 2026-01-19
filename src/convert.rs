use anyhow::Result;
use pulldown_cmark::Parser;

use crate::font::ResolvedFont;

// simple struct to add more pdf rendering engines in a future
pub trait Convert {
    fn convert(events: Parser, font: ResolvedFont) -> Result<Vec<u8>>;
}
