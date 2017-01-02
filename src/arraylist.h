#ifndef ARRLIST_H
#define ARRLIST_H

#include <string.h>

struct arraylist {
    void **data;
    size_t size;
    unsigned int len;
};

struct arraylist * arraylist_new(size_t initsize);
unsigned int arraylist_append(struct arraylist *list, void *data);
unsigned int arraylist_prepend(struct arraylist *list, void *data);
unsigned int arraylist_insert(struct arraylist *list, unsigned int idx, void *data);
void * arraylist_remove(struct arraylist *list, unsigned int idx);
void * arraylist_get(struct arraylist *list, unsigned int idx);
void arraylist_free(struct arraylist *list, void (*el_free_func)(unsigned int, void *));
void arraylist_print(struct arraylist *list, void (*el_print_func)(unsigned int, void *));
void arraylist_sort(struct arraylist *list);
unsigned int arraylist_length(struct arraylist *list);
int arraylist_swap(struct arraylist *list, unsigned int idx1, unsigned int idx2);
void *arraylist_replace(struct arraylist *list, unsigned int idx, void *data);

#endif