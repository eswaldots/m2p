#ifndef YAML_H
#define YAML_H

/// Removes YAML frontmatter in the buffer, if doesn't exists, doesn't
/// mutate the buffer
int yaml_parse(char *buffer);
#endif
