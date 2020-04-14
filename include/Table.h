#include "Pager.h"

#ifndef __PAGER_H__
#define __PAGER_H__

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

typedef struct {
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE+1];
    char email[COLUMN_EMAIL_SIZE+1];
} Row;

typedef struct {
    uint32_t num_rows;
    Pager *pager;
} Table;

typedef struct {
    Table *table;
    uint32_t num_of_row;
    _Bool is_end;
} Cursor;

#define sizeof_attribute(Struct, Attribute) sizeof(((Struct *)0)->Attribute)

extern const uint32_t ID_SIZE;
extern const uint32_t USERNAME_SIZE;
extern const uint32_t EMAIL_SIZE;

extern const uint32_t ROW_SIZE;
extern const uint32_t ID_OFFSET;
extern const uint32_t USERNAME_OFFSET;
extern const uint32_t EMAIL_OFFSET;
extern const uint32_t ROWS_PER_PAGE;
extern const uint32_t TABLE_MAX_ROWS;

extern Table *db_open(const char *path);

extern void db_close(Table *table);

extern void *row_slot(Table *table,uint32_t row_num);

extern Cursor *create_cursor_of_start(Table *table);

extern Cursor *create_cursor_of_end(Table *table);

extern void *cursor_value(Cursor *cursor);

extern void cursor_advanced(Cursor *cursor);

#endif