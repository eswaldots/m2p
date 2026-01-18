use crate::{cli::Opts, convert::Convert, native::NativePDFConvert};
use anyhow::Result;
use clap::Parser;
use pulldown_cmark::Parser as MDParser;
use std::{fs, io::Write};

mod cli;
mod convert;
mod native;

fn main() -> Result<()> {
    init_logger();

    let opts = Opts::parse();

    let data = fs::read_to_string(&opts.input).expect("There is no file");

    let events = MDParser::new(&data);

    let bytes = NativePDFConvert::convert(events);

    let mut file = fs::File::create(opts.output)?;

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
