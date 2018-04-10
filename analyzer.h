#include <stdio.h>

#define ARR_LENGTH(a) (sizeof(a) / sizeof((a)[0]))

#define YELLOW_FG "\x1b[33m"

#define abort_analysis(err_type, err_format, ...) \
    char err_buffer[200]; \
    sprintf(err_buffer, err_format, ##__VA_ARGS__); \
    fprintf(stderr, "%s[X] %s (line %d, col %d - line %d, col %d):%s %s\n", \
            YELLOW_FG, err_type, \
            opening_line, opening_col, closing_line, closing_col, \
            COLOR_RESET, err_buffer); \

void analyze_syntax(struct sexp tree);
void analyze_expression(struct sexp leaf);
void analyze_document(struct sexp tree);
