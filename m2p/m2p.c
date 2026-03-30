#include "m2p.h"
#include "../include/libpdf.h"
#include "md4c.h"
#include <stdio.h>
#include <string.h>

#include "util.h"

int HandleText(MD_TEXTTYPE type, const MD_CHAR *text, MD_SIZE size,
               void *userdata) {
  char *buffer = malloc(strlen(text));

  strcpy(buffer, text);

  buffer[size] = '\0';
  printf("DEBUG: Writing buffer to output: '%s'", buffer);
  printf("with size '%d'\n", size);

  WriteText(buffer);

  return 0;
}
int EnterBlock(MD_BLOCKTYPE type, void *detail, void *userdata) {
  if (type == MD_BLOCK_H) {
    MD_BLOCK_H_DETAIL *size = (MD_BLOCK_H_DETAIL *)detail;

    if (size->level == 1) {
      // TODO: add kerning
      SetFontTypeAndSize(HEADING_H1_SIZE, FONT_BOLD);
    }
  }

  return 0;
};
int LeaveBlock(MD_BLOCKTYPE type, void *detail, void *userdata) {
  printf("DEBUG: hard break\n");
  WriteHardBreak();

  // TODO: implement a style stack
  SetFontTypeAndSize(P_SIZE, FONT_REGULAR);

  return 0;
};

int EnterSpan(MD_SPANTYPE type, void *detail, void *userdata) { return 0; };

int LeaveSpan(MD_SPANTYPE type, void *detail, void *userdata) {
  printf("DEBUG: leaving span\n");
  WriteHardBreak();

  return 0;
};

int main(int argc, char **argv) {
  printf("DEBUG: Initializing m2p...\n");

  if (argc < 2) {
    // TODO: better this output
    printf("USAGE: m2p <target>\n");
    printf("m2p -h for more help\n");

    return 0;
  }

  printf("DEBUG: Opening input file...\n");
  FILE *f = fopen(argv[1], "r");

  if (f == NULL) {
    fprintf(stderr, "Error reading '%s' file, perhaps the file exists?\n",
            argv[1]);

    return 1;
  }

  printf("DEBUG: Reading input file...\n");
  fseek(f, 0, SEEK_END);

  printf("DEBUG: Reading length of input file...\n");
  int length = ftell(f);

  fseek(f, 0, SEEK_SET);

  printf("DEBUG: Allocating memory for input file buffer...\n");
  char *buffer = malloc(length);

  fread(buffer, sizeof(char), length, f);

  printf("DEBUG: Determining path of output file...\n");
  // this will not wprk i think
  char *obuffer = change_ext(argv[1]);
  printf("DEBUG: Output file will be written to: '%s'\n", obuffer);
  // The output is the same name of the file but instead of .md a .pdf extension
  printf("DEBUG: Initializing document writing\n");
  InitDocument(obuffer);

  // now parse the logic
  printf("DEBUG: Initializing parser struct\n");
  MD_PARSER parser = {
      .text = HandleText,
      .leave_span = LeaveSpan,
      .enter_span = EnterSpan,
      .enter_block = EnterBlock,
      .leave_block = LeaveBlock,
      .debug_log = NULL,
      .syntax = NULL,
      .abi_version = 0,
  };

  printf("DEBUG: Parsing document\n");

  SetFontTypeAndSize(P_SIZE, FONT_REGULAR);
  md_parse(buffer, (MD_SIZE)length, &parser, NULL);

  printf("DEBUG: Parsing finished\n");

  free(buffer);

  // WriteText("libpdf", 24, FONT_BOLD);
  // WriteHardBreak();
  // WriteText("libpdf", 14, FONT_BOLD);
  // WriteText("is a C library wrapper for", 14, FONT_REGULAR);
  // WriteText("libharu, a C library to write into PDF files", 14,
  // FONT_REGULAR);

  CloseDocument();

  return 0;
}
