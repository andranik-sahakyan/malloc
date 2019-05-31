#ifndef HELPERS_H
#define HELPERS_H

uint64_t* init_page();
ics_free_header* next_fit(size_t req_size);
ics_free_header* next(ics_free_header* cur);
void insert_head(ics_free_header* new_head); 
void remove_block(ics_free_header* block);

#endif
