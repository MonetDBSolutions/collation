/*
 * LIKE pattern parser based on a Deterministic Finite Automaton architecture.
 */

#include <monetdb_config.h>
#include <gdk.h>

typedef char lchar_t;

typedef enum cardinality {
    GREATER_OR_EQUAL,
    EQUAL
} cardinality_t;

typedef struct _searchstring_t {
    int start;
    cardinality_t card;
    lchar_t* string_buffer;
    size_t capacity;
    size_t ncharacters;
    struct _searchstring_t* next;
} searchstring_t;

typedef struct state_t state_t;

typedef state_t* start_function_t(lchar_t*);

start_function_t start_with_percentage;
start_function_t start_with_underscore;
start_function_t start_with_first_escape;
start_function_t start_with_normal_character;

typedef void transition_function_t(state_t*, void*);

struct state_t {
    searchstring_t* current;
    char* error_string;
    transition_function_t* to_percentage;
    transition_function_t* to_underscore;
    transition_function_t* to_first_escape;
    transition_function_t* to_second_escape;
    transition_function_t* to_normal_character;
    transition_function_t* to_error;
};

transition_function_t from_percentage_to_percentage;
transition_function_t from_percentage_to_underscore;
transition_function_t from_percentage_to_first_escape;
transition_function_t from_percentage_to_normal_character;

transition_function_t from_underscore_to_percentage;
transition_function_t from_underscore_to_underscore;
transition_function_t from_underscore_to_first_escape;
transition_function_t from_underscore_to_normal_character;

transition_function_t from_normal_character_to_percentage;
transition_function_t from_normal_character_to_underscore;
transition_function_t from_normal_character_to_first_escape;
transition_function_t from_normal_character_to_normal_character;

transition_function_t from_first_escape_to_percentage;
transition_function_t from_first_escape_to_underscore;
transition_function_t from_first_escape_to_second_escape;
transition_function_t from_first_escape_to_normal_character;
transition_function_t from_first_escape_to_error;

transition_function_t from_second_escape_to_percentage;
transition_function_t from_second_escape_to_underscore;
transition_function_t from_second_escape_to_first_escape;
transition_function_t from_second_escape_to_normal_character;

const size_t INIT_BUFFER_SIZE = 128 * sizeof(lchar_t);

static state_t* create_initial_state() {
    state_t* init_state;
    searchstring_t* first;

    init_state = (state_t*) GDKmalloc(sizeof(state_t));
    first = (searchstring_t*) GDKmalloc(sizeof(searchstring_t));
    first->start = 0;
    first->card = EQUAL;
    first->string_buffer = (lchar_t*) GDKmalloc(INIT_BUFFER_SIZE);
    first->capacity = INIT_BUFFER_SIZE;
    first->ncharacters = 0;
    first->next = NULL;

    init_state->current = first;
    init_state->error_string = NULL;

    return init_state;
}

state_t* start_with_percentage(lchar_t*  character) {
    state_t* init_state;

    (void) character;

    init_state = create_initial_state();
    init_state->current->card = GREATER_OR_EQUAL;
    assert(init_state->current->start == 0);

    return init_state;
}

state_t* start_with_underscore(lchar_t*  character) {
    state_t* init_state;

    (void) character;

    init_state = create_initial_state();
    assert(init_state->current->card == EQUAL);
    assert(init_state->current->start == 0);

    return init_state;
}

state_t* start_with_normal_character(lchar_t*  character) {
    state_t* init_state;

    (void) character;

    init_state = create_initial_state();
    assert(init_state->current->card == EQUAL);
    assert(init_state->current->start == 0);

    return init_state;
}
