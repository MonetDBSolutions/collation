/*
 * LIKE pattern parser based on Deterministic Finite Automaton architecture.
 */

#include <monetdb_config.h>
#include <gdk.h>

typedef char lchar_t;

typedef enum cardinality {
    UNDEFINED,
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
transition_function_t from_first_escape_to_error;

transition_function_t from_second_escape_to_percentage;
transition_function_t from_second_escape_to_underscore;
transition_function_t from_second_escape_to_first_escape;
transition_function_t from_second_escape_to_normal_character;

const size_t INIT_BUFFER_SIZE = 128;

static state_t* create_initial_state() {
    state_t* init_state;
    searchstring_t* first;

    init_state = (state_t*) GDKmalloc(sizeof(state_t));
    first = (searchstring_t*) GDKmalloc(sizeof(searchstring_t));
    first->start = 0;
    first->card = UNDEFINED;
    first->string_buffer = (lchar_t*) GDKmalloc(INIT_BUFFER_SIZE * sizeof(lchar_t));
    first->capacity = INIT_BUFFER_SIZE;
    first->ncharacters = 0;
    first->next = NULL;

    init_state->current = first;
    init_state->error_string = NULL;

    return init_state;
}

static void set_percentage_transition_functions(state_t* state) {
    state->to_percentage        = from_percentage_to_percentage;
    state->to_underscore        = from_percentage_to_underscore;
    state->to_first_escape      = from_percentage_to_first_escape;
    state->to_second_escape     = NULL;
    state->to_normal_character  = from_percentage_to_normal_character;
    state->to_error             = NULL;
}

static void set_underscore_transition_functions(state_t* state) {
    state->to_percentage        = from_underscore_to_percentage;
    state->to_underscore        = from_underscore_to_underscore;
    state->to_first_escape      = from_underscore_to_first_escape;
    state->to_second_escape     = NULL;
    state->to_normal_character  = from_underscore_to_normal_character;
    state->to_error             = NULL;
}

static void set_first_escape_transition_functions(state_t* state) {
    state->to_percentage        = from_first_escape_to_percentage;
    state->to_underscore        = from_first_escape_to_underscore;
    state->to_first_escape      = NULL;
    state->to_second_escape     = from_first_escape_to_second_escape;
    state->to_normal_character  = NULL;
    state->to_error             = from_first_escape_to_error;
}

static void set_normal_character_transition_functions(state_t* state) {
    state->to_percentage        = from_normal_character_to_percentage;
    state->to_underscore        = from_normal_character_to_underscore;
    state->to_first_escape      = from_normal_character_to_first_escape;
    state->to_second_escape     = NULL;
    state->to_normal_character  = from_normal_character_to_normal_character;
    state->to_error             = NULL;
}

state_t* start_with_percentage(lchar_t*  character) {
    state_t* init_state;

    (void) character;

    init_state = create_initial_state();
    init_state->current->card = GREATER_OR_EQUAL;
    assert(init_state->current->start == 0);

    set_percentage_transition_functions(init_state);

    return init_state;
}

state_t* start_with_underscore(lchar_t*  character) {
    state_t* init_state;

    (void) character;

    init_state = create_initial_state();
    init_state->current->card = EQUAL;
    assert(init_state->current->start == 0);
    init_state->current->start++;

    set_underscore_transition_functions(init_state);

    return init_state;
}

state_t* start_with_normal_character(lchar_t*  character) {
    state_t* init_state;

    (void) character;

    init_state = create_initial_state();
    init_state->current->card = EQUAL;
    assert(init_state->current->start == 0);

    return init_state;
}
