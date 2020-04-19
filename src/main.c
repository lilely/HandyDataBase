#include <stdio.h>
#include <math.h>
#include <string.h>
#include "Table.h"
#include "Btree.h"

typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGINIZED,
} MetaCommandResult;

typedef enum {
    STATEMENT_PREPARE_SUCCESS,
    STATEMENT_PREPARE_SYNTAX_ERROR,
    STATEMENT_PREPARE_PARAM_TOO_LONG,
    STATEMENT_PREPARE_NEGATIVE_ID,
    STATEMENT_PREPARE_UNRECONGINZED,
} StatementPrepareResult;

typedef enum {
    EXECUTE_SUCCESS,
    EXECUTE_FAIL,
    EXECUTE_DUPLICATE_KEY,
    EXECUTE_TABLE_FULL,
} ExecuteResult;

typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT,
} StatementType;

typedef struct {
    StatementType statement_type;
    Row row;
} Statement;

typedef struct {
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;

InputBuffer* new_input_buffer() {
    InputBuffer *input_buffer = (InputBuffer *)malloc(sizeof(InputBuffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;
    return input_buffer;
}

void read_input(InputBuffer *input_buffer) {
    ssize_t bytes_read = getline(&input_buffer->buffer,&input_buffer->buffer_length,stdin);
    if (bytes_read <= 0) {
        printf("Error reading commnad!");
        exit(EXIT_FAILURE);
    }
    input_buffer->input_length = bytes_read - 1;
    input_buffer->buffer[bytes_read-1] = 0;
}

void close_inputBuffer(InputBuffer *input_buffer) {
    free(input_buffer->buffer);
    free(input_buffer);
}

void print_prompt() {
    printf("db > ");
}

void print_constants() {
    printf("ROW_SIZE: %d\n", ROW_SIZE);
    printf("COMMON_NODE_HEADER_SIZE: %d\n", COMMON_NODE_HEADER_SIZE);
    printf("LEAF_NODE_HEADER_SIZE: %d\n", LEAF_NODE_HEADER_SIZE);
    printf("LEAF_NODE_CELL_SIZE: %d\n", LEAF_NODE_CELL_SIZE);
    printf("LEAF_NODE_SPACE_FOR_CELLS: %d\n", LEAF_NODE_SPACE_FOR_CELLS);
    printf("LEAF_NODE_MAX_CELLS: %d\n", LEAF_NODE_MAX_CELLS);
}

void print_leaf_node(void* node) {
    uint32_t num_cells = *leaf_node_num_cells(node);
    printf("leaf (size %d)\n", num_cells);
    for (uint32_t i = 0; i < num_cells; i++) {
        uint32_t key = *leaf_node_key(node, i);
        printf("  - %d : %d\n", i, key);
    }
}

MetaCommandResult process_meta_command(InputBuffer *input_buffer, Table *table) {
    if (strcmp(input_buffer->buffer,".exit") == 0)
    {
        close_inputBuffer(input_buffer);
        db_close(table);
        exit(EXIT_SUCCESS);
    } else if(strcmp(input_buffer->buffer,".constant") == 0) {
        printf("Constants:\n");
        print_constants();
        return META_COMMAND_SUCCESS;
    } else if(strcmp(input_buffer->buffer,".btree") == 0) {
        printf("Tree:\n");
        print_leaf_node(get_page(table->pager,0));
        return META_COMMAND_SUCCESS;
    } else {
        return META_COMMAND_UNRECOGINIZED;
    }
}

void dump_statement(Statement *statement) {
    printf("\nid is %d\n",statement->row.id);
    printf("username is %s\n",statement->row.username);
    printf("email is %s\n",statement->row.email);
}

StatementPrepareResult prepare_insert_command(InputBuffer *input_buffer, Statement *statement) {
    statement->statement_type = STATEMENT_INSERT;
    char *keyword = strtok(input_buffer->buffer," ");
    char *id_string = strtok(NULL, " ");
    char *username = strtok(NULL, " ");
    char *email = strtok(NULL, " ");

    if (keyword == NULL || id_string == NULL || username == NULL || email == NULL)
    {
        return STATEMENT_PREPARE_SYNTAX_ERROR;
    }
    int id = atoi(id_string);
    if (id < 0)
    {
        return STATEMENT_PREPARE_NEGATIVE_ID;
    }
    
    if (strlen(username) > COLUMN_USERNAME_SIZE)
    {
        return STATEMENT_PREPARE_PARAM_TOO_LONG;
    }
    if (strlen(email) > COLUMN_USERNAME_SIZE)
    {
        return STATEMENT_PREPARE_PARAM_TOO_LONG;
    }
    statement->row.id = id;
    strcpy(statement->row.username, username);
    strcpy(statement->row.email, email);

    return STATEMENT_PREPARE_SUCCESS;
}

StatementPrepareResult prepare_db_command(InputBuffer *input_buffer, Statement *statement) {
    if (strncmp(input_buffer->buffer,"insert",6) == 0)
    {
        printf("\n input is %s\n",input_buffer->buffer);
        return prepare_insert_command(input_buffer, statement);
    } else if (strncmp(input_buffer->buffer,"select",6) == 0) {
        statement->statement_type = STATEMENT_SELECT;
        return STATEMENT_PREPARE_SUCCESS;
    }
    return STATEMENT_PREPARE_UNRECONGINZED;
}

void seralize_row(Row *row, void *destination) {
    printf("in seralize_row");
    memcpy(destination+ID_OFFSET, &(row->id), ID_SIZE);
    strncpy(destination+USERNAME_OFFSET, row->username, USERNAME_SIZE);
    strncpy(destination+EMAIL_OFFSET, row->email, EMAIL_SIZE);
}

void deserlize_row(void* source, Row *row) {
    memcpy(&(row->id),source+ID_OFFSET, ID_SIZE);
    strncpy(row->username,source+USERNAME_OFFSET, USERNAME_SIZE);
    strncpy(row->email,source+EMAIL_OFFSET, EMAIL_SIZE);
}

void dump_row(Row *row) {
    printf("(id: %d, username: %s, email: %s)\n",row->id,row->username,row->email);
}

Cursor *leaf_node_find(Table *table, uint32_t page_num, uint32_t key) {
    printf("in leaf_node_find 0\n");
    void *node = get_page(table->pager, page_num);
    uint32_t cell_nums = *(leaf_node_num_cells(node));
    printf("in leaf_node_find 1 cell_nums is%d\n",cell_nums);
    Cursor *cursor = (Cursor *)malloc(sizeof(Cursor));
    cursor->page_num = page_num;
    cursor->table = table;
    uint32_t min_cell = 0;
    uint32_t max_cell = cell_nums;
    while(min_cell != max_cell) {
        uint32_t mid_cell = (min_cell+max_cell)/2;
        uint32_t searchKey = *(leaf_node_key(node,mid_cell));
        if (searchKey == key)
        {
            cursor->cell_num = mid_cell;
            return cursor;
        } else if (searchKey<key) {
            min_cell = mid_cell+1;
        } else {
            max_cell = mid_cell;
        }
    }
    cursor->cell_num = min_cell;
    printf("out of leaf_node_find cell_num is%d\n",min_cell);
    return cursor;
}

Cursor *table_find(Table *table, uint32_t key) {
    uint32_t root_page_num = table->root_page_num;
    void* root_node = get_page(table->pager, root_page_num);
    if (*(get_leaf_node_type(root_node)) == NODE_LEAF)
    {
        printf("befor leaf_node_find\n");
        return leaf_node_find(table,root_page_num,key);
        printf("after leaf_node_find\n");
    } else {
        printf("Need to implement searching an internal node\n");
        exit(EXIT_FAILURE);
    }
}

ExecuteResult leaf_node_insert(Cursor *cursor, uint32_t key, Row *value) {
    void *node = get_page(cursor->table->pager,cursor->page_num);
    uint32_t cell_nums = *(leaf_node_num_cells(node));
    if (cell_nums >= LEAF_NODE_MAX_CELLS) {
        return EXECUTE_TABLE_FULL;
    }
    if (cursor->cell_num < cell_nums) {
        for (uint32_t i = cell_nums; i>cursor->cell_num; i--) {
            memcpy(leaf_node_cell(node,i),leaf_node_cell(node,i-1),LEAF_NODE_CELL_SIZE);
        }
    }
    *(leaf_node_num_cells(node)) += 1;
    *(leaf_node_key(node,cursor->cell_num)) = key;
    seralize_row(value,leaf_node_value(node,cursor->cell_num));
    printf("leaf_node_insert success\n");
    return EXECUTE_SUCCESS;
}

ExecuteResult execute_insert_statment(Statement *statement, Table*table) {
    uint32_t root_page_num = table->root_page_num;
    printf("in execute_insert_statment\n");
    void *node = get_page(table->pager,root_page_num);
    printf("root_page_num is %d\n",root_page_num);
    printf("address of node is %ld\n",node);
    printf("address of pages is %ld\n",table->pager->pages[0]);
    uint32_t cell_nums = *(leaf_node_num_cells(node));
    printf("cell_nums is %d\n",cell_nums);
    if (cell_nums >= LEAF_NODE_MAX_CELLS) {
        return EXECUTE_TABLE_FULL;
    }
    uint32_t key_to_insert = statement->row.id;
    Cursor *cursor = table_find(table,key_to_insert);
    // Cursor *cursor = create_cursor_of_end(table);
    printf("after table_find\n");
    if (cursor!=NULL && cursor->cell_num < cell_nums)
    {
        uint32_t key_at_index = *leaf_node_key(node, cursor->cell_num);
        if (key_at_index == key_to_insert) {
          return EXECUTE_DUPLICATE_KEY;
        }
    }
    printf("before leaf_node_insert\n");
    ExecuteResult result = leaf_node_insert(cursor,key_to_insert,&(statement->row));
    if (cursor != NULL)
    {
        free(cursor);
    }
    
    return result;
}

ExecuteResult execute_select_statment(Statement *statment, Table*table) {
    Row row;
    Cursor *cursor = create_cursor_of_start(table);
    while (!cursor->is_end)
    {
        deserlize_row(cursor_value(cursor), &row);
        dump_row(&row);
        cursor_advanced(cursor);
    }
    free(cursor);
    return EXECUTE_SUCCESS;
}

ExecuteResult execute_statement(Statement *statement, Table *table) {
    switch (statement->statement_type)
    {
        case STATEMENT_INSERT:
            return execute_insert_statment(statement, table);
        case STATEMENT_SELECT:
            return execute_select_statment(statement, table);
        default:
            printf("Unrecongized input!\n");
            break;
    }
    return EXECUTE_FAIL;
}

void print_row(Row *row) {
    printf("(Row: %d, %s, %s)\n",row->id,row->username,row->email);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Must supply a database name!\n");
        exit(EXIT_FAILURE);
    }
    char *filename = argv[1];
    Table *table = db_open(filename);
    InputBuffer *input_buffer = new_input_buffer();
    while (1)
    {
        print_prompt();
        read_input(input_buffer);
        if (input_buffer->buffer[0] == '.')
        {
            switch (process_meta_command(input_buffer, table))
            {
                case META_COMMAND_UNRECOGINIZED:
                default:
                    continue;
            }
        }
        Statement statement;
        switch (prepare_db_command(input_buffer, &statement)) {
            case STATEMENT_PREPARE_SUCCESS:
                break;

            case STATEMENT_PREPARE_PARAM_TOO_LONG:
                printf("Command input parameter is too long!\n");
                continue;

            case STATEMENT_PREPARE_NEGATIVE_ID:
                printf("negative id is invalid\n");
                continue;

            case STATEMENT_PREPARE_SYNTAX_ERROR:
                printf("Command failed!\n");
                continue;

            case STATEMENT_PREPARE_UNRECONGINZED:
                printf("Unrecongized input!\n");
                continue;
        }
        
        switch (execute_statement(&statement, table)) {
            case EXECUTE_SUCCESS:
                continue;
            case EXECUTE_DUPLICATE_KEY:
                printf("key duplicated!\n");
                continue;
            case EXECUTE_TABLE_FULL:
                printf("table full!\n");
                continue;
            case EXECUTE_FAIL:
                printf("command executed failed!\n");
                continue;
        }
    }
    free_table(table);
    return 0;
}