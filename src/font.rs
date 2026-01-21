use anyhow::Result;
use font_kit::family_name::FamilyName;
use font_kit::properties::{Properties, Style, Weight};
use font_kit::source::SystemSource;
use std::borrow::Cow;

pub struct ResolvedFont {
    pub regular: Cow<'static, [u8]>,
    pub bold: Cow<'static, [u8]>,
    pub italic: Cow<'static, [u8]>,
    pub bold_italic: Cow<'static, [u8]>,
}

pub struct FontResolver;

impl FontResolver {
    pub fn new() -> Self {
        Self
    }

    pub fn resolve(&self, pattern: String) -> Result<ResolvedFont> {
        let mut families = Vec::new();
        for family in pattern.split(',') {
            let name = family.replace('\'', "").trim().to_string();
            families.push(match name.as_str() {
                "serif" => FamilyName::Serif,
                "sans-serif" => FamilyName::SansSerif,
                "monospace" => FamilyName::Monospace,
                _ => FamilyName::Title(name),
            });
        }

        let source = SystemSource::new();
        let builtin = self.resolve_builtin()?;

        let load =
            |w: Weight, s: Style, fallback: Cow<'static, [u8]>| -> Result<Cow<'static, [u8]>> {
                let h =
                    source.select_best_match(&families, &Properties::new().weight(w).style(s))?;

                Ok(h.load()
                    .map(|f| Cow::Owned(f.copy_font_data().unwrap().to_vec()))
                    .unwrap_or(fallback))
            };

        Ok(ResolvedFont {
            regular: load(Weight::NORMAL, Style::Normal, builtin.regular)?,
            bold: load(Weight::BOLD, Style::Normal, builtin.bold)?,
            italic: load(Weight::NORMAL, Style::Italic, builtin.italic)?,
            bold_italic: load(Weight::BOLD, Style::Italic, builtin.bold_italic)?,
        })
    }

    pub fn resolve_builtin(&self) -> Result<ResolvedFont> {
        Ok(ResolvedFont {
            regular: Cow::Borrowed(include_bytes!("../assets/fonts/Inter-Regular.ttf")),
            bold: Cow::Borrowed(include_bytes!("../assets/fonts/Inter-Bold.ttf")),
            italic: Cow::Borrowed(include_bytes!("../assets/fonts/Inter-Italic.ttf")),
            bold_italic: Cow::Borrowed(include_bytes!("../assets/fonts/Inter-BoldItalic.ttf")),
        })
    }
}
