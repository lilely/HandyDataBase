#include "Pager.h"
#include "Row.h"

#ifndef __PAGER_H__
#define __PAGER_H__

typedef struct {
    uint32_t root_page_num;
    Pager *pager;
} Table;

typedef struct {
    Table *table;
    uint32_t page_num;
    uint32_t cell_num;
    _Bool is_end;
} Cursor;

extern const uint32_t ROWS_PER_PAGE;
extern const uint32_t TABLE_MAX_ROWS;

extern Table *db_open(const char *path);

extern void db_close(Table *table);

extern void *row_slot(Table *table,uint32_t row_num);

extern Cursor *create_cursor_of_start(Table *table);

extern Cursor *create_cursor_of_end(Table *table);

extern void *cursor_value(Cursor *cursor);

extern void cursor_advanced(Cursor *cursor);

extern void *get_page(Pager *pager,uint32_t page_num);

extern uint32_t get_unused_page_num(Table *table);

extern void create_new_root(Table *table, uint32_t right_child_page_num);

#endif