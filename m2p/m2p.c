#include "../include/libpdf.h"

int main() {
  InitDocument("demo.pdf");

  WriteText("libpdf", 24, FONT_BOLD);
  WriteHardBreak();
  WriteText("libpdf", 14, FONT_ITALIC_BOLD);
  WriteText("is a C library wrapper for ", 14, FONT_REGULAR);
  WriteText("libharu, a C library to write into PDF files", 14,
            FONT_ITALIC_BOLD);

  CloseDocument();

  return 0;
}
