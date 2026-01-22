use anyhow::Result;
use std::borrow::Cow;

use crate::yaml::YAMLReader;

pub struct Metadata {
    pub title: String,
}

impl Metadata {
    pub fn parse<'a>(input: &'a str, path: &'a str) -> Result<(Self, Cow<'a, str>)> {
        if let Some((matter, content)) = matter::matter(input) {
            let reader = YAMLReader::new(&matter);

            if let Some(id) = reader.get("id") {
                return Ok((
                    Metadata {
                        title: id.to_string(),
                    },
                    Cow::from(content),
                ));
            }

            return Ok((
                Metadata {
                    title: "no id".to_string(),
                },
                Cow::from(content),
            ));
        }

        let title = input.lines().next();

        match title {
            Some(title) => {
                return Ok((
                    Metadata {
                        title: title.replace("#", "").to_owned(),
                    },
                    Cow::from(input),
                ));
            }
            None => {
                return Ok((
                    Metadata {
                        title: path.to_owned(),
                    },
                    Cow::from(input),
                ));
            }
        }
    }
}
