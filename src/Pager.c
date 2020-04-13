#include <fcntl.h>
#include "Pager.h"

Pager *pager_open(const char * path) {
    int fd = open(path,
                O_RDWR |    // Read/Write
                O_CREAT |   // Create file if not exist
                S_IWUSR |   // User write permission
                S_IRUSR);    // User read permission
    if (fd == -1) {
        printf("Unable to open file\n");
        exit(EXIT_FAILURE);
    }
    off_t file_length = lseek(fd,0,SEEK_END);
    Pager *pager = (Pager *)malloc(sizeof(Pager));
    pager->file_descriptor = fd;
    pager->file_length = file_length;
    for (uint32_t i = 0; i< TABLE_MAX_PAGES; i++) {
        pager->pages[i] = NULL;
    }
    return pager;
}

void flush_page(Pager *pager,uint32_t page_num,uint32_t size) {
    if (pager->pages[page_num] == NULL)
    {
        printf("Tried to flush null page\n");
        exit(EXIT_FAILURE);
    }
    off_t offset = lseek(pager->file_descriptor,page_num*PAGE_SIZE,SEEK_SET);
    if (offset == -1)
    {
        printf("Error when seeking file!");
        exit(EXIT_FAILURE);
    }
    ssize_t bytes_written = write(pager->file_descriptor, pager->pages[page_num], size);
    if (bytes_written == -1)
    {
        printf("Error when writing file!");
        exit(EXIT_FAILURE);
    }
}

void free_pager(Pager *pager, uint32_t full_page_num, ssize_t addition_row_size) {

    for (uint32_t i = 0; i< TABLE_MAX_PAGES; i++) {
        if (i < full_page_num && pager->pages[i] != NULL)
        {
            flush_page(pager, i, PAGE_SIZE);
        } else if (addition_row_size > 0 && i == full_page_num)
        {
            flush_page(pager, i, addition_row_size);
        }
        
        if(pager->pages[i] != NULL) {
            free(pager->pages[i]);
        }
        pager->pages[i] = NULL;
    }
    int result = close(pager->file_descriptor);
    if (result == -1)
    {
        printf("Error when closing file!");
        exit(EXIT_FAILURE);
    }
    
    free(pager);
}
