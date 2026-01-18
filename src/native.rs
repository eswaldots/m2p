use crate::convert::Convert;
use genpdf::Document;
use pulldown_cmark::{Event, HeadingLevel, Parser, Tag, TagEnd};

pub struct NativePDFConvert {}

impl Convert for NativePDFConvert {
    fn convert(events: Parser) -> Vec<u8> {
        let font_family =
            genpdf::fonts::from_files("/usr/share/fonts/Adwaita/", "AdwaitaMono", None)
                .expect("Failed to load font family");

        let mut doc = Document::new(font_family);
        doc.set_minimal_conformance();
        doc.set_line_spacing(1.25);

        let mut decorator = genpdf::SimplePageDecorator::new();
        decorator.set_margins(10);

        doc.set_page_decorator(decorator);

        for event in events {
            match event {
                Event::Start(Tag::Heading { level, .. }) => {
                    let size = match level {
                        HeadingLevel::H1 => 20,
                        _ => 18,
                    };

                    doc.set_font_size(size);
                }
                Event::Text(text) => {
                    doc.push(genpdf::elements::Paragraph::new(text.into_string()));
                }
                Event::End(tag) => {
                    match tag {
                        TagEnd::Heading(_) => {
                            doc.set_font_size(12);
                        }
                        TagEnd::Paragraph => {}
                        _ => (),
                    };

                    doc.push(genpdf::elements::Break::new(1));
                }
                _ => (),
            }
        }

        let mut w = Vec::new();

        doc.render(&mut w);

        w
    }
}
