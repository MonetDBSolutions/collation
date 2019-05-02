/*
 * LIKE pattern parser based on Deterministic Finite Automaton architecture.
 */
#include <monetdb_config.h>

typedef enum cardinality {
    GREATER_OR_EQUAL,
    EQUAL
} cardinality_t;

typedef struct {
    char* data;
    size_t capacity;
    size_t nbytes;
} string_buffer_t;

typedef struct _searchstring_t {
    int start;
    cardinality_t card;
    string_buffer_t string_buffer;
    struct _searchstring_t* next;
} searchstring_t;

searchstring_t* create_searchstring_list(const char* pattern, char esc_char);
