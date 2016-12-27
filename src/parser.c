#include "parser.h"
#include "sds.h"
#include "commit.h"

#include <stdio.h>
#include <stdlib.h>

static void
print_commit(unsigned int idx, void *data) {
    struct commit *commit = (struct commit *)data;
    printf("%d: %s %s %s\n", idx, commit_action_to_str(commit->ca), 
        commit->hash, commit->msg);
}

int 
parse_git_todo(const char *path, struct arraylist *commits) {
    char line[1024];
    FILE *fp;

    if (!(fp = fopen(path, "r"))) {
        perror("fopen");
        return -1;
    }

    while(fgets(line, sizeof(line), fp)) {
        sds curline = sdsnew(line);

        if (sdscmp(curline, "noop") == 0) {
            fprintf(stderr, "Noop rebase\n");
            return -1;
        }

        if (sdslen(curline) < 10 || (curline[0] == '#')) {
            // ignore comments/empty lines
        } else {
            struct commit *commit = commit_parse_from_line(curline);

            if (!commit) {
                return -1;
            }

#ifdef DEBUG
            printf("%s\n", curline);
#endif
            arraylist_append(commits, commit);
        }
    }
    return 0;
}