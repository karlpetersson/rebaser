#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>

#include "arraylist.h"

#define RBUFSIZE (1024 * 100)
#define DEBUG (1)

enum T_KEY {
    KEY_NONE, KEY_ENTER, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_A, KEY_E,
    KEY_D, KEY_F, KEY_H, KEY_N, KEY_P, KEY_R, KEY_S, KEY_Q, KEY_W, KEY_Y, KEY_X,
    KEY_ESC, NUM_KEYS
};

struct term_event_msg {
    struct term *term;
    enum T_KEY key;
    void *user_data;
};

struct term {
    int in_fd;
    int out_fd;
    struct termios saved_termios;
    unsigned int should_run;

    void (*key_event_handler)(struct term_event_msg *);
    void (*render_func)(struct term *, void *, 
        void(*)(struct term *, const char *, size_t));

    void *render_data;
    void *key_event_data;

    enum mode {
        MODE_NORM,
        MODE_RAW
    } mode;

    struct render_buf {
        char data[RBUFSIZE];
        char *writeptr;
        size_t bytes_left;
    } render_buf;

    struct debug {
        char last_keycode[64];
        char last_key_pressed[64];
    } debug_info;
};

struct term * term_new(int in_fd, int out_fd);
void term_register_key_event_handler(struct term *term, 
        void (*handler)(struct term_event_msg *msg), 
        void *data);
void term_register_render_func(struct term *term,
        void (*render_func)(struct term *, void *, 
            void(*)(struct term *, const char *, size_t)), 
        void *data);
//void term_write(struct term *term, const char *str, size_t nbytes);
int term_start(struct term *term);
int term_exit(struct term *term);

#endif