#include "term.h"

static int 
tty_raw(struct term *term) {
    struct termios buf;

    if (tcgetattr(term->in_fd, &term->saved_termios) < 0) {
        return -1;
    }

    buf = term->saved_termios;

    buf.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    buf.c_iflag &= ~(BRKINT | ICRNL | ISTRIP | IXON);
    buf.c_cflag &= ~(CSIZE | PARENB);
    buf.c_cflag |= CS8;
    buf.c_oflag &= ~(OPOST);
    buf.c_cc[VMIN] = 1; 
    buf.c_cc[VTIME] = 0;

    if (tcsetattr(term->in_fd, TCSAFLUSH, &buf) < 0) {
        return -1;
    }

    term->mode = MODE_RAW;

    return 0;
}

//TODO: should not be specific to in_fd
static int 
tty_reset(struct term *term) {
    if (term->mode == MODE_RAW) {
        if (tcsetattr(term->in_fd, TCSAFLUSH, &term->saved_termios) < 0) {
            return -1;
        }
    }

    return 0;
}

static enum T_KEY 
term_parse_keytype(struct term *term, const char* buf) {
    int p[3];
    enum T_KEY k = KEY_NONE;

    p[0] = *buf;
    p[1] = *(buf+1);
    p[2] = *(buf+2);

    /*memset(term->debug_info.last_keycode, 0, 64);
    sprintf(term->debug_info.last_keycode, "\t%d %d %d", p[0], p[1], p[2]);*/

    if(p[0] == 27) {
        if(p[1] == 0 && p[2] == 0) k = KEY_ESC;
        else if(p[1] == 91 && p[2] == 65) k = KEY_UP;
        else if(p[1] == 91 && p[2] == 66) k = KEY_DOWN;
        else if(p[1] == 91 && p[2] == 67) k = KEY_RIGHT;
        else if(p[1] == 91 && p[2] == 68) k = KEY_LEFT;
        else k = KEY_NONE;
    }
    else if(p[0] == 13 && p[1] == 0) k = KEY_ENTER;
    else if(p[0] == 97 && p[1] == 0) k = KEY_A;
    else if(p[0] == 100 && p[1] == 0) k = KEY_D;
    else if(p[0] == 101 && p[1] == 0) k = KEY_E;
    else if(p[0] == 102 && p[1] == 0) k = KEY_F;
    else if(p[0] == 104 && p[1] == 0) k = KEY_H;
    else if(p[0] == 110 && p[1] == 0) k = KEY_N;
    else if(p[0] == 112 && p[1] == 0) k = KEY_P;
    else if(p[0] == 113 && p[1] == 0) k = KEY_Q;
    else if(p[0] == 119 && p[1] == 0) k = KEY_W;
    else if(p[0] == 114 && p[1] == 0) k = KEY_R;
    else if(p[0] == 115 && p[1] == 0) k = KEY_S;
    else if(p[0] == 120 && p[1] == 0) k = KEY_X;
    else if(p[0] == 121 && p[1] == 0) k = KEY_Y;
    else k = KEY_NONE;

    return k;
}

static enum T_KEY
term_get_key(struct term *term) {
    char raw_key[64];
    ssize_t num_bytes;
    enum T_KEY key_pressed;

    if(tty_raw(term) != 0) {
        printf("raw term fail");
        exit(EXIT_FAILURE);
    }

    memset(raw_key, 0, 64);

    num_bytes = read(term->in_fd, raw_key, 64);

    if (num_bytes == -1) {
        perror("read");
        return KEY_NONE;
    }
    
    key_pressed = term_parse_keytype(term, raw_key);

#ifdef DEBUG
    memset(term->debug_info.last_key_pressed, 0, 64);
    sprintf(term->debug_info.last_key_pressed, "last key: %d\n", key_pressed);
#endif

    if(tty_reset(term) != 0) {
        printf("reset term fail");
        exit(EXIT_FAILURE);
    }

    return key_pressed;
}

static void
term_base_key_event_handler(struct term_event_msg *msg) {
    switch(msg->key) {
        case KEY_NONE: 
            break;
        case KEY_ESC:
            term_exit(msg->term);
            break;
        default: 
            break;
    }
}

static void 
term_clr() {
    system("clear");
}

static void
term_reset(struct term *term) {
    memset(term->render_buf.data, 0, RBUFSIZE);
    term->render_buf.bytes_left = RBUFSIZE;
    term->render_buf.writeptr = term->render_buf.data;
}

static void
term_write(struct term *term, const char *str, size_t nbytes) {
    size_t w_size;

    w_size = (nbytes > term->render_buf.bytes_left) ? 
        term->render_buf.bytes_left :
        nbytes;

    term->render_buf.bytes_left -= w_size;

    if (term->render_buf.bytes_left == 0) {
        printf("%s: Buffer full\n", __FUNCTION__);
        return;
    }

    memcpy(term->render_buf.writeptr, str, w_size);
    term->render_buf.writeptr += w_size;
}

static void 
term_render(struct term *term) {
    size_t nbytes; 

    if (term->render_func) {
        term->render_func(term, term->render_data, term_write);    
    }

    nbytes = RBUFSIZE - term->render_buf.bytes_left;
    term_clr();
    write(term->out_fd, term->render_buf.data, nbytes);

#ifdef DEBUG
    write(term->out_fd, "\n", 2);
    write(term->out_fd, term->debug_info.last_key_pressed, 64);
    write(term->out_fd, term->debug_info.last_keycode, 64);
#endif

    term_reset(term);
}

static int 
term_do_run(struct term *term) {
    while (term->should_run) {
        enum T_KEY key_pressed;
        struct term_event_msg msg;

        term_render(term);

        msg.key = term_get_key(term);
        msg.term = term;
        msg.user_data = term->key_event_data;

        term_base_key_event_handler(&msg);
        
        if (term->key_event_handler) {
            term->key_event_handler(&msg);    
        }
    }
    return 0;
}

struct term *
term_new(int in_fd, int out_fd) {
    struct term *term = malloc(sizeof(struct term));

    if (!term) {
        printf("%s: no mem\n", __FUNCTION__);
        return NULL;
    }

    term_reset(term);

    term->in_fd = in_fd;
    term->out_fd = out_fd;
    term->mode = MODE_NORM;
    term->should_run = 0;

    return term;
}

void
term_free(struct term *term) {
    free(term);
}

//TODO: should support multiple handlers
void 
term_register_key_event_handler(struct term *term, 
        void (*handler)(struct term_event_msg *msg), 
        void *data) {
    term->key_event_handler = handler;
    term->key_event_data = data;
}

//TODO: should support multiple handlers
void
term_register_render_func(struct term *term,
        void (*render_func)(struct term *, void *, 
            void(*)(struct term *, const char *, size_t)), 
        void *data) {
    term->render_func = render_func;
    term->render_data = data;
}

int 
term_start(struct term *term) {
    int ret;
    
    term->should_run = 1;
    ret = term_do_run(term);

    return ret;
}

int
term_exit(struct term *term) {
    term->should_run = 0;
    return 0;
}