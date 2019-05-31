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

	ics_free_header* found_block = next_fit(block_size);	
	
	while (found_block == NULL) {
		char* brk = ics_inc_brk();	
		if (brk == (void*)(-1) && errno == ENOMEM) return NULL;	
		ics_free_header* new_block = (ics_free_header*)(brk - WSIZE);		
		uint64_t prev_size = ((ics_footer*)(((char*)(new_block)) - WSIZE))->block_size;
		uint64_t new_block_size = 0;
		
		if (prev_size & 1) new_block_size = PSIZE;
		else {
			new_block = (ics_free_header*)(((char*)(new_block) - prev_size));
			new_block_size = new_block->header.block_size + PSIZE;
			remove_block(new_block);
		}
	
		new_block->header.block_size = new_block_size;
		new_block->header.unused = HEADER_UNUSED;
		((ics_footer*)(((char*)(new_block)) + new_block_size - WSIZE))->block_size = new_block_size;
		((ics_footer*)(((char*)(new_block)) + new_block_size - WSIZE))->unused = FOOTER_UNUSED;
	
		((ics_header*)(((char*)(new_block)) + new_block_size))->block_size = 1;

		insert_head(new_block);		
		freelist_next = new_block;
		found_block = next_fit(block_size);
	}

	size_t split_block_size = found_block->header.block_size - block_size;
	
	if (split_block_size >= MIN_BSIZE) {
		ics_free_header* split_block = (ics_free_header*)(((char*)(found_block)) + block_size);
		split_block->header.unused = HEADER_UNUSED;
		split_block->header.block_size = split_block_size;
		((ics_footer*)(((char*)(split_block)) + split_block_size - WSIZE))->block_size = split_block_size;
		insert_head(split_block);	
	}	

	coalesce(found_block);	

	found_block->header.requested_size = size;
	found_block->header.block_size = (block_size | 1);	
	((ics_footer*)(((char*)(found_block)) + WSIZE + aligned_size))->block_size = (block_size | 1);
	((ics_footer*)(((char*)(found_block)) + WSIZE + aligned_size))->unused = FOOTER_UNUSED;
	
	return (((char*)(found_block)) + WSIZE);
}

void* ics_realloc(void* ptr, size_t size) { return NULL; }

int ics_free(void* ptr) {
	ics_free_header* free_block = (ics_free_header*)(((char*)(ptr)) - WSIZE);
	free_block->header.block_size &= ~(1 << 0);
	((ics_footer*)(((char*)(free_block)) + free_block->header.block_size - WSIZE))->block_size &= ~(1 << 0);
	insert_head(free_block);
	coalesce(free_block);
	return 0;
}
