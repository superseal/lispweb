#include "parser.h"
#include "analyzer.h"

const char *valid_cars[] = {
    "document",
    "title",
    "section",
    "'",
    "b",
    "i",
    "o",
    "u",
    "sub",
    "sup",
    "sp",
    "box",
    "img",
    "bul-list",
    "num-list",
    "it",
    "link",
};

char is_valid_car(const char* car) {
    int i; 
    for (i = 0; i < ARR_LENGTH(valid_cars); i++) { 
        if (strcmp(car, valid_cars[i]) == 0) {  
            return 1;
        }
    } 
    return 0;
}

void analyze_document(struct sexp tree)
{
    analyze_expression(tree);
    
    if (tree.cdr_subnodes) {
        int e;
        for (e = 0; e < tree.cdr_subnodes; e++) {
            analyze_document(tree.cdr_list[e]);
        }
    }
}

unsigned int opening_line = 0, opening_col = 0;
unsigned int closing_line = 0, closing_col = 0;

void analyze_expression(struct sexp leaf) 
{
    opening_line = leaf.opening_line; opening_col = leaf.opening_col;
    closing_line = leaf.closing_line; closing_col = leaf.closing_col;

    /* ***** General errors ***** */
    if (!is_valid_car(leaf.car)) {
        abort_analysis("Semantic error", "%s is not a valid car", leaf.car);
    }

    /***** Expression-specific errors *****/

    /* Higher arity functions */
    if (strcmp("section", leaf.car) == 0 || strcmp("link", leaf.car) == 0 || strcmp("img", leaf.car) == 0) {
        if (leaf.cdr_subnodes < 2) {
            abort_analysis("Semantic error", "%s expression takes at least 2 arguments, got %d arguments", leaf.car, leaf.cdr_subnodes);
            return;
        }
        if (strcmp("\'", leaf.cdr_list[0].car) != 0) {
            abort_analysis("Semantic error", "First argument of a %s expression must be a literal text expression, found %s expression instead", leaf.car, leaf.cdr_list[0].car);
        }
    } 
    
    if (strcmp("bul-list", leaf.car) == 0 || strcmp("num-list", leaf.car) == 0) {
        if (leaf.cdr_subnodes < 2) {
            abort_analysis("Semantic error", "Lists must have at least one it expression");
            return;
        }
        int i;
        for (i = 1; i < leaf.cdr_subnodes; i++) {
            if (strcmp(leaf.cdr_list[i].car, "it") != 0) {
                abort_analysis("Semantic error", "Lists can only be composed by it expressions, found %s expression", leaf.cdr_list[i].car);
            }
        }
    }
}
