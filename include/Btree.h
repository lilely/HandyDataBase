
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
extern const uint32_t LEAF_NODE_NEXT_LEAF_SIZE;
extern const uint32_t LEAF_NODE_NEXT_LEAF_OFFSET;
extern const uint32_t LEAF_NODE_HEADER_SIZE;

// leaf node body
extern const uint32_t LEAF_NODE_KEY_SIZE;
extern const uint32_t LEAF_NODE_KEY_OFFSET;
extern const uint32_t LEAF_NODE_VALUE_SIZE;
extern const uint32_t LEAF_NODE_VALUE_OFFSET;
extern const uint32_t LEAF_NODE_CELL_SIZE;
extern const uint32_t LEAF_NODE_SPACE_FOR_CELLS;
extern const uint32_t LEAF_NODE_MAX_CELLS;
extern const uint32_t LEAF_NODE_RIGHT_SPLIT_COUNT;
extern const uint32_t LEAF_NODE_LEFT_SPLIT_COUNT;

// Internal Node Header Layout
extern const uint32_t INTERNAL_NODE_NUM_KEYS_SIZE;
extern const uint32_t INTERNAL_NODE_NUM_KEYS_OFFSET;
extern const uint32_t INTERNAL_NODE_RIGHT_CHILD_SIZE;
extern const uint32_t INTERNAL_NODE_RIGHT_CHILD_OFFSET;
extern const uint32_t INTERNAL_NODE_HEADER_SIZE;

// Internal Node Body Layout
extern const uint32_t INTERNAL_NODE_KEY_SIZE;
extern const uint32_t INTERNAL_NODE_CHILD_SIZE;
extern const uint32_t INTERNAL_NODE_CELL_SIZE;

extern uint8_t is_node_root(void *node);

extern void set_node_root(void *node, uint8_t is_root);

extern uint32_t* leaf_node_num_cells(void *node);

extern uint32_t* leaf_node_next_leaf(void *node);

extern void *leaf_node_cell(void *node, uint32_t cell_num);

extern uint32_t* leaf_node_key(void *node, uint32_t cell_num);

extern void* leaf_node_value(void *node, uint32_t cell_num);

extern void initialize_leaf_node(void *node);

extern void initialize_internal_node(void *node);

extern uint8_t* get_leaf_node_type(void *node);

extern void set_leaf_node_type(void *node, NodeType node_type);

extern uint32_t *internal_node_num_keys(void *node);

extern uint32_t *internal_node_right_child(void *node);

extern uint32_t *internal_node_cell(void *node, uint32_t cell_num);

extern uint32_t *internal_node_child(void *node, uint32_t child_num);

extern uint32_t* internal_node_key(void* node, uint32_t key_num);

extern uint32_t get_node_max_key(void *node);