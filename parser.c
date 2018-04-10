#include "parser.h"

char *filter(char *content) 
{
    return content;
}

unsigned int char_index = 0;
unsigned int line = 1, col = 1;

/* Not part of the parser, just for convenience */
int formatting_depth = 0;
void format_sexp(struct sexp exp) 
{
    int pw;
    for (pw = 0; pw < formatting_depth; pw++) {
        printf("    ");
    }

    printf("### car=<%s> ", exp.car);

    if (exp.id != NULL) {
        printf("id=<%s> ", exp.id);
    }

    if (exp.cdr_length) {
        printf("(%d subexps)\n", exp.cdr_length);
        int e;
        for (e = 0; e < exp.cdr_length; e++) {
            formatting_depth += 1;
            format_sexp(exp.cdr_list[e]);
            formatting_depth -= 1;
        }
    } else {
        printf("cdr=<%s>, %d bytes\n", exp.cdr_atom, exp.cdr_atom_length);
    }
}

struct sexp parse_document(char *content) 
{
    /* Expression buffer and index */
    struct sexp *exp_list = malloc(sizeof(struct sexp) * EXP_LIST_LENGTH);
    unsigned int exp_index = 0;

    while (content[char_index]) {
        struct sexp node = parse_expression(content);
        if (node.car != "empty-expression") {
            format_sexp(node);
            exp_list[exp_index++] = node;
        }
    }

    struct sexp root = {
        .car = "document",
        .cdr_length = exp_index,
    };

    root.cdr_list = malloc(sizeof(struct sexp) * exp_index);
    int e;
    for (e = 0; e < exp_index; e++) {
        root.cdr_list[e] = exp_list[e];
    }

    return root;
}

struct sexp parse_expression(char *content) 
{
    unsigned int length = strlen(content);

    enum parser_state state = START;
    
    /* Parse buffer and index */
    char parse_buffer[PARSE_BUFFER_LENGTH] = {0};
    unsigned int i = 0;
   
    /* Expression buffer and index */
    struct sexp *exp_list = malloc(sizeof(struct sexp) * EXP_LIST_LENGTH);
    unsigned int exp_index = 0;

    char *car = NULL, *id = NULL, *cdr = NULL;

    /* Indices for error reporting */
    unsigned int opening_line, opening_col;
    unsigned int closing_line, closing_col;
    
    /* Parsing conditions */
    char complex_cdr = 0, whitespace_cdr = 1;

    /* Sub-expression length */
    unsigned int sublength = 0;

    char c = ' ';
    while (c) {
        c = content[char_index];
        debug("Reading #%d [%c], line %d, col %d: ", char_index, c, line, col);
        
        /* Special cases */
        if (c == '}' && (state == CAR || state == WHITESPACE)) {
            abort_parsing("Syntax error", "%s expression has an empty body", car);
        }

        if (c == '\\') {
            /* Escape next character */
            char_index++;
            parse_buffer[i++] = content[char_index++];
            continue;
        }
        
        /* State machine */
        if (state == START) {
            debug("START\n");

            if (c == '{') {
                opening_line = line; opening_col = col;
                state = CAR;
            }
        } else if (state == CAR) {
            debug("CAR\n");
            
            if (isspace(c)) {
                parse_buffer[i++] = '\0';
                car = strdup(parse_buffer);
                state = WHITESPACE;
                i = 0;
            } else if (c == '#') {
                parse_buffer[i++] = '\0';
                car = strdup(parse_buffer);
                state = ID;
                i = 0;
                char_index++;
                continue;
            } else {
                parse_buffer[i++] = c;
            }
        } else if (state == ID) {
            debug("ID\n");

            if (!isspace(c)) {
                parse_buffer[i++] = c;
            } else {
                parse_buffer[i++] = '\0';
                id = strdup(parse_buffer);
                state = WHITESPACE;
                i = 0;
            } 
        } else if (state == WHITESPACE) {
            debug("WHITESPACE\n");

            /* Skip all whitespace between car and cdr */
            if (!isspace(c)) {
                state = CDR;
                continue;
            }
        } else if (state == CDR) {
            debug("CDR sub %d\n", sublength);

            if (c == '{') {
                complex_cdr = 1;
                /* Convert dangling strings into text expressions */
                if (i > 0) {
                    parse_buffer[i++] = '\0';
                    i = 0;
                    cdr = strdup(parse_buffer);
                    /* All-whitespace cdr */
                    int w;
                    for (w = 0; w < strlen(cdr); w++) {
                        if (!isspace(cdr[w])) {
                            whitespace_cdr = 0;
                            break;
                        }
                    }
                    /* Avoid creating text expression if cdr is all-whitespace */
                    if (!whitespace_cdr) {
                        struct sexp promoted_car = {
                            .car = "\'", 
                            .cdr_atom = cdr, 
                            .cdr_length = 0
                        };
                        exp_list[exp_index++] = promoted_car;
                        whitespace_cdr = 1;
                    }
                }
                /* Recursive parsing */
                struct sexp cons = parse_expression(content);
                //sublength += cons.cdr_length;
                exp_list[exp_index++] = cons;
            } else if (c == '}') {
                parse_buffer[i++] = '\0';
                cdr = strdup(parse_buffer);
                /* All-whitespace cdr */
                int w;
                for (w = 0; w < strlen(cdr); w++) {
                    if (!isspace(cdr[w])) {
                        whitespace_cdr = 0;
                        break;
                    }
                }
                /* Same as before, promote dangling strings to text expressions */
                if (complex_cdr && !whitespace_cdr) {
                    struct sexp promoted_car = {
                        .car = "\'", 
                        .cdr_atom = cdr, 
                        .cdr_atom_length = sublength,
                        .cdr_length = 0
                    };
                    exp_list[exp_index++] = promoted_car;
                }

                closing_line = line; closing_col = col;
                state = FINISH;
                i = 0;

                continue;
            } else {
                /* Normal cdr atom parsing */
                parse_buffer[i++] = c;
                sublength++;
            }
        } else if (state == FINISH) {
            debug("FINISH\n");

            struct sexp exp = {
                .car = car,
                .id = id,
                .cdr_atom = cdr,
                .cdr_atom_length = sublength,
                .cdr_length = exp_index,
                .opening_line = opening_line,
                .opening_col = opening_col,
                .closing_line = closing_line,
                .closing_col = closing_col,
            };
            
            if (exp_index > 0) {
                exp.cdr_list = malloc(sizeof(struct sexp) * exp_index);
                int e;
                for (e = 0; e < exp_index; e++) {
                    exp.cdr_list[e] = exp_list[e];
                }
            } else {
                exp.cdr_list = NULL;
                exp.cdr_length = 0;
            }
            
            car = NULL;
            id = NULL;
            cdr = NULL;
            exp_index = 0;
            return exp;
        }
        
        get_token();
    }
    
    /* State machine should be in FINISH state at this point */
    if (state == START) {
        return (struct sexp) {.car = "empty-expression"};
    } else if (state == CDR) {
        abort_parsing("Syntax error", "Missing closing brace in %s expression", car);
    }
} 
