#ifndef HELPERS_H
#define HELPERS_H

uint64_t* init_page();
ics_header* next_fit(size_t req_size);
ics_free_header* next(ics_free_header* cur);

#endif
