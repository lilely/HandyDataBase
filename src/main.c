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

uint32_t internal_node_find_child(void *node, uint32_t key) {
    uint32_t num_keys = internal_node_num_keys(node);
    uint32_t left_index = 0;
    uint32_t right_index = num_keys;
    while (left_index != right_index)
    {
        uint32_t mid_index = (left_index+right_index)/2;
        uint32_t key_to_right = internal_node_key(node,mid_index);
        // 寻找第一个大于等于key的key所在index。
        // 若寻找第一个小于等于key的key所在index，则 
        // if (key_to_right <= key) {
        //    left_index = mid_index;
        // } else {
        //    right_index = mid_index-1;   
        // }
        if (key_to_right >= key)
        {
            right_index = mid_index;
        } else {
            left_index = mid_index+1;
        }
    }
    return left_index;
}

void update_internal_node_key(void* node, uint32_t old_key, uint32_t new_key) {
    uint32_t old_child_index = internal_node_find_child(node, old_key);
    *internal_node_key(node, old_child_index) = new_key;
 }

Cursor *internal_node_find(Table *table, uint32_t page_num, uint32_t key) {
    void *node = get_page(table->pager, page_num);
    uint32_t index = internal_node_find_child(node, key);
    uint32_t next_node_num = *internal_node_child(node,index);
    void *next_node = get_page(table->pager, next_node_num);
    if (get_leaf_node_type(next_node) == NODE_LEAF)
    {
        Cursor *cursor = (Cursor *)malloc(sizeof(Cursor));
        cursor->table = table;
        cursor->page_num = next_node_num;
        cursor->cell_num = leaf_node_num_cells(next_node);
        return cursor;
    } else {
        return internal_node_find(table,next_node_num,key);
    }
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
        return internal_node_find(table,root_page_num,key);
    }
}

Cursor *table_start(Table *table) {
    Cursor *cursor = table_find(table,0);
    void *node = get_page(table->pager,cursor->page_num);
    uint32_t num_cells = *leaf_node_num_cells(node);
    cursor->is_end = (num_cells == 0);

    return cursor;
}

void internal_node_insert(Table *table, uint32_t parent_page_num, uint32_t child_page_num) {
    void *parent = get_page(table,parent_page_num);
    void *child_node = get_page(table,child_page_num);
    uint32_t child_max_key = get_node_max_key(child_node);
    uint32_t index = internal_node_find_child(parent,child_max_key);
    uint32_t original_num_keys = *internal_node_num_keys(parent);
    *internal_node_num_keys(parent) = original_num_keys + 1;
    if(original_num_keys > INTERNAL_NODE_MAX_CELLS) {
        printf("Need spilit internal node!");
        exit(EXIT_FAILURE);
    }
    uint32_t right_child_page_num = *internal_node_right_child(parent);
    void* right_child = get_page(table->pager, right_child_page_num);

    if (child_max_key > get_node_max_key(right_child)) {
        /* Replace right child */
        *internal_node_child(parent, original_num_keys) = right_child_page_num;
        *internal_node_key(parent, original_num_keys) = get_node_max_key(right_child);
        *internal_node_right_child(parent) = child_page_num;
    } else {
        /* Make room for the new cell */
        for (uint32_t i = original_num_keys; i > index; i--) {
          void* destination = internal_node_cell(parent, i);
          void* source = internal_node_cell(parent, i - 1);
          memcpy(destination, source, INTERNAL_NODE_CELL_SIZE);
        }
        *internal_node_child(parent, index) = child_page_num;
        *internal_node_key(parent, index) = child_max_key;
    }
}

ExecuteResult leaf_node_split_and_insert(Cursor *cursor, uint32_t key, Row *value) {
    void *old_node = get_page(cursor,cursor->page_num);
    uint32_t new_page_num = get_unused_page_num(cursor->table);
    void *new_node = get_page(cursor,new_page_num);
    initialize_leaf_node(new_node);
    uint32_t old_node_max_key = get_node_max_key(old_node);
    for(int i = 0; i<LEAF_NODE_MAX_CELLS; i++) {
        void *destination_node = NULL;
        if(i >= LEAF_NODE_LEFT_SPLIT_COUNT) {
            destination_node = new_node;
        } else {
            destination_node = old_node;
        }
        uint32_t cell_num = i%LEAF_NODE_LEFT_SPLIT_COUNT;
        void *destination = leaf_node_cell(destination_node,cell_num);
        if (i == cursor->cell_num)
        {
            seralize_row(value,leaf_node_value(destination_node,cursor->cell_num));
            *leaf_node_key(destination_node,cell_num) = key;
        } else if (i < cursor->cell_num)
        {
            memcpy(destination,leaf_node_cell(old_node,i),LEAF_NODE_CELL_SIZE);
        } else if (i > cursor->cell_num)
        {
            memcpy(destination,leaf_node_cell(old_node,i-1),LEAF_NODE_CELL_SIZE);
        }
    }
    *(leaf_node_num_cells(old_node)) = LEAF_NODE_LEFT_SPLIT_COUNT;
    *(leaf_node_next_leaf(new_node)) = *(leaf_node_next_leaf(old_node));
    *(leaf_node_next_leaf(old_node)) = new_page_num;
    *(leaf_node_num_cells(new_node)) = LEAF_NODE_RIGHT_SPLIT_COUNT;
    
    *node_parent(new_node) = *node_parent(old_node);
    if (is_node_root(old_node))
    {
        create_new_root(cursor->table,new_page_num);
    } else {
        printf("Need to implement updating parent after split\n");
        uint32_t parent_page_num = *node_parent(old_node);
        void *parent_node = get_page(cursor->table,parent_page_num);
        uint32_t new_max = get_node_max_key(old_node);
        update_internal_node_key(parent_node,old_node_max_key,new_max);
        internal_node_insert(cursor->table,parent_page_num,new_page_num);
    }
    
    return EXECUTE_SUCCESS;
}

ExecuteResult leaf_node_insert(Cursor *cursor, uint32_t key, Row *value) {
    void *node = get_page(cursor->table->pager,cursor->page_num);
    uint32_t cell_nums = *(leaf_node_num_cells(node));
    if (cell_nums >= LEAF_NODE_MAX_CELLS) {
        return leaf_node_split_and_insert(cursor, key, value);
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