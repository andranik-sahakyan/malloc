/**
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 * If you want to make helper functions, put them in helpers.c
 */
#include "icsmm.h"
#include "helpers.h"
#include <stdio.h>
#include <stdlib.h>

ics_free_header *freelist_head = NULL;
ics_free_header *freelist_next = NULL;

void *ics_malloc(size_t size) { return NULL; }

void *ics_realloc(void *ptr, size_t size) { return NULL; }

int ics_free(void *ptr) { return -1; }
