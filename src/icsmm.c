#include "icsmm.h"
#include "helpers.h"
#include "macros.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

ics_free_header* freelist_head = NULL;
ics_free_header* freelist_next = NULL;
int first_request = 1;

void* ics_malloc(size_t size) { 
	if (size > (PSIZE * MAX_PAGES - WSIZE)) { errno = ENOMEM; return NULL; }
	else if (size == 0) { errno = EINVAL; return NULL; }

	//uint64_t* brk = (uint64_t*)(ics_get_brk());

	if (first_request) {
		init_page();
		first_request = 0;
	}
	
	size_t aligned_size = ((size - 1) | 15) + 1;
	ics_header* found_block = next_fit(aligned_size);	
	
	if (found_block == NULL) {
		// extend heap
		return NULL;	
	}

	found_block->requested_size = size;
	found_block->block_size = ((aligned_size + DWSIZE) | 1);	
	((ics_footer*)(((char*)(found_block)) + WSIZE + aligned_size))->block_size = ((aligned_size + DWSIZE) | 1);
	((ics_footer*)(((char*)(found_block)) + WSIZE + aligned_size))->unused = 0xaaaaaaaaaaaa;
	
	return (((char*) found_block) + WSIZE);
}

void* ics_realloc(void* ptr, size_t size) { return NULL; }

int ics_free(void* ptr) { return -1; }
