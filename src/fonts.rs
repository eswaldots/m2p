use anyhow::Result;
use font_kit::family_name::FamilyName;
use font_kit::properties::{Properties, Style, Weight};
use font_kit::source::SystemSource;
use std::borrow::Cow;

pub struct Fonts {
    pub monospace: ResolvedFont,
    pub sans: ResolvedFont,
}

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

    pub fn resolve(&self, pattern: String) -> Result<Fonts> {
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

        let sans = ResolvedFont {
            regular: load(Weight::NORMAL, Style::Normal, builtin.sans.regular)?,
            bold: load(Weight::BOLD, Style::Normal, builtin.sans.bold)?,
            italic: load(Weight::NORMAL, Style::Italic, builtin.sans.italic)?,
            bold_italic: load(Weight::BOLD, Style::Italic, builtin.sans.bold_italic)?,
        };

        // TODO: add flag for custom monospace fonts
        let monospace = ResolvedFont {
            regular: load(Weight::NORMAL, Style::Normal, builtin.monospace.regular)?,
            bold: load(Weight::BOLD, Style::Normal, builtin.monospace.bold)?,
            italic: load(Weight::NORMAL, Style::Italic, builtin.monospace.italic)?,
            bold_italic: load(Weight::BOLD, Style::Italic, builtin.monospace.bold_italic)?,
        };

        Ok(Fonts { monospace, sans })
    }

    pub fn resolve_builtin(&self) -> Result<Fonts> {
        let sans = ResolvedFont {
            regular: Cow::Borrowed(include_bytes!("../assets/fonts/Inter-Regular.ttf")),
            bold: Cow::Borrowed(include_bytes!("../assets/fonts/Inter-Bold.ttf")),
            italic: Cow::Borrowed(include_bytes!("../assets/fonts/Inter-Italic.ttf")),
            bold_italic: Cow::Borrowed(include_bytes!("../assets/fonts/Inter-BoldItalic.ttf")),
        };

        let monospace = ResolvedFont {
            regular: Cow::Borrowed(include_bytes!("../assets/fonts/AdwaitaMono-Regular.ttf")),
            bold: Cow::Borrowed(include_bytes!("../assets/fonts/AdwaitaMono-Bold.ttf")),
            italic: Cow::Borrowed(include_bytes!("../assets/fonts/AdwaitaMono-Italic.ttf")),
            bold_italic: Cow::Borrowed(include_bytes!(
                "../assets/fonts/AdwaitaMono-BoldItalic.ttf"
            )),
        };

        Ok(Fonts { monospace, sans })
    }
}
