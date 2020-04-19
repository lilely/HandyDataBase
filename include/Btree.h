
typedef enum {
    NODE_INTERNAL,
    NODE_LEAF,
} NodeType;

// common node header
extern const uint32_t NODE_TYPE_SIZE;
extern const uint32_t NODE_TYPE_OFFSET;
extern const uint32_t IS_ROOT_SIZE;
extern const uint32_t IS_ROOT_OFFSET;
extern const uint32_t PARENT_POINTER_SIZE;
extern const uint32_t PARENT_POINTER_OFFSET;
extern const uint32_t COMMON_NODE_HEADER_SIZE;

// leaf node header
extern const uint32_t LEAF_NODE_NUM_CELLS_SIZE;
extern const uint32_t LEAF_NODE_NUM_CELLS_OFFSET;
extern const uint32_t LEAF_NODE_HEADER_SIZE;

// leaf node body
extern const uint32_t LEAF_NODE_KEY_SIZE;
extern const uint32_t LEAF_NODE_KEY_OFFSET;
extern const uint32_t LEAF_NODE_VALUE_SIZE;
extern const uint32_t LEAF_NODE_VALUE_OFFSET;
extern const uint32_t LEAF_NODE_CELL_SIZE;
extern const uint32_t LEAF_NODE_SPACE_FOR_CELLS;
extern const uint32_t LEAF_NODE_MAX_CELLS;

extern uint32_t* leaf_node_num_cells(void *node);

extern void *leaf_node_cell(void *node, uint32_t cell_num);

extern uint32_t* leaf_node_key(void *node, uint32_t cell_num);

extern void* leaf_node_value(void *node, uint32_t cell_num);

extern void initialize_leaf_node(void *node);