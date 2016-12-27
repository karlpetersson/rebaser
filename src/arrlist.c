#include "arrlist.h"

#include <stdio.h>
#include <stdlib.h>

struct arraylist *
arraylist_new(size_t initsize) {
    struct arraylist *list = malloc(sizeof(struct arraylist));
    list->data = malloc(sizeof(void *) * initsize);
    list->size = initsize;
    list->len = 0;

    if (!list || !list->data) {
        fprintf(stderr, "%s: no mem\n", __FUNCTION__);
        return NULL;
    }

    return list;
}

unsigned int
arraylist_prepend(struct arraylist *list, void *data) {
    return arraylist_insert(list, 0, data);
}

unsigned int
arraylist_append(struct arraylist *list, void *data) {
    return arraylist_insert(list, list->len, data);
}

unsigned int
arraylist_grow(struct arraylist *list) {
    int new_size = list->size * 2;
    list->data = realloc(list->data, sizeof(void*) * new_size);

    if (!list->data) {
        fprintf(stderr, "%s: no mem\n", __FUNCTION__);
        return 0;
    }

    list->size = new_size;
    
    return 0;
}

unsigned int
arraylist_insert(struct arraylist *list, unsigned int idx, void *data) {
    int ret, shift, i;

    if (idx > list->len) {
        fprintf(stderr, "%s: Index (%d) out of bounds\n", __FUNCTION__, idx);
        return 0;
    }

    shift = list->len - idx;

    if (shift > 0) {
        for (i = list->len - 1; i >= shift; i++) {
            list->data[i+1] = list->data[i];
        }
    }

    list->data[idx] = data;
    list->len++;

    if (list->len >= list->size) {
        ret = arraylist_grow(list);
        if (ret != 0) {
            return 0;
        }
    }

    return idx;
}

void *
arraylist_remove(struct arraylist *list, unsigned int idx) {
    int ret, i;
    void *element;

    if (idx >= list->len) {
        fprintf(stderr, "%s: Index (%d) out of bounds\n", __FUNCTION__, idx);
        return NULL;
    }

    element = list->data[idx];

    // potentially shift elements
    for (i = idx; i < list->len; i++) {
        list->data[i] = list->data[i+1];
    }

    list->data[list->len--] = NULL;

    return element;
}

void *
arraylist_get(struct arraylist *list, unsigned int idx) {
    if (idx >= arraylist_length(list)) {
        fprintf(stderr, "%s: index out of bounds\n", __FUNCTION__);
        return NULL;
    }

    return list->data[idx];
}

void
arraylist_print(struct arraylist *list, void (*el_print_func)(unsigned int, void *)) {
    printf("size: %zd\n", list->size);
    printf("length: %d\n", list->len);
    printf("-- elements -- \n");

    if (el_print_func) {
        int i;

        for (i = 0; i < list->len; i++) {
            el_print_func(i, arraylist_get(list, i));
        }
    }
}

unsigned int
arraylist_length(struct arraylist *list) {
    return list->len;
}

void *
arraylist_replace(struct arraylist *list, unsigned int idx, void *data) {
    void *rm;

    if (idx >= arraylist_length(list)) {
        fprintf(stderr, "%s: index out of bounds\n", __FUNCTION__);
        return NULL;
    }

    rm = list->data[idx];
    list->data[idx] = data;

    return rm;
}

int
arraylist_swap(struct arraylist *list, unsigned int idx1, unsigned int idx2) {
    void *temp;

    if (idx1 >= list->len || idx2 >= list->len) {
        return -1;
    }

    temp = list->data[idx1];
    list->data[idx1] = list->data[idx2];
    list->data[idx2] = temp;

    return 0;
}


void
arraylist_free(struct arraylist *list, void (*el_free_func)(unsigned int, void *));
