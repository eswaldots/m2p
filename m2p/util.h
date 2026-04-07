#ifndef UTIL_H

#define UTIL_H
#include "m2p.h"
void change_ext(char *filename, char *buffer);

int m2p_printf(m2p_log_level level, const char *format, ...);
#endif
