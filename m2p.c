#include <hpdf.h>
#include <hpdf_font.h>
#include <md4c.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>

typedef int MM;

#define DEFAULT_SIZE 12
#define MARGIN 254

typedef struct DocumentWriter {
        HPDF_Doc doc;
        HPDF_Font font;
        int font_size;
        HPDF_Page current_page;
        int y, x;
} DocumentWriter;

int enter_block(MD_BLOCKTYPE type, void *detail, void *user_data) {
        printf("DEBUG: entering block type: %d  with detail %d \n");

        DocumentWriter *dw = user_data;
        MD_BLOCK_H_DETAIL level;

        switch (type) {
        case MD_BLOCK_H:
                level = *(MD_BLOCK_H_DETAIL *)(detail);
                int font_size;

                printf("DEBUG: eetting font size for level: %d \n",
                       level.level);

                switch (level.level) {
                case 1:
                        font_size = 28;

                        break;
                case 2:
                        font_size = 14;

                        break;
                case 3:
                        font_size = 16;

                        break;
                default:
                        font_size = 24;
                }

                HPDF_Page_SetFontAndSize(dw->current_page, dw->font, font_size);
        }

        return 0;
}

int leave_block(MD_BLOCKTYPE type, void *detail, void *user_data) {
        printf("DEBUG: leaving block\n");

        DocumentWriter *dw = user_data;
        int level, tw;

        switch (type) {
        case MD_BLOCK_H:
                HPDF_Page_SetFontAndSize(dw->current_page, dw->font,
                                         DEFAULT_SIZE);
        }

        return 0;
}

char *split(const MD_CHAR *line, int lim) {
        char *new = malloc(lim + 2);
        int i;

        for (i = 0; i < lim; ++i) {
                new[i] = line[i];
        }

        new[i + 1] = '\n';
        new[i + 2] = '\0';

        return new;
}

int handle_text(MD_TEXTTYPE type, const MD_CHAR *text, MD_SIZE size,
                void *user_data) {
        DocumentWriter *dw = user_data;
        printf("DEBUG: handling text: %s with type %d and size %d\n", text,
               type, size);

        int tw;
        char *splitted;

        switch (type) {
        case MD_TEXT_NORMAL:
                HPDF_Page_BeginText(dw->current_page);

                splitted = split(text, size);

                tw = HPDF_Page_TextWidth(dw->current_page, splitted);

                dw->x += (HPDF_Page_GetWidth(dw->current_page) - tw) / 2;
                dw->y += HPDF_Page_GetHeight(dw->current_page) - 50;

                HPDF_Page_TextOut(dw->current_page, dw->x, dw->y, splitted);

                free(splitted);

                HPDF_Page_EndText(dw->current_page);
        }

        return 0;
}

void debug_log(const char *msg, void *user_data) { printf("%s", msg); }

void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no,
                   void *user_data) {
        /* invoke longjmp() when an error has occurred */
        printf("ERROR: error_no=%04X, detail_no=%d\n", (unsigned int)error_no,
               (int)detail_no);
}

int main(int argc, char *argv[]) {
        // verify commands
        if (argc < 2) {
                printf("USAGE: m2p <input> <output>\n");

                return 1;
        }

        printf("DEBUG: reading input file\n");

        // read the markdown file and process into string
        long input_size;
        FILE *finput = fopen(argv[1], "rb");

        if (finput == NULL) {
                perror("file input not found\n");

                return 1;
        }

        fseek(finput, 0, SEEK_END);

        input_size = ftell(finput);

        fseek(finput, 0, SEEK_SET);

        printf("DEBUG: allocating dynamic memory for file buffer\n");
        char *buffer = malloc(input_size);

        if (buffer == NULL) {
                perror("error allocating memory for the input buffer\n");

                return 1;
        }

        fread(buffer, 1, input_size, finput);
        fclose(finput);

        printf("DEBUG: file readed succesfully\n");

        if (buffer == NULL) {
                perror("error writing markdown for the input buffer\n");

                return 1;
        }

        printf("DEBUG: initializing markdown parser\n");
        // now the buffer has all the markdown data, is time to parse into SAX
        MD_PARSER parser = {
            .enter_block = enter_block,
            .text = handle_text,
            .leave_block = leave_block,
            .enter_span = NULL,
            .leave_span = NULL,
            .abi_version = 0,
            .debug_log = debug_log,
            .flags = MD_FLAG_WIKILINKS,
        };

        // declare the pdf write

        HPDF_Doc pdf;
        HPDF_Font font;
        HPDF_Page page;

        printf("DEBUG: initializing pdf document\n");
        pdf = HPDF_New(error_handler, NULL);

        if (!pdf) {
                perror("error creating pdf object\n");

                return 1;
        }

        HPDF_SetCompressionMode(pdf, HPDF_COMP_ALL);
        HPDF_SetPageMode(pdf, HPDF_PAGE_MODE_USE_OUTLINE);

        page = HPDF_AddPage(pdf);

        printf("DEBUG: getting embedded helvetica font\n");
        font = HPDF_GetFont(pdf, "Helvetica", NULL);

        printf("debug: initializing document writer\n");
        DocumentWriter dw = {
            .current_page = page,
            .font = font,
            .x = MARGIN,
            .y = MARGIN,
            .doc = pdf,
        };

        printf("DEBUG: converting and writing ast to pdf\n");
        if ((md_parse(buffer, input_size, &parser, &dw)) == -1) {
                perror("error parsing the pdf document");

                free(buffer);

                return 1;
        };

        free(buffer);

        HPDF_SaveToFile(dw.doc, "./out.pdf");

        HPDF_Free(dw.doc);

        return 0;
}
