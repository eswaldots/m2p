use std::borrow::Cow;

use tracing::{debug, info};

// a very minimal simple yaml reader
pub struct YAMLReader<'a> {
    input: Cow<'a, str>,
}

impl<'a> YAMLReader<'a> {
    pub fn new<'b>(input: &'a str) -> YAMLReader<'a> {
        YAMLReader {
            input: Cow::from(input),
        }
    }
    pub fn get<'b>(&'a self, key: &str) -> Option<Cow<'a, str>> {
        for line in self.input.lines() {
            if let Some(index) = line.find(':') {
                let (key_value, value_part) = line.split_at(index);

                // TODO: check the key at the beginning of the function, not at the end, dumbass
                if key.contains(key_value) {
                    return Some(Cow::Borrowed(value_part[1..].trim()));
                }
            }

            continue;
        }

        return None;
    }
}
