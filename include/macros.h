#define ALIGNMENT 16
#define PSIZE (1 << 12)
#define WSIZE sizeof(void*)
#define DWSIZE sizeof(void*) * 2
#define MAX_PAGES 4
#define MIN_BSIZE (2 * WSIZE + ALIGNMENT)
