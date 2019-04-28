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

typedef struct state_t state_t;

typedef state_t* start_function_t(lchar_t*);

start_function_t start_with_percentage;
start_function_t start_with_underscore;
start_function_t start_with_escape;
start_function_t start_with_normal_character;

typedef void transition_function_t(state_t*, void*);

struct state_t {
    searchstring_t* current;
    char* error_string;
    transition_function_t* to_percentage;
    transition_function_t* to_underscore;
    transition_function_t* to_escape;
    transition_function_t* to_normal_character;
    transition_function_t* to_error;
};

transition_function_t from_percentage_to_percentage;
transition_function_t from_percentage_to_underscore;
transition_function_t from_percentage_to_escape;
transition_function_t from_percentage_to_normal_character;

transition_function_t from_underscore_to_percentage;
transition_function_t from_underscore_to_underscore;
transition_function_t from_underscore_to_escape;
transition_function_t from_underscore_to_normal_character;

transition_function_t from_normal_character_to_percentage;
transition_function_t from_normal_character_to_underscore;
transition_function_t from_normal_character_to_escape;
transition_function_t from_normal_character_to_normal_character;

transition_function_t from_escape_to_normal_character;
transition_function_t from_escape_to_error;

const size_t INIT_BUFFER_SIZE = 128;

static state_t* create_initial_state() {
    state_t* init_state;
    searchstring_t* first;

    init_state = (state_t*) GDKmalloc(sizeof(state_t));
    first = (searchstring_t*) GDKmalloc(sizeof(searchstring_t));
    first->start = 0;
    first->card = UNDEFINED;
    first->string_buffer.data = (lchar_t*) GDKmalloc(INIT_BUFFER_SIZE * sizeof(lchar_t));
    first->string_buffer.capacity = INIT_BUFFER_SIZE;
    first->string_buffer.ncharacters = 0;
    first->next = NULL;

    init_state->current = first;
    init_state->error_string = NULL;

    return init_state;
}

static void set_percentage_transition_functions(state_t* state) {
    state->to_percentage        = from_percentage_to_percentage;
    state->to_underscore        = from_percentage_to_underscore;
    state->to_escape            = from_percentage_to_escape;
    state->to_normal_character  = from_percentage_to_normal_character;
    state->to_error             = NULL;
}

static void set_underscore_transition_functions(state_t* state) {
    state->to_percentage        = from_underscore_to_percentage;
    state->to_underscore        = from_underscore_to_underscore;
    state->to_escape            = from_underscore_to_escape;
    state->to_normal_character  = from_underscore_to_normal_character;
    state->to_error             = NULL;
}

static void set_escape_transition_functions(state_t* state) {
    state->to_percentage        = NULL;
    state->to_underscore        = NULL;
    state->to_escape            = NULL;
    state->to_normal_character  = from_escape_to_normal_character;
    state->to_error             = from_escape_to_error;
}

static void set_normal_character_transition_functions(state_t* state) {
    state->to_percentage        = from_normal_character_to_percentage;
    state->to_underscore        = from_normal_character_to_underscore;
    state->to_escape            = from_normal_character_to_escape;
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

static void append_character(string_buffer_t* buffer, const lchar_t character) {
    size_t capacity;

    capacity = buffer->capacity;

    if (capacity == ++buffer->ncharacters){
        // reallocate string buffer with exponential reallocation strategy.
        capacity = 2 * capacity;
        buffer->data = (lchar_t*) GDKrealloc(buffer->data, capacity * sizeof(lchar_t));
        buffer->capacity = capacity;
    }

    buffer->data[buffer->ncharacters] = character;
}

#define CHECK_PERCENTAGE_STATE(state) ({\
    assert(state->error_string == NULL); \
    assert(state->current->card == GREATER_OR_EQUAL); \
    assert(state->to_percentage == from_percentage_to_percentage); \
    assert(state->to_underscore == from_percentage_to_underscore); \
    assert(state->to_escape == from_percentage_to_escape); \
    assert(state->to_normal_character == from_percentage_to_normal_character); \
    assert(state->to_error == NULL); \
})

void from_percentage_to_percentage(state_t* state, void* data) {
    (void) data;

    CHECK_PERCENTAGE_STATE(state);
}

void from_percentage_to_underscore(state_t* state, void* data) {
    (void) data;

    CHECK_PERCENTAGE_STATE(state);
    state->current->start++;

    set_underscore_transition_functions(state);
}

void from_percentage_to_escape(state_t* state, void* data) {
    (void) data;

    CHECK_PERCENTAGE_STATE(state);

    set_escape_transition_functions(state);
}

void from_percentage_to_normal_character(state_t* state, void* data) {
    lchar_t character;
    searchstring_t* search_string;

    CHECK_PERCENTAGE_STATE(state);

    character = *(lchar_t*) data;
    search_string = state->current;

    append_character(&search_string->string_buffer, character);

    set_normal_character_transition_functions(state);
}

#define CHECK_UNDERSCORE_STATE(state) ({\
    assert(state->error_string == NULL); \
    assert(state->current->start > 0); \
    assert(state->to_percentage == from_underscore_to_percentage); \
    assert(state->to_underscore == from_underscore_to_underscore); \
    assert(state->to_escape == from_underscore_to_escape); \
    assert(state->to_normal_character == from_underscore_to_normal_character); \
    assert(state->to_error == NULL); \
})

void from_underscore_to_percentage(state_t* state, void* data) {
    (void) data;

    CHECK_UNDERSCORE_STATE(state);

    state->current->card = GREATER_OR_EQUAL;

    set_percentage_transition_functions(state);
}

void from_underscore_to_underscore(state_t* state, void* data) {
    (void) data;

    CHECK_UNDERSCORE_STATE(state);

    state->current->start++;

    set_underscore_transition_functions(state);
}

void from_underscore_to_escape(state_t* state, void* data) {
    (void) data;

    CHECK_UNDERSCORE_STATE(state);

    set_escape_transition_functions(state);
}

void from_underscore_to_normal_character(state_t* state, void* data) {
    lchar_t character;
    searchstring_t* search_string;

    CHECK_UNDERSCORE_STATE(state);

    character = *(lchar_t*) data;
    search_string = state->current;

    append_character(&search_string->string_buffer, character);

    set_normal_character_transition_functions(state);
}

#define CHECK_ESCAPE_STATE(state) ({\
    assert(state->error_string == NULL); \
    assert(state->to_percentage == NULL); \
    assert(state->to_underscore == NULL); \
    assert(state->to_escape == NULL); \
    assert(state->to_normal_character == from_escape_to_normal_character); \
    assert(state->to_error == from_escape_to_error); \
})

void from_escape_to_normal_character(state_t* state, void* data) {
    lchar_t character;
    searchstring_t* search_string;

    CHECK_ESCAPE_STATE(state);

    character = *(lchar_t*) data;
    search_string = state->current;

    append_character(&search_string->string_buffer, character);

    set_normal_character_transition_functions(state);
}

void from_escape_to_error(state_t* state, void* data) {
    // lchar_t character;
    char* message_buffer;

    CHECK_ESCAPE_STATE(state);

    (void) data;
    // TODO: think of how to insert character in error string.
    // character = *(lchar_t*) data;

    const char* error = "Non-escapable character.";

    message_buffer = (char*) GDKmalloc(strlen(error) + 1);

    // Setting the error_string in the state signals an exception of the parsing process.
    state->error_string = message_buffer;
}
