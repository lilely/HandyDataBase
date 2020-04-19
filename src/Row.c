#include "Row.h"

const uint32_t ID_SIZE = sizeof_attribute(Row,id);
const uint32_t USERNAME_SIZE = sizeof_attribute(Row,username);
const uint32_t EMAIL_SIZE = sizeof_attribute(Row,email);
// const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_SIZE;
const uint32_t EMAIL_OFFSET = ID_SIZE + USERNAME_SIZE;