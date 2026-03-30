#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *change_ext(char *filename) {
  int i = strlen(filename);
  int j;

  char *buffer;

  buffer = malloc(i + 1);

  // TODO: maybe the file doesn't have a extension or the path haves too much
  // dots...
  for (j = 0; filename[j] != '.'; ++j) {
    buffer[j] = filename[j];
  }

  buffer[j] = '.';
  buffer[j + 1] = 'p';
  buffer[j + 2] = 'd';
  buffer[j + 3] = 'f';
  buffer[j + 4] = '\0';

  return buffer;
}
