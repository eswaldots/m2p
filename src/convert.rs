use pulldown_cmark::Parser;

// simple struct to add more pdf rendering engines in a future
pub trait Convert {
    fn convert(events: Parser) -> Vec<u8>;
}
