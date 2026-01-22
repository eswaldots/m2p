use crate::{builder::PDFBuilder, fonts::Fonts, metadata::Metadata, native::code::CodeBlock};
use anyhow::Result;
use genpdf::{
    elements::{Break, CellDecorator, FrameCellDecorator, PaddedElement, Paragraph},
    fonts::FontData,
    render::TextSection,
    style::{Style, StyledString},
    *,
};
use pulldown_cmark::{Event, HeadingLevel, Parser, Tag, TagEnd};
use std::io::BufWriter;

pub struct NativePDFBuilder {}

impl PDFBuilder for NativePDFBuilder {
    fn build(events: Parser, fonts: Fonts, metadata: Metadata) -> Result<Vec<u8>> {
        // TODO: use a Into<T> trait abstraction to perform this convertion
        let sans = genpdf::fonts::FontFamily {
            regular: FontData::new(fonts.sans.regular.into_owned(), None)?,
            italic: FontData::new(fonts.sans.italic.into_owned(), None)?,
            bold: FontData::new(fonts.sans.bold.into_owned(), None)?,
            bold_italic: FontData::new(fonts.sans.bold_italic.into_owned(), None)?,
        };

        let mono = genpdf::fonts::FontFamily {
            regular: FontData::new(fonts.monospace.regular.into_owned(), None)?,
            italic: FontData::new(fonts.monospace.italic.into_owned(), None)?,
            bold: FontData::new(fonts.monospace.bold.into_owned(), None)?,
            bold_italic: FontData::new(fonts.monospace.bold_italic.into_owned(), None)?,
        };

        let mut doc = Document::new(sans);

        let mono = doc.add_font_family(mono);
        doc.set_title(metadata.title);
        doc.set_line_spacing(1.25);

        let mut decorator = SimplePageDecorator::new();

        decorator.set_margins(20);

        doc.set_page_decorator(decorator);

        // TODO: move this to the struct
        let mut style_stack = vec![Style::new()];
        let mut text_buffer: Vec<StyledString> = Vec::with_capacity(32);
        let mut current_block_style = Style::new();
        let mut lang: Option<String> = None;

        for event in events {
            match event {
                Event::Start(tag) => match tag {
                    Tag::Paragraph => {
                        text_buffer.clear();
                        current_block_style = Style::new()
                    }
                    Tag::Heading { level, .. } => {
                        text_buffer.clear();
                        let size = match level {
                            HeadingLevel::H1 => 18,
                            HeadingLevel::H2 => 16,
                            _ => 14,
                        };
                        current_block_style = Style::new().bold().with_font_size(size);
                        style_stack.push(Style::new().bold().with_font_size(size));
                    }
                    Tag::CodeBlock(kind) => match kind {
                        pulldown_cmark::CodeBlockKind::Indented => {
                            dbg!(kind);
                            style_stack.push(style_stack.last().unwrap().with_font_family(mono));
                        }
                        pulldown_cmark::CodeBlockKind::Fenced(code) => {
                            lang = Some(code.into());

                            doc.push(Break::new(1));
                        }
                    },
                    Tag::Strong => style_stack.push(style_stack.last().unwrap().bold()),
                    Tag::Emphasis => style_stack.push(style_stack.last().unwrap().italic()),
                    _ => {}
                },
                Event::Text(text) => {
                    if let Some(code) = lang {
                        let block = CodeBlock::new(&text, mono, code);

                        doc.push(block);

                        lang = None;
                        continue;
                    }
                    if let Some(style) = style_stack.last() {
                        text_buffer.push(StyledString::new(text, *style));
                    }
                }
                Event::Code(text) => {
                    if let Some(style) = style_stack.last() {
                        text_buffer.push(StyledString::new(text, style.with_font_family(mono)));
                    }
                }
                Event::End(tag) => match tag {
                    TagEnd::Paragraph | TagEnd::Heading(_) => {
                        let mut p = Paragraph::new("");
                        for span in text_buffer.drain(..) {
                            p.push(span);
                        }
                        doc.push(p.styled(current_block_style));
                        doc.push(Break::new(1));

                        if matches!(tag, TagEnd::Heading(_)) {
                            style_stack.pop();
                        }
                    }
                    // TODO: not pop in fenced codeblock
                    TagEnd::Strong | TagEnd::Emphasis | TagEnd::CodeBlock => {
                        style_stack.pop();
                    }
                    _ => {}
                },

                _ => {}
            }
        }

        let mut w = BufWriter::with_capacity(1024 * 64, Vec::new());
        doc.render(&mut w)?;

        let result = w.into_inner().map_err(|e| anyhow::anyhow!(e))?;
        Ok(result)
    }
}

mod code {
    use std::{
        rc::Rc,
        sync::{Arc, Mutex},
    };

    use genpdf::{
        Element, Margins, Mm, Position, RenderResult,
        elements::{Break, Paragraph},
        fonts::{Font, FontFamily},
        render::TextSection,
        style::{Color, Style, StyledString},
    };
    use syntect::{
        easy::HighlightLines,
        highlighting::{Style as SyntectStyle, ThemeSet},
        parsing::SyntaxSet,
        util::{LinesWithEndings, as_24_bit_terminal_escaped},
    };

    #[derive(Debug, Clone)]
    pub struct CodeBlock {
        text: String,
        font_family: FontFamily<Font>,
        lang: String,
    }

    impl CodeBlock {
        pub fn new(text: &str, font_family: FontFamily<Font>, lang: String) -> Self {
            CodeBlock {
                text: text.to_owned(),
                font_family,
                lang,
            }
        }
    }

    impl Element for CodeBlock {
        fn render(
            &mut self,
            context: &genpdf::Context,
            mut area: genpdf::render::Area<'_>,
            style: genpdf::style::Style,
        ) -> Result<genpdf::RenderResult, genpdf::error::Error> {
            let area = Arc::new(area);
            let mut result = RenderResult::default();

            let margins = Margins::all(1.0);

            let b_result = Break::new(1).render(context, Arc::clone(&area).into(), style)?;

            // TODO: add a function to simplify this syntax
            result.size.width += b_result.size.width;
            result.size.height += b_result.size.height;

            area.add_margins(margins);

            let ps = SyntaxSet::load_defaults_newlines();
            let ts = ThemeSet::load_defaults();

            let syntax = ps.find_syntax_by_token(&self.lang).unwrap();
            let mut h = HighlightLines::new(syntax, &ts.themes["base16-ocean.light"]);
            let mut buffers: Vec<Vec<StyledString>> = Vec::new();

            for line in LinesWithEndings::from(&self.text) {
                let ranges: Vec<(SyntectStyle, &str)> = h.highlight_line(line, &ps).unwrap();
                let mut line_buf: Vec<StyledString> = Vec::new();

                for range in ranges.iter() {
                    line_buf.push(StyledString::new(
                        range.1,
                        Style::new()
                            .with_color(Color::Rgb(
                                range.0.foreground.r,
                                range.0.foreground.g,
                                range.0.foreground.b,
                            ))
                            .with_font_family(self.font_family),
                    ));
                }

                buffers.push(line_buf);
            }

            for mut buf in buffers {
                let mut p = Paragraph::new("");

                for span in buf.drain(..) {
                    p.push(span);
                }

                let p_result = p.render(context, area.clone(), style)?;

                result.size.width += p_result.size.width;

                result.size.height += p_result.size.height;

                let b_result = Break::new(1).render(context, area.clone(), style)?;

                // TODO: add a function to simplify this syntax
                result.size.width += b_result.size.width;
                result.size.height += b_result.size.height;
            }

            let b_result = Break::new(1).render(context, area.clone(), style)?;

            // TODO: add a function to simplify this syntax
            result.size.width += b_result.size.width;
            result.size.height += b_result.size.height;

            result.size.width += 2.0.into();
            result.size.height += 2.0.into();

            Ok(result)
        }
    }
}
