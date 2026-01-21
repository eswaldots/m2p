use crate::font::FontResolver;
use crate::native::NativePDFBuilder;
use crate::{builder::PDFBuilder, cli::Opts};
use anyhow::Result;
use clap::Parser;
use pulldown_cmark::Parser as MDParser;
use std::time::Instant;
use std::{fs, io::Write};
use tracing::info;

mod builder;
mod cli;
mod font;
mod metadata;
mod native;
mod yaml;

fn main() -> Result<()> {
    init_logger();

    let opts = Opts::parse();

    let data = fs::read_to_string(&opts.input).expect("There is no file");

    let resolver = FontResolver::new();

    let start = Instant::now();

    let font = match opts.font_family {
        Some(pattern) => resolver.resolve(pattern)?,
        _ => resolver.resolve_builtin()?,
    };

    let (metadata, content) = metadata::Metadata::parse(&data)?;

    let events = MDParser::new(&content);

    info!("Rendering markdown into pdf...");

    let bytes = NativePDFBuilder::build(events, font, metadata)?;

    let elapsed = start.elapsed();

    info!("Rendering completed in {:?}", elapsed);

    let mut file = fs::File::create(opts.output.unwrap_or(opts.input.replace(".md", ".pdf")))?;

    file.write_all(&bytes)?;

    Ok(())
}

fn init_logger() {
    let filter = tracing_subscriber::EnvFilter::from_env("M2P_LOG");
    let log_env = std::env::var("M2P_LOG");

    let with_target = log_env.is_ok();

    tracing_subscriber::fmt()
        .without_time()
        // i dont think some person will use this crate without ansi support, wtf
        .with_ansi(true)
        .with_env_filter(filter)
        .with_target(with_target)
        .init();
}
