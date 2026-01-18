use clap::Parser;

#[derive(Parser)]
#[command(
    name = "m2p",
    version,
    about = "Blazingly fast Markdown to PDF converter",
    after_long_help = "Bugs can be reported on GitHub: https://github.com/eswaldots/m2p/issues",
    max_term_width = 98,
    args_override_self = true
)]
pub struct Opts {
    #[arg(
        value_name = "INPUT",
        help = "Markdown file to convert ('-' for stdin)"
    )]
    pub input: String,
    #[arg(value_name = "OUTPUT", help = "PDF file path output")]
    pub output: String,
}
