#include "gui.h"
#include "term.h"
#include "sds.h"
#include "commit.h"
#include "colors.h"

#include <stdlib.h>
#include <stdio.h>

#define GUI_HELP_TEXT \
    "\n" \
    "Key bindings\n" \
    "---\n" \
    "p = pick, use commit\n" \
    "r = reword, use commit, but edit the commit message\n" \
    "e = edit, use commit, but stop for amending\n" \
    "s = squash, use commit, but meld into previous commit\n" \
    "f = fixup, like squash, but discard this commit's log message\n" \
    "x = exec (not supported yet)\n" \
    "d = drop, remove commit\n" \
    "ENTER = move commit (place with ENTER)\n" \
    "a = abort rebase\n" \
    "q = prompt exit dialogue\n" \
    "h = show/hide help text\n" \

#define GUI_EXIT_PROMPT "\nSave & exit? (y/n)"
#define GUI_EMPTY_LINE "    |\n"
#define GUI_CURSOR (CLR_TWO " ➜  " CLR_RESET)
#define GUI_NO_CURSOR "    "
#define GUI_MOVING_COMMIT "| "

static int gui_cursor_pos;
static int gui_yanking_commit;
static int gui_should_prompt_exit;
static int gui_should_display_help;

static int save_on_exit;

static void gui_exit_prompt_handler(struct term_event_msg *msg);
static void gui_event_handler(struct term_event_msg *msg);

static void
gui_exit_prompt_handler(struct term_event_msg *msg) {
    switch(msg->key) {
        case KEY_Y:
            save_on_exit = 1;
            term_exit(msg->term);
            break;
        case KEY_N:
            gui_should_prompt_exit = 0;
            term_register_key_event_handler(msg->term, gui_event_handler, msg->user_data);
            break;
        default:
            break;
    }
}

static void
gui_event_handler(struct term_event_msg *msg) {
    struct arraylist *list = (struct arraylist *)msg->user_data;
    struct commit *cur = (struct commit *) arraylist_get(list, gui_cursor_pos);

    switch(msg->key) {
        case KEY_UP:
            if (gui_yanking_commit) {
                if (gui_cursor_pos > 0) {
                    arraylist_swap(list, gui_cursor_pos, gui_cursor_pos-1);
                }
            }
            if (--gui_cursor_pos < 0) {
                gui_cursor_pos = 0;
            } 
            break;
        case KEY_DOWN:
            if (gui_yanking_commit) {
                if (gui_cursor_pos < (arraylist_length(list) - 1)) {
                    arraylist_swap(list, gui_cursor_pos, gui_cursor_pos+1);
                }
            }
            if (++gui_cursor_pos > (arraylist_length(list) - 1)) {
                gui_cursor_pos = arraylist_length(list) - 1;
            }
            break;
        case KEY_D: commit_set_action(cur, CA_DROP); break;
        case KEY_P: commit_set_action(cur, CA_PICK); break;
        case KEY_E: commit_set_action(cur, CA_EDIT); break;    
        case KEY_F: commit_set_action(cur, CA_FIXUP); break;
        case KEY_R: commit_set_action(cur, CA_REWORD); break;
        case KEY_S: commit_set_action(cur, CA_SQUASH); break;
        case KEY_X: commit_set_action(cur, CA_EXEC); break;
        case KEY_A:
            term_exit(msg->term);
            break;
        case KEY_Q:
            gui_should_prompt_exit = 1;
            term_register_key_event_handler(msg->term, gui_exit_prompt_handler, 
                msg->user_data);
            break;
        case KEY_H:
            gui_should_display_help = !gui_should_display_help;
            break;
        case KEY_ENTER:
            gui_yanking_commit = !gui_yanking_commit;
            break;
        default: break;
    }
}

// ugly
static void
gui_render(struct term *term, void *user_data, 
        void(*write_hook)(struct term *, const char *, size_t)) {
    struct arraylist *list = (struct arraylist *)user_data;
    sds render_buffer = sdsempty();
    int i;

    for (i = 0; i < arraylist_length(list); i++) {
        struct commit *commit = (struct commit *)arraylist_get(list, i);
        sds commit_str = sdsempty();

        render_buffer = sdscat(render_buffer, GUI_EMPTY_LINE);

        if (i == gui_cursor_pos) {
            commit_str = sdscat(commit_str, GUI_CURSOR);
            if (gui_yanking_commit) {
                commit_str = sdscat(commit_str, GUI_MOVING_COMMIT);
            }
        } else {
            commit_str = sdscat(commit_str, GUI_NO_CURSOR);
        }

        commit_str = commit_render(commit, commit_str);
        render_buffer = sdscatsds(render_buffer, commit_str);
        sdsfree(commit_str);
    }

    if (gui_should_display_help) {
        render_buffer = sdscat(render_buffer, GUI_HELP_TEXT);
    }

    if (gui_should_prompt_exit) {
        render_buffer = sdscat(render_buffer, GUI_EXIT_PROMPT);
    }

    write_hook(term, render_buffer, sdslen(render_buffer));
    sdsfree(render_buffer);
}

int
gui_start(struct arraylist *commits, int *should_save) {
    struct term *term;
    int ret;

    term = term_new(STDIN_FILENO, STDOUT_FILENO);
    if (!term) {
        fprintf(stderr, "%s: no mem\n", __FUNCTION__);
        return -1;
    }

    gui_cursor_pos = 0;
    gui_yanking_commit = 0;
    save_on_exit = 0;
    gui_should_prompt_exit = 0;
    gui_should_display_help = 0;

    term_register_key_event_handler(term, gui_event_handler, commits);
    term_register_render_func(term, gui_render, commits);
    
    ret = term_start(term);
    if (ret == -1) {
        fprintf(stderr, "%s: term exited wrongly\n", __FUNCTION__);
        return -1;
    }

    *should_save = save_on_exit;

    return 0;
}