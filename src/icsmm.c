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

	if (first_request) {
		init_page();
		first_request = 0;
	}
	
	size_t aligned_size = ((size - 1) | 15) + 1;
	size_t block_size = aligned_size + DWSIZE;	

	ics_header* found_block = next_fit(block_size);	
	
	while (found_block == NULL) {
		return NULL;
		//next_fit(block_size);	
	}

	size_t split_block_size = found_block->block_size - block_size;
	
	if (split_block_size >= MIN_BSIZE) {
		ics_free_header* split_block = (ics_free_header*)(((char*)(found_block)) + block_size);
		split_block->header.unused = 0xaaaaaaaa;
		split_block->header.block_size = split_block_size;
		((ics_footer*)(((char*)(split_block)) + split_block_size - WSIZE))->block_size = split_block_size;
		
	}	

	found_block->requested_size = size;
	found_block->block_size = (block_size | 1);	
	((ics_footer*)(((char*)(found_block)) + WSIZE + aligned_size))->block_size = (block_size | 1);
	((ics_footer*)(((char*)(found_block)) + WSIZE + aligned_size))->unused = 0xffffffffffff;
	
	return (((char*) found_block) + WSIZE);
}

void* ics_realloc(void* ptr, size_t size) { return NULL; }

int ics_free(void* ptr) { return -1; }
