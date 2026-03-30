#include <hpdf_doc.h>
#include <hpdf_font.h>

typedef float Px;

#define DEFAULT_DPI 72;

static Px PAGE_MARGIN = 24;
static Px LINE_HEIGHT = 28;

typedef enum FontType {
  FONT_BOLD,
  FONT_REGULAR,
  FONT_ITALIC,
  FONT_ITALIC_BOLD,
} FontType;

void InitDocument(char *filename);
void CloseDocument();

void WriteText(const char *text, Px font_size, FontType type);
void WriteHardBreak();
