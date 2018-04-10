#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define get_token()\
    char_index++; \
    if (c == '\t') { \
        col += 4; \
    } else if (c == '\n') { \
        line += 1; col = 0; \
    } else { \
        col += 1; \
    }

#define RED_FG "\x1b[31m"
#define COLOR_RESET "\x1b[0m"

#define abort_parsing(err_type, err_format, ...) \
    char err_buffer[200]; \
    sprintf(err_buffer, err_format, ##__VA_ARGS__); \
    fprintf(stderr, "%s[X] %s:%s %s (line %d, col %d)\n", RED_FG, err_type, COLOR_RESET, err_buffer, line, col); \
    exit(1) 

#define PARSE_BUFFER_LENGTH 4096
#define EXP_LIST_LENGTH 128

//#define DEBUG

#ifdef DEBUG
#define debug(message, ...) printf(message, ##__VA_ARGS__)
#else
#define debug(message, ...)
#endif

enum parser_state {
    START,
    CAR, 
    ID,
    WHITESPACE, 
    CDR,
    FINISH,
};

struct sexp {
    char *car;
    char *id;
    char *cdr_atom;
    unsigned int cdr_atom_length;
    /* Nested structures, not always used */
    unsigned char cdr_length;
    struct sexp *cdr_list;
    /* Error handling */
    unsigned int opening_line, opening_col;
    unsigned int closing_line, closing_col;
};

char *filter(char *content);
struct sexp parse_document(char *content);
struct sexp parse_expression(char *content);
