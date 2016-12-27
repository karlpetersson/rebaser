#ifndef COMMIT_H
#define COMMIT_H

#include "sds.h"

enum commit_action {
    CA_PICK,
    CA_REWORD,
    CA_EDIT,
    CA_SQUASH,
    CA_FIXUP,
    CA_EXEC,
    CA_DROP
};

struct commit {
    enum commit_action ca;
    sds msg;
    sds hash;
};

struct commit * commit_parse_from_line(sds line);
const char * commit_action_to_str(enum commit_action ca);
const char * commit_action_to_symbol(enum commit_action ca);
sds commit_render(struct commit *commit, sds buf);
sds commit_to_str(struct commit *commit);
void commit_set_action(struct commit *commit, enum commit_action ca);
void commit_free(struct commit *commit);

#endif