#include "icsmm.h"
#include "macros.h"
#include "helpers.h"

uint64_t* init_page() {
	uint64_t* brk = ics_inc_brk();
	if (brk == (uint64_t*)(-1)) return NULL;
 
	((ics_footer*)(brk))->block_size = 1;
	((ics_header*)(((char*)(brk)) + PSIZE - WSIZE))->block_size = 1;

	ics_free_header* first_block = ((ics_free_header*)(((char*)(brk)) + WSIZE)); 
	first_block->header.unused = HEADER_UNUSED;
	first_block->header.block_size = PSIZE - DWSIZE;
	first_block->prev = first_block->next = NULL;

	((ics_footer*)(((char*)(brk)) + PSIZE - DWSIZE))->unused = FOOTER_UNUSED;
	((ics_footer*)(((char*)(brk)) + PSIZE - DWSIZE))->block_size = PSIZE - DWSIZE;

	freelist_head = freelist_next = (ics_free_header*)(&first_block->header);
	
	return (uint64_t*)(((char*)(brk)) + DWSIZE);
}

ics_free_header* next_fit(size_t req_size) {
	if (freelist_next == NULL) return NULL;
	ics_free_header* cur_block = freelist_next;
	
	while (cur_block->header.block_size < req_size) {
		cur_block = next(cur_block);
		if (cur_block == freelist_next) return NULL;
	}	

	freelist_next = next(cur_block);
	remove_block(cur_block);	

	return cur_block;	
}

ics_free_header* next(ics_free_header* cur) { 
	return (cur->next == NULL) ? freelist_head : cur->next;
}

void insert_head(ics_free_header* new_head) {
	if (new_head == NULL) return;

	if (freelist_head) freelist_head->prev = new_head;
	else freelist_next = new_head;

	new_head->next = freelist_head;
	new_head->prev = NULL;
	freelist_head = new_head;
}

void remove_block(ics_free_header* block) {
	if (block == NULL) return;
	if (block->prev) block->prev->next = block->next;		
	if (block->next) block->next->prev = block->prev;	
	if (block->prev == NULL && block->next == NULL) freelist_head = freelist_next = NULL;
}

void coalesce(ics_free_header* block) {
	uint64_t prev_size = ((ics_footer*)(((char*)(block)) - WSIZE))->block_size;
	uint64_t next_size = ((ics_header*)(((char*)(block)) + block->header.block_size))->block_size;
	ics_free_header* prev = (ics_free_header*)(((char*)(block)) - prev_size);
	ics_free_header* next = (ics_free_header*)(((char*)(block)) + block->header.block_size);
	
	if (((prev_size & 1) == 0) && (next_size & 1)) {
		remove_block(prev);	
		prev->header.block_size = prev_size + block->header.block_size;
		((ics_footer*)(((char*)(prev)) + prev->header.block_size - WSIZE))->block_size = prev->header.block_size;
		insert_head(prev);
	} else if (((next_size & 1) == 0) && (prev_size & 1)) {
		remove_block(next);	
		block->header.block_size = next_size + block->header.block_size;
		((ics_footer*)(((char*)(next)) + next->header.block_size - WSIZE))->block_size = block->header.block_size;
		insert_head(block);
		if (freelist_next == next) freelist_next = freelist_head;
	} else if (((prev_size & 1) == 0) && ((next_size & 1) == 0)) {	
		remove_block(prev);	
		remove_block(next);	
		prev->header.block_size = prev_size + block->header.block_size + next_size;
		((ics_footer*)(((char*)(next)) + next->header.block_size - WSIZE))->block_size = prev->header.block_size;
		insert_head(prev);
		if (freelist_next == next) freelist_next = freelist_head;
	}
}
