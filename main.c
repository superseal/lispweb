#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "main.h"
#include "parser.h"
#include "analyzer.h"

#define BUFFER_SIZE 1024 * 20

int main(int argc, char **argv) 
{
    if (argc != 2) {
        printf("Usage: %s FILE\n", argv[0]);
        exit(1);
    }
    
    char* content = read_source(argv[1]);
    char* filtered_content = filter(content);
    struct sexp tree = parse_document(filtered_content);

    analyze_document(tree);

    return 0;
}

char *read_source(const char *path)
{
    char *content = malloc(BUFFER_SIZE);
    FILE *source_file = fopen(path, "r");

    if (source_file == NULL) {
        fprintf(stderr, "(X) Could not open file %s: %s\n", path, strerror(errno));
        exit(1);
    }
        
    int i = 1, size; 
    while ((size = fread(content + BUFFER_SIZE * (i - 1), 1, BUFFER_SIZE, source_file)) == BUFFER_SIZE) {
        content = realloc(&content, BUFFER_SIZE * ++i);
    }

    return content;
}
