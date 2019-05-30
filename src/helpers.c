#include "icsmm.h"
#include "macros.h"
#include "helpers.h"

uint64_t* init_page() {
	uint64_t* brk = ics_inc_brk();
	if (brk == (uint64_t*)(-1)) return NULL;
 
	((ics_header*)(brk))->block_size = 0x0001; // prologue
	((ics_header*)(((char*)(brk)) + PSIZE - WSIZE))->block_size = 0x0001; // epilogue		

	ics_free_header* first_block = ((ics_free_header*)(((char*)(brk)) + WSIZE)); 
	first_block->header.unused = 0xaaaaaaaa;
	first_block->header.block_size = PSIZE - DWSIZE;
	first_block->prev = first_block->next = NULL;

	((ics_footer*)(((char*)(brk)) + PSIZE - DWSIZE))->unused = 0xffffffffffff;
	((ics_footer*)(((char*)(brk)) + PSIZE - DWSIZE))->block_size = PSIZE - DWSIZE;

	freelist_head = freelist_next = (ics_free_header*)(&first_block->header);
	
	return (uint64_t*)(((char*)(brk)) + DWSIZE);
}

ics_header* next_fit(size_t req_size) {
	ics_free_header* cur_block = freelist_next;
	while (cur_block->header.block_size < req_size) {
		cur_block = next(cur_block);
		if (cur_block == freelist_next) return NULL;
	}	

	freelist_next = next(cur_block);

	if (cur_block->prev && cur_block->next) {
		cur_block->next->prev = cur_block->prev;
		cur_block->prev->next = cur_block->next;
	}

	return (&cur_block->header);	
}

ics_free_header* next(ics_free_header* cur) { 
	return (cur->next == NULL) ? freelist_head : cur->next;
}
