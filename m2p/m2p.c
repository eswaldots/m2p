#include "m2p.h"
#include "../include/libpdf.h"
#include "md4c.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

// TODO: maybe I can create two structs, and instead of declaring an MD_SPANTYPE
// or MD_BLOCKTYPE, I can declare a int and cast the type later.
typedef struct {
        MD_SPANTYPE type;
        char *detail;
} Span;

typedef struct {
        MD_BLOCKTYPE type;
        char *detail;
        // used for blocks like OL
        int count;
} Block;

typedef struct StyleBuffer {
        char *detail;
        MD_SPANTYPE current_span;
        MD_SPANTYPE parent_span;
        MD_BLOCKTYPE current_block;
        // TODO: maybe shouldn't be a fixed size, but I don't now edge cases
        // where the span_stack overflows
        Span spans_stack[256];
        Block blocks_stack[256];
} StyleBuffer;

// TODO: Instead of calculating the positions all the time, maybe we can put an
// index on style buffer? Maybe that is less dumbass
int GetMaxSPosition(Span v[]) {
        int i;
        for (i = 0; v[i].type != '\0'; ++i)
                ;

        return i;
}

int GetMaxBPosition(Block v[]) {
        int i;
        for (i = 0; v[i].type != '\0'; ++i)
                ;

        return i;
}

// TODO: for the upside functions maybe I can use the count in struct trick, but
// I don't know how do with this functions to don't repeat the same
// implementation twice, maybe I can try something with _Generic C11
int IsInSpanStack(MD_SPANTYPE type, Span v[]) {
        int max_pos = GetMaxSPosition(v);

        for (int i = 0; i < max_pos + 1; ++i) {
                if ((v[i].type - 1) == type) {
                        return 1;
                }
        }

        return 0;
}

int IsInBlockStack(MD_BLOCKTYPE type, Block v[]) {
        int max_pos = GetMaxBPosition(v);

        for (int i = 0; i < max_pos + 1; ++i) {
                if ((v[i].type - 1) == type) {
                        return 1;
                }
        }

        return 0;
}

// TODO: Also for this should be another way to don't repeat the
// implementation...
int GetSpanInStack(MD_SPANTYPE type, Span v[]) {
        int max_pos = GetMaxSPosition(v);

        for (int i = 0; i < max_pos + 1; ++i) {
                if ((v[i].type - 1) == type) {
                        return i;
                }
        }

        return -1;
}

int GetBlockInStack(MD_BLOCKTYPE type, Block v[]) {
        int max_pos = GetMaxBPosition(v);

        for (int i = 0; i < max_pos + 1; ++i) {
                if ((v[i].type - 1) == type) {
                        return i;
                }
        }

        return -1;
}

int HandleText(MD_TEXTTYPE type, const MD_CHAR *text, MD_SIZE size,
               void *userdata) {
        if (type == MD_TEXT_SOFTBR) {
                m2p_printf(M2P_LOG_DEBUG, "Introducing soft break\n");
                WriteSoftBreak();

                return 0;
        } else if (type == MD_TEXT_BR) {
                m2p_printf(M2P_LOG_DEBUG, "introducing hard break\n");

                WriteHardBreak();

                return 0;
        }

        char *buffer = malloc(strlen(text));

        strcpy(buffer, text);

        buffer[size] = '\0';

        StyleBuffer *buf = userdata;

        int is_italic = IsInSpanStack(MD_SPAN_EM, buf->spans_stack);
        int is_bold = IsInSpanStack(MD_SPAN_STRONG, buf->spans_stack);

        if (is_italic && is_bold) {
                SetFontTypeAndSize(P_SIZE, FONT_ITALIC_BOLD);
        } else if (is_bold) {
                SetFontTypeAndSize(P_SIZE, FONT_BOLD);
        } else if (is_italic) {
                SetFontTypeAndSize(P_SIZE, FONT_ITALIC);
        }

        if (IsInBlockStack(MD_BLOCK_LI, buf->blocks_stack)) {
                if (IsInBlockStack(MD_BLOCK_UL, buf->blocks_stack)) {
                        WriteDotSymbol();
                } else {
                        int indexf =
                            GetBlockInStack(MD_BLOCK_OL, buf->blocks_stack);

                        char *format = NULL;
                        // TODO: THIS IS WEIRD
                        format = malloc(strlen("%d .") * sizeof(char));

                        sprintf(format, "%d. ",
                                buf->blocks_stack[indexf].count);

                        WriteText(format);
                }
        }

        if (IsInSpanStack(MD_SPAN_A, buf->spans_stack)) {
                int i = GetSpanInStack(MD_SPAN_A, buf->spans_stack);

                WriteLink(buffer, buf->spans_stack[i].detail);
        } else {
                WriteText(buffer);
        }

        free(buffer);

        return 0;
}

int EnterBlock(MD_BLOCKTYPE type, void *detail, void *userdata) {
        StyleBuffer *buf = userdata;
        // TODO: use an array please
        //
        int index = GetMaxBPosition(buf->blocks_stack);
        buf->blocks_stack[index].type = type + 1;

        m2p_printf(M2P_LOG_DEBUG, "Entering in block type: %d\n", type);
        if (type == MD_BLOCK_OL) {
                buf->blocks_stack[index].count = 0;
        }
        if (type == MD_BLOCK_HR) {
                WriteHR();
        }

        if (type == MD_BLOCK_LI &&
            IsInBlockStack(MD_BLOCK_OL, buf->blocks_stack)) {
                int id = GetBlockInStack(MD_BLOCK_OL, buf->blocks_stack);
                int last = buf->blocks_stack[id].count;

                char *new_detail = malloc(1);
                sprintf(new_detail, "%d", (last + 1));

                buf->blocks_stack[index].detail = new_detail;
                buf->blocks_stack[id].count = last + 1;

                free(new_detail);
        }

        if (type == MD_BLOCK_OL) {
                buf->current_block = type;
        }

        if (type == MD_BLOCK_H) {
                MD_BLOCK_H_DETAIL *size = (MD_BLOCK_H_DETAIL *)detail;

                if (size->level == 1) {
                        // TODO: add a switch bro
                        SetFontTypeAndSize(HEADING_H1_SIZE, FONT_BOLD);
                } else if (size->level == 2) {
                        SetFontTypeAndSize(HEADING_H2_SIZE, FONT_BOLD);
                } else if (size->level == 3) {
                        SetFontTypeAndSize(HEADING_H3_SIZE, FONT_BOLD);
                } else if (size->level == 4) {
                        SetFontTypeAndSize(HEADING_H4_SIZE, FONT_BOLD);
                } else if (size->level == 5) {
                        SetFontTypeAndSize(HEADING_H5_SIZE, FONT_BOLD);
                } else if (size->level == 6) {
                        SetFontTypeAndSize(HEADING_H6_SIZE, FONT_BOLD);
                }
        }

        return 0;
}

int LeaveBlock(MD_BLOCKTYPE type, void *detail, void *userdata) {
        StyleBuffer *buf = userdata;

        int index = GetMaxBPosition(buf->blocks_stack);

        buf->blocks_stack[index - 1].type = 0;
        buf->blocks_stack[index - 1].detail = 0;

        if (type == MD_BLOCK_OL) {
                m2p_printf(M2P_LOG_DEBUG, "Leaving OL block with hard break\n");

                WriteSoftBreak();
        }

        if (type == MD_BLOCK_LI) {
                m2p_printf(M2P_LOG_DEBUG, "Leaving block with soft break\n");
                WriteSoftBreak();

                SetFontTypeAndSize(P_SIZE, FONT_REGULAR);
                return 0;
        } else if (buf->current_block != MD_BLOCK_OL &&
                   !IsInBlockStack(MD_BLOCK_UL, buf->blocks_stack)) {
                m2p_printf(M2P_LOG_DEBUG, "Leaving block: %d \n", type);
                m2p_printf(M2P_LOG_DEBUG, "Leaving block with hard break\n");
                WriteHardBreak();

                SetFontTypeAndSize(P_SIZE, FONT_REGULAR);
                return 0;
        }

        // TODO: implement a style stack
        SetFontTypeAndSize(P_SIZE, FONT_REGULAR);

        return 0;
};

int EnterSpan(MD_SPANTYPE type, void *detail, void *userdata) {
        // TODO: handle bold and italic

        StyleBuffer *buf = userdata;
        int index = GetMaxSPosition(buf->spans_stack);

        buf->spans_stack[index].type = type + 1;

        // here will send the details
        switch (type) {
        case MD_SPAN_A:
                MD_SPAN_A_DETAIL *link = detail;

                char *buffer = malloc(strlen(link->href.text));

                strcpy(buffer, link->href.text);

                buffer[link->href.size] = '\0';

                m2p_printf(M2P_LOG_DEBUG,
                           "Inserting href detail in stack %d: %s\n", index,
                           buffer);
                buf->spans_stack[index].detail = buffer;
        default:
                break;
        }

        return 0;
}

int LeaveSpan(MD_SPANTYPE type, void *detail, void *userdata) {
        StyleBuffer *buf = userdata;

        m2p_printf(M2P_LOG_DEBUG, "Leaving span, resetting font style\n");

        int index = GetMaxSPosition(buf->spans_stack);

        buf->spans_stack[index - 1].type = 0;
        buf->spans_stack[index - 1].detail = 0;

        SetFontTypeAndSize(P_SIZE, FONT_REGULAR);
        // WriteHardBreak();

        return 0;
}

static void usage(void) {
        printf("USAGE: m2p [...OPTIONS] [FILE]\n"
               "Convert input FILE (or standard input) in Markdown format to "
               "PDF.\n");
}

int main(int argc, char **argv) {
        m2p_printf(M2P_LOG_DEBUG, "Initializing m2p...\n");

        if (argc < 2) {
                usage();

                return 0;
        }

        m2p_printf(M2P_LOG_DEBUG, "Opening input file...\n");
        FILE *f = fopen(argv[1], "r");

        if (f == NULL) {
                fprintf(stderr,
                        "Error reading '%s' file, perhaps the file exists?\n",
                        argv[1]);

                return 1;
        }

        m2p_printf(M2P_LOG_DEBUG, "Reading input file...\n");
        fseek(f, 0, SEEK_END);

        m2p_printf(M2P_LOG_DEBUG, "Reading length of input file...\n");
        int length = ftell(f);

        fseek(f, 0, SEEK_SET);

        m2p_printf(M2P_LOG_DEBUG,
                   "Allocating memory for input file buffer...\n");
        char *buffer = malloc(length);

        fread(buffer, sizeof(char), length, f);
        fclose(f);

        m2p_printf(M2P_LOG_DEBUG, "Determining path of output file...\n");
        // this will not wprk i think

        int len = strlen(argv[1]);

        char *obuffer = malloc((len + 1) * sizeof(char));

        if (obuffer == NULL) {
                m2p_printf(M2P_LOG_ERROR,
                           "Failed to allocate memory buffer for the output "
                           "filename, exiting...\n");

                return -1;
        }

        m2p_printf(M2P_LOG_DEBUG, "Memory allocated for output filename\n");

        change_ext(argv[1], obuffer);

        m2p_printf(M2P_LOG_DEBUG, "Output file will be written to: '%s'\n",
                   obuffer);
        // The output is the same name of the file but instead of .md a .pdf
        // extension
        m2p_printf(M2P_LOG_DEBUG, "Initializing document writing\n");
        InitDocument(obuffer);

        // now parse the logic
        m2p_printf(M2P_LOG_DEBUG, "Initializing parser struct\n");
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

        m2p_printf(M2P_LOG_DEBUG, "Parsing document\n");

        SetFontTypeAndSize(P_SIZE, FONT_REGULAR);

        StyleBuffer buf = {0};

        buf.spans_stack[0].type = '\0';

        md_parse(buffer, (MD_SIZE)length, &parser, &buf);

        m2p_printf(M2P_LOG_DEBUG, "Parsing finished\n");

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
