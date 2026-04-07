#include "util.h"
#include "m2p.h"
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void change_ext(char *filename, char *buffer) {

        char *match = strstr(filename, ".md");

        if (match) {
                m2p_printf(M2P_LOG_DEBUG, "Filename contains a .md extension, "
                                          "proceed to modifying...\n");

                int i = strlen(filename);

                m2p_printf(M2P_LOG_DEBUG,
                           "Copying values from filename to buffer...\n");

                strcpy(buffer, filename);

                buffer[i + 1] = '\0';

                buffer[i - 2] = 'p';
                buffer[i - 1] = 'd';
                buffer[i] = 'f';

                m2p_printf(
                    M2P_LOG_DEBUG,
                    "Buffer overrided, extension modified successfully.\n");
        } else {
                m2p_printf(M2P_LOG_ERROR,
                           "This file doesn't contain a .md extension, ensure "
                           "this is a markdown file before following\n");

                exit(-1);
        }
}

m2p_log_level get_log_level(void) {
        char *logmask = getenv("M2P_LOG");

        if (logmask == NULL) {
                return M2P_LOG_ERROR;
        }

        if (strcmp(logmask, "INFO") == 0) {
                return M2P_LOG_INFO;
        } else if (strcmp(logmask, "ERROR") == 0) {
                return M2P_LOG_ERROR;
        } else if (strcmp(logmask, "WARNING") == 0) {
                return M2P_LOG_WARNING;
        } else if (strcmp(logmask, "DEBUG") == 0) {
                return M2P_LOG_DEBUG;
        } else {
                return M2P_LOG_ERROR;
        }

        free(logmask);
}

#define ANSI_BLUE "\033[0;34m"
#define ANSI_GREEN "\033[0;32m"
#define ANSI_RED "\033[0;31m"
#define ANSI_YELLOW "\033[0;33m"
#define ANSI_RESET "\033[0m"

int m2p_printf(m2p_log_level level, const char *format, ...) {
        int ret = 0;

        m2p_log_level logmask = get_log_level();

        // printf("i not understand\n");
        if (logmask < level) {
                return ret;
        }

        char *msg = NULL;

        va_list args;

        va_start(args, format);
        ret = vasprintf(&msg, format, args);
        va_end(args);

        switch (level) {
        case M2P_LOG_DEBUG:
                printf(ANSI_BLUE "DEBUG:" ANSI_RESET " ");
                break;
        case M2P_LOG_INFO:
                printf(ANSI_GREEN "INFO:" ANSI_RESET " ");

                break;
        case M2P_LOG_WARNING:
                printf(ANSI_YELLOW "WARNING:" ANSI_RESET " ");

                break;
        case M2P_LOG_ERROR:
                printf(ANSI_RED "ERROR:" ANSI_RESET " ");
                break;
        default:
                break;
        }

        ret = printf("%s", msg);

        // TODO: why freed?
        free(msg);

        return ret;
}
