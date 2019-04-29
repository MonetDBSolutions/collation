/*
 * LIKE pattern parser based on Deterministic Finite Automaton architecture.
 */
#include <monetdb_config.h>

typedef char lchar_t;

typedef enum cardinality {
    GREATER_OR_EQUAL,
    EQUAL
} cardinality_t;

typedef struct {
    lchar_t* data;
    size_t capacity;
    size_t ncharacters;
} string_buffer_t;

typedef struct _searchstring_t {
    int start;
    cardinality_t card;
    string_buffer_t string_buffer;
    struct _searchstring_t* next;
} searchstring_t;

searchstring_t* create_searchstring_list(lchar_t* pattern, size_t length, lchar_t* esc_char);
