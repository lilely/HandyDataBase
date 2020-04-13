#include "Table.h"
#include <fcntl.h>

const uint32_t ID_SIZE = sizeof_attribute(Row,id);
const uint32_t USERNAME_SIZE = sizeof_attribute(Row,username);
const uint32_t EMAIL_SIZE = sizeof_attribute(Row,email);

const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_SIZE;
const uint32_t EMAIL_OFFSET = ID_SIZE + USERNAME_SIZE;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE/ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = TABLE_MAX_PAGES*ROWS_PER_PAGE;

void db_close(Table *table){
    free_pager(table->pager,table->num_rows/ROWS_PER_PAGE,(table->num_rows%ROWS_PER_PAGE)*ROW_SIZE);
    free(table);
}

Table *db_open(const char *path) {
    Pager *pager = pager_open(path);
    uint32_t num_rows = pager->file_length/ROW_SIZE;
    Table *table = (Table *)malloc(sizeof(Table));
    printf("pager->file_length is %ld\n",pager->file_length);
    printf("ROW_SIZE is %ld\n",ROW_SIZE);
    printf("num_rows is %ld\n",num_rows);
    table->pager = pager;
    table->num_rows = num_rows;
    return table;
}

void *get_page(Table *table,uint32_t page_num) {
    if (page_num > TABLE_MAX_ROWS)
    {
        printf("Tried to fetch page number out of bounds. %d > %d\n", page_num,
           TABLE_MAX_PAGES);
        exit(EXIT_FAILURE);
    }
    
    void *page = table->pager->pages[page_num];
    if (page == NULL)
    {
        page = malloc(PAGE_SIZE);
        Pager *pager = table->pager;
        uint32_t num_pages = table->pager->file_length/PAGE_SIZE;
        if (pager->file_length % PAGE_SIZE)
        {
            num_pages += 1;
        }
        if(page_num <= num_pages) {
            lseek(pager->file_descriptor,page_num*PAGE_SIZE,SEEK_SET);
            size_t bytes_read = read(pager->file_descriptor,page,PAGE_SIZE);
            if (bytes_read == -1)
            {
                printf("Error when reading file!");
                exit(EXIT_FAILURE);
            }
        }
        table->pager->pages[page_num] = page;
    }
    
    return page;
}

void *row_slot(Table *table,uint32_t row_num) {
    uint32_t page_num = row_num/ROWS_PER_PAGE;
    printf("page_num is %d\n",page_num);
    uint32_t row_offset = row_num%ROWS_PER_PAGE;
    void *page = get_page(table, page_num);
    printf("ROW_SIZE is %d\n",ROW_SIZE);
    printf("row_offset is %d\n",row_offset);
    void *row_slot = page+row_offset*ROW_SIZE;
    return row_slot;
}
