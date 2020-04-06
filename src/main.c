#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGINIZED,
} MetaCommandResult;

typedef enum {
    STATEMENT_PREPARE_SUCCESS,
    STATEMENT_PREPARE_UNRECONGINZED,
} StatementPrepareResult;

typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT,
} StatementType;

typedef struct {
    StatementType statement_type;
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

MetaCommandResult process_meta_command(InputBuffer *input_buffer) {
    if (strcmp(input_buffer->buffer,".exit") == 0)
    {
        close_inputBuffer(input_buffer);
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