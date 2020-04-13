#include <stdlib.h>
#include <stdio.h>

#define TABLE_MAX_PAGES 100
#define PAGE_SIZE 4096

typedef struct {
    int file_descriptor;
    uint32_t file_length;
    void *pages[TABLE_MAX_PAGES]; 
} Pager;

extern Pager *pager_open(const char * path);
extern void free_pager(Pager *pager, uint32_t full_page_num, ssize_t addition_row_size);