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
} search_string_t;

typedef struct _searchcriterium_t {
    int start;
    cardinality_t card;
    search_string_t search_string;
    struct _searchcriterium_t* next;
} searchcriterium_t;

searchcriterium_t* create_searchcriteria(const char* pattern, char esc_char);
