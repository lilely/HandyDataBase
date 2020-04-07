#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define TABLE_MAX_PAGES 100
#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

#define sizeof_attribute(Struct, Attribute) sizeof(((Struct *)0)->Attribute)

const uint32_t ID_SIZE = sizeof_attribute(Row,id);
const uint32_t USERNAME_SIZE = sizeof_attribute(Row,username);
const uint32_t EMAIL_SIZE = sizeof_attribute(Row,email);
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_SIZE;
const uint32_t EMAIL_OFFSET = ID_SIZE + USERNAME_SIZE;

const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE/ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = TABLE_MAX_PAGES*ROWS_PER_PAGE;

typedef struct {
    uint32_t num_rows;
    void *Pages[TABLE_MAX_PAGES];
} Table;

typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGINIZED,
} MetaCommandResult;

typedef enum {
    STATEMENT_PREPARE_SUCCESS,
    STATEMENT_PREPARE_ERROR,
    STATEMENT_PREPARE_UNRECONGINZED,
} StatementPrepareResult;

typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT,
} StatementType;

typedef struct {
    StatementType statement_type;
    Row row;
} Statement;

typedef struct {
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];
} Row;

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

MetaCommandResult process_meta_command(InputBuffer *input_buffer, Table *table) {
    if (strcmp(input_buffer->buffer,".exit") == 0)
    {
        close_inputBuffer(input_buffer);
        free_table(table);
        exit(EXIT_SUCCESS);
    } else {
        return META_COMMAND_UNRECOGINIZED;
    }
}

StatementPrepareResult prepare_db_command(InputBuffer *input_buffer, Statement *statement) {
    if (strncmp(input_buffer->buffer,"insert",6) == 0)
    {
        statement->statement_type = STATEMENT_INSERT;
        return STATEMENT_PREPARE_SUCCESS;
    } else if (strncmp(input_buffer->buffer,"select",6) == 0) {
        statement->statement_type = STATEMENT_SELECT;
        return STATEMENT_PREPARE_SUCCESS;
    }
    return STATEMENT_PREPARE_UNRECONGINZED;
}

void process_statement(InputBuffer *input_buffer, Statement *statement) {
    switch (statement->statement_type)
    {
    case STATEMENT_INSERT:
        printf("This is a insert command!\n");
        break;
    case STATEMENT_SELECT:
        printf("Thit is a select command!\n");
    default:
        printf("Unrecongized input!\n");
        break;
    }
}

void print_row(Row *row) {
    printf("(Row: %d, %s, %s)\n",row->id,row->username,row->email);
}

void seralize_row(Row *row, void *destination) {
    memcpy(destination+ID_OFFSET, row->id);
    memcpy(destination+USERNAME_OFFSET, row->username);
    memcpy(destination+EMAIL_OFFSET, row->email);
}

void deserlize_row(void* source, Row *row) {
    memcpy(&(row->id),source+ID_OFFSET);
    memcpy(&(row->username),source+USERNAME_OFFSET);
    memcpy(&(row->email),source+EMAIL_OFFSET);
}

void *row_slot(Table *table,uint32_t row_num) {
    uint32_t page_num = row_num/ROWS_PER_PAGE;
    uint32_t row_offset = row_num%ROWS_PER_PAGE;
    void *page = table->Pages[page_num];
    if (page == NULL) {
        page = malloc(PAGE_SIZE);
    }
    void *row_slot = page+row_offset*ROW_SIZE;
    return row_slot;
}

Table *new_table() {
    Table *table = malloc(sizeof(Table));
    table->num_rows = 0;
    for (uint32_t i = 0; i<TABLE_MAX_PAGES; i++) {
        table->Pages[i] = NULL;
    }
    return table;
}

void free_table(Table *table){
    for (uint32_t i = 0; i<TABLE_MAX_PAGES; i++) {
        if (table->Pages[i] == NULL)
        {
            continue;
        }
        free(table->Pages[i]);
    }
    free(table);
}

int main(int argc, char *argv[])
{
    InputBuffer *input_buffer = new_input_buffer();
    while (1)
    {
        print_prompt();
        read_input(input_buffer);
        if (input_buffer->buffer[0] == '.')
        {
            switch (process_meta_command(input_buffer))
            {
                case META_COMMAND_SUCCESS:
                    break;
                case META_COMMAND_UNRECOGINIZED:
                default:
                    continue;
            }
        }
        
        Statement statement;
        switch (prepare_db_command(input_buffer, &statement)) {
            case STATEMENT_PREPARE_SUCCESS:
                process_statement(input_buffer, &statement);
                continue;

            case STATEMENT_PREPARE_UNRECONGINZED:
                printf("Unrecongized input!\n");
                continue;
        }
    }
    
    return 0;
}