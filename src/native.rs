use crate::{
    convert::Convert,
    metadata::{self, Metadata},
};
use anyhow::Result;
use genpdf::{fonts::FontData, style::Style, *};
use pulldown_cmark::{Event, HeadingLevel, MetadataBlockKind, Parser, Tag, TagEnd};
use tracing::info;

#[derive(Copy, Clone)]
enum FontSize {
    Body,
    Heading1,
    Heading2,
}

impl Into<u8> for FontSize {
    fn into(self) -> u8 {
        match self {
            FontSize::Heading1 => 18,
            FontSize::Heading2 => 16,
            FontSize::Body => 12,
        }
    }
}

pub struct NativePDFConvert {}

impl Convert for NativePDFConvert {
    fn convert(
        events: Parser,
        font: crate::font::ResolvedFont,
        metadata: Metadata,
    ) -> Result<Vec<u8>> {
        info!("Using native pdf rendering engine");

        let font_data = font;

        let font_family = genpdf::fonts::FontFamily {
            regular: FontData::new(font_data.regular.into_owned(), None)?,
            italic: FontData::new(font_data.italic.into_owned(), None)?,
            bold: FontData::new(font_data.bold.into_owned(), None)?,
            bold_italic: FontData::new(font_data.bold_italic.into_owned(), None)?,
        };

        let mut doc = Document::new(font_family);
        doc.set_minimal_conformance();
        doc.set_line_spacing(1.25);

        let mut decorator = genpdf::SimplePageDecorator::new();
        decorator.set_margins(20);

        doc.set_page_decorator(decorator);
        doc.set_title(metadata.title);

        // variable to track the font size
        let mut current_style = Style::new().with_font_size(FontSize::Body.into());
        // temporary text buffer
        let mut s = String::new();

        for event in events {
            match event {
                Event::TaskListMarker(checked) => {
                    doc.push(genpdf::elements::Paragraph::new("checked"));
                }
                Event::Start(tag) => {
                    match tag {
                        Tag::Heading { level, .. } => {
                            current_style = Style::new();

                            let size = match level {
                                HeadingLevel::H1 => {
                                    current_style = current_style.bold();

                                    FontSize::Heading1
                                }
                                HeadingLevel::H2 => {
                                    current_style = current_style.bold();

                                    FontSize::Heading2
                                }
                                _ => FontSize::Body,
                            };

                            current_style = current_style.with_font_size(size.into());
                        }
                        e => {
                            dbg!("unknown event: {e}", e);
                        }
                    };
                }
                Event::Text(text) => {
                    dbg!(&text);
                    doc.push(genpdf::elements::Paragraph::new(&*text).styled(current_style));
                }
                Event::End(tag) => {
                    match tag {
                        TagEnd::Heading(_) => {}
                        TagEnd::Paragraph => {}
                        _ => (),
                    };

                    current_style = Style::new().with_font_size(FontSize::Body.into());
                }
                e => {
                    dbg!("unkwnown tag: {}", e);
                }
            }
        }

        let mut w = Vec::new();

        // TODO: this is slow by a reason
        doc.render(&mut w)?;

        Ok(w)
    }
}
