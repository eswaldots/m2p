#include "../../include/libpdf.h"

int main() {
  InitDocument("demo.pdf");

  SetFontTypeAndSize(16, FONT_REGULAR);
  WriteText("libpdf");
  WriteHardBreak();
  WriteText("libpdf");
  WriteText("is a C library wrapper for");
  WriteText("libharu, a C library to write into PDF files");

  CloseDocument();

  return 0;
}
