#include "parser.h"
#include "arraylist.h"
#include "commit.h"
#include "gui.h"

#include <stdio.h>
#include <stdlib.h>

static void
save_rebase(const char *path, struct arraylist *list) {
    FILE *fp = fopen(path, "w");
    int i;

    for (i = 0; i < arraylist_length(list); i++) {
        struct commit *commit = (struct commit *) arraylist_get(list, i);
        sds line = commit_to_str(commit);
        size_t ret;

        ret = fwrite(line, 1, sdslen(line), fp);

        if (ret < sdslen(line) || (ret == 0 && (feof(fp) == 0))) {
            fprintf(stderr, "%s: failed to write line\n", __FUNCTION__);
        }

        sdsfree(line);
    }

    fclose(fp);
}

static void
abort_rebase(const char *path) {
    FILE *fp = fopen(path, "w");
    fclose(fp);
}

static void
clean_up(struct arraylist *commits) {
    int i;

    if (!commits) {
        return;
    }

    for (i = 0; i < arraylist_length(commits); i++) {
        struct commit *commit = (struct commit *) arraylist_get(commits, i);
        commit_free(commit);
    }
}

int main(int argc, char *argv[]) {  
    struct arraylist *commits;
    int save_on_exit;
    int ret;

    if (argc < 2) {
        fprintf(stderr, "Usage: reb <git-rebase-todo-file>\n");
        goto error;
    }

    commits = arraylist_new(16);
    if (!commits) {
        fprintf(stderr, "%s: no mem\n", __FUNCTION__);
        goto error;
    }

    ret = parse_git_todo(argv[1], commits);
    if (ret == -1) {
        fprintf(stderr, "%s: failed to parse git-todo-file\n", __FUNCTION__);
        goto error;
    }

    ret = gui_start(commits, &save_on_exit);
    if (ret == -1) {
        fprintf(stderr, "%s: gui exited erroneously\n", __FUNCTION__);
        goto error;
    }

    if (save_on_exit) {
        save_rebase(argv[1], commits);
    } else {
        abort_rebase(argv[1]);
    }

    clean_up(commits);
    exit(EXIT_SUCCESS);
error:
    clean_up(commits);
    exit(EXIT_FAILURE);
}