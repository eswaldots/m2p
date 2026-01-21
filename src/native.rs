use crate::{
    builder::PDFBuilder,
    metadata::{self, Metadata},
};
use anyhow::Result;
use genpdf::{
    elements::{Break, Paragraph},
    fonts::FontData,
    style::{Style, StyledString},
    *,
};
use pulldown_cmark::{Event, HeadingLevel, Parser, Tag, TagEnd};
use std::io::BufWriter;

pub struct NativePDFBuilder {}

impl PDFBuilder for NativePDFBuilder {
    fn build(
        events: Parser,
        font: crate::font::ResolvedFont,
        metadata: Metadata,
    ) -> Result<Vec<u8>> {
        let font_family = genpdf::fonts::FontFamily {
            regular: FontData::new(font.regular.into_owned(), None)?,
            italic: FontData::new(font.italic.into_owned(), None)?,
            bold: FontData::new(font.bold.into_owned(), None)?,
            bold_italic: FontData::new(font.bold_italic.into_owned(), None)?,
        };

        let mut doc = Document::new(font_family);

        doc.set_title(metadata.title);
        doc.set_line_spacing(1.25);

        let mut decorator = SimplePageDecorator::new();

        decorator.set_margins(20);

        doc.set_page_decorator(decorator);

        // TODO: move this to the struct
        let mut style_stack = vec![Style::new()];
        let mut text_buffer: Vec<StyledString> = Vec::with_capacity(32);
        let mut current_block_style = Style::new();

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
                    Tag::Strong => style_stack.push(style_stack.last().unwrap().bold()),
                    Tag::Emphasis => style_stack.push(style_stack.last().unwrap().italic()),
                    _ => {}
                },

                Event::Text(text) => {
                    if let Some(style) = style_stack.last() {
                        text_buffer.push(StyledString::new(text, *style));
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
                    TagEnd::Strong | TagEnd::Emphasis => {
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
