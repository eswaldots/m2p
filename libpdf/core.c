#include "../include/libpdf.h"

#include "hpdf.h"
#include <hpdf_doc.h>
#include <hpdf_font.h>
#include <hpdf_objects.h>
#include <hpdf_types.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void ErrorHandler(HPDF_STATUS error_no, HPDF_STATUS detail_no,
                  void *user_data) {
        fprintf(stderr, "\033[0;31mERROR:\033[0m internal libHaru error, ");
        fprintf(stderr, "error_no=%04X detail_no=%lu\n", error_no, detail_no);
};

int SplitWords(const char *line, char **words) {
        char *buffer = malloc(strlen(line) - 1);

        strcpy(buffer, line);

        char *tok = strtok(buffer, " ");

        int count = 0;

        while (tok != NULL) {
                words[count] = strdup(tok);

                ++count;

                tok = strtok(NULL, " ");
        }

        return count;
}

typedef struct Font {
        HPDF_Font bold, regular, italic, italic_bold;
} Font;

typedef struct PageData {
        char *filename;
        int xpos, ypos;
        Font font;
        Px font_size;
        FontType weight;
        HPDF_Page page;
        HPDF_Doc pdf;
} PageData;

PageData data = {0};

void InitDocument(char *filename) {
        HPDF_Doc pdf;

        pdf = HPDF_New(ErrorHandler, NULL);

        if (!pdf) {
                printf("ERROR: cannot create pdf object.\n");

                HPDF_Free(pdf);
        }

        HPDF_Page page;

        // TODO: add custom font support
        HPDF_Font regular = HPDF_GetFont(pdf, "Times-Roman", NULL);
        HPDF_Font bold = HPDF_GetFont(pdf, "Times-Bold", NULL);
        HPDF_Font italic = HPDF_GetFont(pdf, "Times-Italic", NULL);
        HPDF_Font italic_bold = HPDF_GetFont(pdf, "Times-BoldItalic", NULL);

        page = HPDF_AddPage(pdf);

        HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

        data.filename = filename;
        data.xpos = PAGE_MARGIN;
        data.ypos = HPDF_Page_GetHeight(page) - PAGE_MARGIN * 2;
        data.font_size = 16;
        data.font.regular = regular;
        data.font.italic = italic;
        data.font.italic_bold = italic_bold;
        data.font.bold = bold;
        data.weight = FONT_REGULAR;
        data.page = page;
        data.pdf = pdf;
}

void RestoreDocumentPosition() {
        data.xpos = PAGE_MARGIN;
        data.ypos = HPDF_Page_GetHeight(data.page) - PAGE_MARGIN * 2;
}

void CloseDocument() {
        HPDF_SetCompressionMode(data.pdf, HPDF_COMP_ALL);
        HPDF_SetPageMode(data.pdf, HPDF_PAGE_MODE_USE_OUTLINE);

        HPDF_SaveToFile(data.pdf, data.filename);

        HPDF_Free(data.pdf);
}

void SetFontTypeAndSize(Px font_size, FontType type) {
        // TODO: if font size doens't change don't make this call

        if (type == FONT_BOLD) {
                HPDF_Page_SetFontAndSize(data.page, data.font.bold, font_size);
        } else if (type == FONT_ITALIC) {
                HPDF_Page_SetFontAndSize(data.page, data.font.italic,
                                         font_size);
        } else if (type == FONT_ITALIC_BOLD) {
                HPDF_Page_SetFontAndSize(data.page, data.font.italic_bold,
                                         font_size);
        } else {
                HPDF_Page_SetFontAndSize(data.page, data.font.regular,
                                         font_size);
        }

        data.font_size = font_size;
        data.weight = type;
}

void WriteDotSymbol() {
        Px temp;
        temp = data.ypos;

        const Px space_width = HPDF_Page_TextWidth(data.page, " ");

        data.ypos += space_width;

        HPDF_Page_Circle(data.page, data.xpos, data.ypos, 2);
        HPDF_Page_Fill(data.page);

        data.xpos += space_width * 2;

        data.ypos = temp;
}

// TODO: refactor this and allow splitted links
void WriteLink(const char *text, const char *uri) {
        if (data.ypos < PAGE_MARGIN) {
                HPDF_Page new_page;

                new_page = HPDF_AddPage(data.pdf);

                data.page = new_page;
                RestoreDocumentPosition();

                SetFontTypeAndSize(data.font_size, data.weight);
        }
        // we calculate the space width based on the font size
        const Px space_width = HPDF_Page_TextWidth(data.page, " ");
        const Px line_height = data.font_size * 1.25;

        HPDF_Page page = data.page;

        Px pw = HPDF_Page_GetWidth(page);
        Px tw = HPDF_Page_TextWidth(page, text);

        HPDF_Page_BeginText(page);

        if (tw > pw) {
                char **words = malloc(strlen(text));

                // TODO: maybe in a higher level
                const Px line_width = pw - PAGE_MARGIN * 2;
                Px space_left = line_width;

                // If not equal is because is not a newline
                if (data.xpos != PAGE_MARGIN) {
                        space_left -= data.xpos;
                }

                printf("DEBUG: splitting words\n");
                int length = SplitWords(text, words);

                printf("DEBUG: words splitted up succesfully\n");

                printf("DEBUG: length %d\n", length);
                for (int i = 0; i < length; ++i) {
                        char *word = words[i];

                        Px ww = HPDF_Page_TextWidth(page, word);

                        if ((ww + space_width) > space_left) {
                                data.xpos = PAGE_MARGIN;
                                // create new line
                                data.ypos -= line_height;

                                space_left = (line_width - ww);
                        } else {
                                space_left = space_left - (ww + space_width);
                        }

                        HPDF_Page_TextOut(page, data.xpos, data.ypos, word);

                        // 6.67 is the space size
                        data.xpos += ww + space_width;
                }

                free(words);
        } else {
                HPDF_Rect rect = {
                    .bottom = data.ypos - data.font_size * 0.25,
                    .top = data.ypos + data.font_size,
                    .left = data.xpos,
                    .right = (data.xpos + tw),
                };

                HPDF_Page_CreateURILinkAnnot(page, rect, uri);

                HPDF_Page_SetRGBFill(page, 0.0, 0.0, 1.0);
                HPDF_Page_TextOut(page, data.xpos, data.ypos, text);

                HPDF_Page_SetRGBFill(page, 0.0, 0.0, 0.0);

                // don't add space, the user haves to care about that
                data.xpos += tw /* + space_width */;
        }

        // data.xpos = PAGE_MARGIN;
        // data.ypos -= LINE_HEIGHT;

        HPDF_Page_EndText(page);

        HPDF_Page_SetRGBFill(page, 0.0, 0.0, 1.0);
        HPDF_Page_Rectangle(page, data.xpos - tw,
                            data.ypos - data.font_size * 0.15, tw, 0.5);

        HPDF_Page_Fill(data.page);

        HPDF_Page_SetRGBFill(page, 0.0, 0.0, 0.0);
        // printf("browft? %d, %f\n", data.ypos,
        // HPDF_Page_GetHeight(data.page));
}

void WriteText(const char *text) {
        if (data.ypos < PAGE_MARGIN) {
                HPDF_Page new_page;

                new_page = HPDF_AddPage(data.pdf);

                data.page = new_page;
                RestoreDocumentPosition();

                SetFontTypeAndSize(data.font_size, data.weight);
        }
        // we calculate the space width based on the font size
        const Px space_width = HPDF_Page_TextWidth(data.page, " ");
        const Px line_height = data.font_size * 1.25;

        HPDF_Page page = data.page;

        Px pw = HPDF_Page_GetWidth(page);
        Px tw = HPDF_Page_TextWidth(page, text);

        HPDF_Page_BeginText(page);

        if (tw > pw) {
                char **words = malloc(strlen(text));

                // TODO: maybe in a higher level
                const Px line_width = pw - PAGE_MARGIN * 2;
                Px space_left = line_width;

                // If not equal is because is not a newline
                if (data.xpos != PAGE_MARGIN) {
                        space_left -= data.xpos;
                }

                printf("DEBUG: splitting words\n");
                int length = SplitWords(text, words);

                printf("DEBUG: words splitted up succesfully\n");

                printf("DEBUG: length %d\n", length);
                for (int i = 0; i < length; ++i) {
                        char *word = words[i];

                        Px ww = HPDF_Page_TextWidth(page, word);

                        if ((ww + space_width) > space_left) {
                                data.xpos = PAGE_MARGIN;
                                data.ypos -= line_height;

                                space_left = (line_width - ww);
                        } else {
                                space_left = space_left - (ww + space_width);
                        }

                        HPDF_Page_TextOut(page, data.xpos, data.ypos, word);

                        // 6.67 is the space size
                        data.xpos += ww + space_width;
                }

                free(words);
        } else {
                HPDF_Page_TextOut(page, data.xpos, data.ypos, text);

                // don't add space, the user haves to care about that
                data.xpos += tw /* + space_width */;
        }

        // data.xpos = PAGE_MARGIN;
        // data.ypos -= LINE_HEIGHT;

        HPDF_Page_EndText(page);
        // printf("browft? %d, %f\n", data.ypos,
        // HPDF_Page_GetHeight(data.page));
}

void WriteHardBreak() {
        data.xpos = PAGE_MARGIN;
        data.ypos -= LINE_HEIGHT;
}

void WriteSoftBreak() {
        data.xpos = PAGE_MARGIN;
        data.ypos -= LINE_HEIGHT / 2;
}

void WriteHR() {
        data.ypos += LINE_HEIGHT;

        HPDF_Page_Rectangle(data.page, data.xpos, data.ypos,
                            HPDF_Page_GetWidth(data.page) - PAGE_MARGIN * 2,
                            0.1);
        HPDF_Page_Fill(data.page);

        data.ypos += LINE_HEIGHT / 3;
}
