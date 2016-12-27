#include "commit.h"
#include "colors.h"

#include <stdlib.h>
#include <stdio.h>

struct commit *
commit_parse_from_line(sds line) {
    struct commit *commit;
    sds *tokens;
    sds c_action, c_hash, c_msg;
    int count, i;

    commit = malloc(sizeof(struct commit));

    if (!commit) {
        return NULL;
    }

    tokens = sdssplitlen(line, sdslen(line), " ", 1, &count);

    if (count < 3) {
        fprintf(stderr, "%s: Invalid line \n", __FUNCTION__);
        return NULL;
    }

    commit->ca = CA_PICK;
    commit->hash = tokens[1];
    commit->msg = sdsempty();

    for (i = 2; i < count; i++) {
        commit->msg = sdscatsds(commit->msg, tokens[i]);
        
        if (i != (count - 1 )) {
            commit->msg = sdscat(commit->msg, " ");
        }
        
        sdsfree(tokens[i]);
    }

    return commit;
}

void
commit_set_action(struct commit *commit, enum commit_action ca) {
    commit->ca = (commit->ca != ca) ? ca : CA_PICK;
}

void
commit_free(struct commit *commit) {
    sdsfree(commit->hash);
    sdsfree(commit->msg);
    free(commit);
}

sds
commit_to_str(struct commit *commit) {
    sds cstr = sdsempty();
    cstr = sdscat(cstr, commit_action_to_str(commit->ca));
    cstr = sdscat(cstr, " ");
    cstr = sdscat(cstr, commit->hash);
    cstr = sdscat(cstr, " ");
    cstr = sdscat(cstr, commit->msg);
    return cstr;
}

sds
commit_render(struct commit *commit, sds buf) {
    buf = sdscat(buf, commit_action_to_symbol(commit->ca));
    buf = sdscat(buf, " " CLR_ONE);
    buf = sdscatsds(buf, commit->hash);
    buf = sdscat(buf, CLR_RESET " ");
    buf = sdscatsds(buf, commit->msg);
    return buf;
}

const char *
commit_action_to_str(enum commit_action ca) {
    switch (ca) {
        case CA_PICK: return "pick";
        case CA_REWORD: return "reword";
        case CA_EDIT: return "edit";
        case CA_SQUASH: return "squash";
        case CA_FIXUP: return "fixup";
        case CA_EXEC: return "exec";
        case CA_DROP: return "drop";
    }
    return "";
}

const char *
commit_action_to_symbol(enum commit_action ca) {
    switch (ca) {
        case CA_PICK: return "o";
        case CA_REWORD: return CLR_TWO "r" CLR_RESET;
        case CA_EDIT: return CLR_TWO "e" CLR_RESET;
        case CA_SQUASH: return CLR_TWO "s" CLR_RESET;
        case CA_FIXUP: return CLR_TWO "f" CLR_RESET;
        case CA_EXEC: return CLR_TWO "x" CLR_RESET;
        case CA_DROP: return CLR_TWO "d" CLR_RESET;
    }
    return "";
}