#include <stdlib.h>
#include "Row.h"
#include "Pager.h"
#include "Btree.h"

// common node header
const uint32_t NODE_TYPE_SIZE = sizeof(uint8_t);
const uint32_t NODE_TYPE_OFFSET = 0;
const uint32_t IS_ROOT_SIZE = sizeof(uint8_t);
const uint32_t IS_ROOT_OFFSET = NODE_TYPE_OFFSET+IS_ROOT_SIZE;
const uint32_t PARENT_POINTER_SIZE = sizeof(uint32_t);
const uint32_t PARENT_POINTER_OFFSET = IS_ROOT_OFFSET + PARENT_POINTER_SIZE;
const uint32_t COMMON_NODE_HEADER_SIZE = NODE_TYPE_SIZE+IS_ROOT_SIZE+PARENT_POINTER_SIZE;

// leaf node header
const uint32_t LEAF_NODE_NUM_CELLS_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_NUM_CELLS_OFFSET = COMMON_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_HEADER_SIZE = COMMON_NODE_HEADER_SIZE+LEAF_NODE_NUM_CELLS_SIZE;

// leaf node body
const uint32_t LEAF_NODE_KEY_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_KEY_OFFSET = 0;
const uint32_t LEAF_NODE_VALUE_SIZE = sizeof(Row);
const uint32_t LEAF_NODE_VALUE_OFFSET = LEAF_NODE_KEY_OFFSET+LEAF_NODE_KEY_SIZE;
const uint32_t LEAF_NODE_CELL_SIZE = LEAF_NODE_KEY_SIZE+LEAF_NODE_VALUE_SIZE;
const uint32_t LEAF_NODE_SPACE_FOR_CELLS = PAGE_SIZE - LEAF_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_MAX_CELLS = LEAF_NODE_SPACE_FOR_CELLS/LEAF_NODE_CELL_SIZE;

uint32_t* leaf_node_num_cells(void *node) {
    return node + LEAF_NODE_NUM_CELLS_OFFSET;
}

void *leaf_node_cell(void *node, uint32_t cell_num) {
    return node + LEAF_NODE_HEADER_SIZE + cell_num*LEAF_NODE_CELL_SIZE;
}

uint32_t* leaf_node_key(void *node, uint32_t cell_num) {
    return leaf_node_cell(node,cell_num);
}

void* leaf_node_value(void *node, uint32_t cell_num) {
    return leaf_node_cell(node,cell_num)+LEAF_NODE_VALUE_OFFSET;
}

uint8_t* get_leaf_node_type(void *node) {
    return (uint8_t *)(node+NODE_TYPE_OFFSET);
}

void set_leaf_node_type(void *node, NodeType node_type) {
    *((uint8_t *)(node+NODE_TYPE_OFFSET)) = node_type;
}

void initialize_leaf_node(void *node){
    printf("in initialize_leaf_node\n");
    set_leaf_node_type(node, NODE_LEAF);
    *leaf_node_num_cells(node) = 0;
}