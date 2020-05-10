#include "Table.h"
#include <fcntl.h>
#include "Btree.h"

const uint32_t ROWS_PER_PAGE = PAGE_SIZE/sizeof(Row);
const uint32_t TABLE_MAX_ROWS = TABLE_MAX_PAGES*ROWS_PER_PAGE;

void db_close(Table *table){
    free_pager(table->pager);
    free(table);
}

uint32_t get_unused_page_num(Table *table){
    return table->pager->num_pages;
}

void* get_page(Pager *pager,uint32_t page_num) {
    if (page_num > TABLE_MAX_ROWS)
    {
        printf("Tried to fetch page number out of bounds. %d > %d\n", page_num,
           TABLE_MAX_PAGES);
        exit(EXIT_FAILURE);
    }
    printf("in get_page, page num is %d\n",page_num);
    printf("pager address is %ld",pager);
    void *page = pager->pages[page_num];
    if (page == NULL)
    {
        printf("in get_page before read0\n");
        page = malloc(PAGE_SIZE);
        printf("in get_page before read1 file_length is %d\n",pager->file_length);
        uint32_t num_pages = pager->file_length/PAGE_SIZE;
        printf("in get_page before read2\n");
        if (pager->file_length % PAGE_SIZE)
        {
            num_pages += 1;
        }
        printf("in get_page before read3\n");
        if(page_num <= num_pages) {
            printf("in get_page before read\n");
            lseek(pager->file_descriptor,page_num*PAGE_SIZE,SEEK_SET);
            size_t bytes_read = read(pager->file_descriptor,page,PAGE_SIZE);
            if (bytes_read == -1)
            {
                printf("Error when reading file!");
                exit(EXIT_FAILURE);
            }
            printf("in get_page after read, read length is %d\n",bytes_read);
        }
        pager->pages[page_num] = page;
        if (page_num >= pager->num_pages)
        {
            pager->num_pages = page_num + 1;
        }
        printf("pager's page_num is %d\n",pager->num_pages);
    } else {
        printf("Page is not NULL\n");
    }
    printf("out of get_page, address of page is %ld\n",pager->pages[page_num]);
    return pager->pages[page_num];
}

Table *db_open(const char *path) {
    Pager *pager = pager_open(path);
    Table *table = (Table *)malloc(sizeof(Table));
    printf("pager->file_length is %ld\n",pager->file_length);
    printf("ROW_SIZE is %ld\n",ROW_SIZE);
    table->pager = pager;
    table->root_page_num = 0;
    if (pager->num_pages == 0)
    {
        void *root_node = get_page(table->pager,0);
        initialize_leaf_node(root_node);
        set_node_root(root_node,1);
        printf("address of root node is %ld\n",root_node);
        uint32_t num_of_cells = *(leaf_node_num_cells(root_node));
        printf("number of cells is %d",num_of_cells);
    }
    
    return table;
}

void *row_slot(Table *table,uint32_t row_num) {
    uint32_t page_num = row_num/ROWS_PER_PAGE;
    printf("page_num is %d\n",page_num);
    uint32_t row_offset = row_num%ROWS_PER_PAGE;
    void *page = get_page(table->pager, page_num);
    printf("ROW_SIZE is %d\n",ROW_SIZE);
    printf("row_offset is %d\n",row_offset);
    void *row_slot = page+row_offset*ROW_SIZE;
    return row_slot;
}

Cursor *create_cursor_of_start(Table *table) {
    Cursor *cursor = (Cursor *)malloc(sizeof(Cursor));
    cursor->table = table;
    cursor->page_num = table->root_page_num;
    cursor->cell_num = 0;
    void* root_node = get_page(table->pager,table->root_page_num);
    uint32_t num_cells = leaf_node_num_cells(root_node);
    cursor->is_end = (num_cells == 0);
    return cursor;
}

Cursor *create_cursor_of_end(Table *table) {
    Cursor *cursor = (Cursor *)malloc(sizeof(Cursor));
    cursor->table = table;
    cursor->page_num = table->root_page_num;
    printf("in create cursor, table->root_page_num is %d",table->root_page_num);
    void* root_node = get_page(table->pager,table->root_page_num);
    // cursor->cell_num = leaf_node_num_cells(root_node);
    // cursor->is_end = 1;
    return cursor;
}

void *cursor_value(Cursor *cursor) {
    uint32_t page_num = cursor->page_num;
    void *root_node = get_page(cursor->table->pager, page_num);
    return leaf_node_value(root_node,cursor->cell_num);
}

void cursor_advanced(Cursor *cursor) {
    uint32_t page_num = cursor->page_num;
    void* node = get_page(cursor->table->pager,page_num);
    cursor->cell_num +=1;
    uint32_t next_page_num = *leaf_node_next_leaf(node);
    if (next_page_num == 0)
    {
        cursor->is_end = 1;
    } else {
        cursor->page_num = next_page_num;
        cursor->is_end = 0;
        cursor->cell_num = 0;
    }
    
}

void create_new_root(Table *table, uint32_t right_child_page_num) {
    void *root = get_page(table->pager, table->root_page_num);
    void *right_node = get_page(table->pager, right_child_page_num);
    uint32_t left_page_num = get_unused_page_num(table);
    void *left_node = get_page(table->pager, left_page_num);

    memcpy(left_node,root, PAGE_SIZE);
    set_node_root(left_node,0);

    initialize_internal_node(root);
    set_node_root(left_node,1);

    *internal_node_num_keys(root) = 1;
    *internal_node_right_child(root) = right_child_page_num;
    uint32_t max_key = get_node_max_key(left_node);
    *internal_node_key(root,0) = max_key;
    *internal_node_child(root,0) = left_page_num;
}