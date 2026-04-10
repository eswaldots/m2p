#include "m2p.h"
#include "util.h"
#include <string.h>

int yaml_parse(char *buffer) {
        if (strncmp(buffer, "---", 3) == 0) {
                m2p_printf(
                    M2P_LOG_DEBUG,
                    "Markdown file contains YAML frontmatter, removing...");

                return 1;
        } else {
                return 0;
        };
}
