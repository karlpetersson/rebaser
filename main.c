#include "stdio.h"
#include "stdlib.h"

int main(int argc, char *argv[]) {
    FILE *fp;
        
    if (argc < 2) {
        fprintf(stderr, "Usage: reb <git-rebase-todo-file>\n");
        exit(EXIT_FAILURE);
    }

    if (!(fp = fopen(argv[1], "rw"))) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    parse(fp);

    exit(EXIT_SUCCESS);
}

int parse(FILE *fp) {
    char line[256];

    while(fgets(line, sizeof(line), fp)) {
        printf("%s", line);
    }
}