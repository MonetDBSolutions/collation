#include "dfa.h"

#include <gdk.h>

typedef struct state_t state_t;

typedef void transition_function_t(state_t*, const char);

typedef enum state_type {
    MULTI_CHARACTER_WILDCARD,
    SINGLE_CHARACTER_WILDCARD,
    ESCAPE,
    LITERAL
} state_type_t;

struct state_t {
    state_type_t type;
    char esc_char;
    searchstring_t* current;
    char* error_string;
    transition_function_t* handle_percentage;
    transition_function_t* handle_underscore;
    transition_function_t* handle_escape_character;
    transition_function_t* handle_normal_character;
    transition_function_t* to_error;
};

transition_function_t initial_handle_percentage;
transition_function_t initial_handle_underscore;
transition_function_t initial_handle_escape;
transition_function_t initial_handle_normal_character;

transition_function_t mc_wildcard_handle_percentage;
transition_function_t mc_wildcard_handle_underscore;
transition_function_t mc_wildcard_handle_escape_character;
transition_function_t mc_wildcard_handle_normal_character;

transition_function_t sc_wildcard_handle_percentage;
transition_function_t sc_wildcard_handle_underscore;
transition_function_t sc_wildcard_handle_escape_character;
transition_function_t sc_wildcard_handle_normal_character;

transition_function_t literal_handle_percentage;
transition_function_t literal_handle_underscore;
transition_function_t literal_handle_escape_character;
transition_function_t literal_handle_normal_character;

transition_function_t escape_handle_normal_character;
transition_function_t escape_handle_error;

const size_t INIT_BUFFER_SIZE = 128;

static searchstring_t* create_string_buffer() {
    searchstring_t* first;

    first = (searchstring_t*) GDKmalloc(sizeof(searchstring_t));
    first->start = 0;
    first->string_buffer.data = (char*) GDKmalloc(INIT_BUFFER_SIZE * sizeof(char));
    first->string_buffer.capacity = INIT_BUFFER_SIZE;
    first->string_buffer.ncharacters = 0;
    first->next = NULL;
}

static void set_initial_state(state_t* state, char esc_char) {
    searchstring_t* first;

    first = create_string_buffer();

    state->esc_char = esc_char;
    state->current = first;
    state->error_string = NULL;

    state->handle_percentage        = initial_handle_percentage;
    state->handle_underscore        = initial_handle_underscore;
    state->handle_escape_character  = initial_handle_escape;
    state->handle_normal_character  = initial_handle_normal_character;
    state->to_error                 = NULL;
}

static void set_mc_wildcard_state(state_t* state) {
    state->type                     = MULTI_CHARACTER_WILDCARD;

    state->handle_percentage        = mc_wildcard_handle_percentage;
    state->handle_underscore        = mc_wildcard_handle_underscore;
    state->handle_escape_character  = mc_wildcard_handle_escape_character;
    state->handle_normal_character  = mc_wildcard_handle_normal_character;
    state->to_error                 = NULL;
}

static void set_sc_wildcard_state(state_t* state) {
    state->type                     = SINGLE_CHARACTER_WILDCARD;

    state->handle_percentage        = sc_wildcard_handle_percentage;
    state->handle_underscore        = sc_wildcard_handle_underscore;
    state->handle_escape_character  = sc_wildcard_handle_escape_character;
    state->handle_normal_character  = sc_wildcard_handle_normal_character;
    state->to_error                 = NULL;
}

static void set_escape_state(state_t* state) {
    state->type                     = ESCAPE;

    state->handle_percentage        = escape_handle_normal_character;
    state->handle_underscore        = escape_handle_normal_character;
    state->handle_escape_character  = escape_handle_normal_character;
    state->handle_normal_character  = NULL;
    state->to_error                 = escape_handle_error;
}

static void set_literal_state(state_t* state) {
    state->type                     = LITERAL;

    state->handle_percentage        = literal_handle_percentage;
    state->handle_underscore        = literal_handle_underscore;
    state->handle_escape_character  = literal_handle_escape_character;
    state->handle_normal_character  = literal_handle_normal_character;
    state->to_error                 = NULL;
}

#define CHECK_PERCENTAGE_STATE(state) ({\
    assert(state->error_string == NULL); \
    assert(state->current->card == GREATER_OR_EQUAL); \
    assert(state->handle_percentage == mc_wildcard_handle_percentage); \
    assert(state->handle_underscore == mc_wildcard_handle_underscore); \
    assert(state->handle_escape_character == mc_wildcard_handle_escape_character); \
    assert(state->handle_normal_character == mc_wildcard_handle_normal_character); \
    assert(state->to_error == NULL); \
})

static void append_character(string_buffer_t* buffer, const char character) {
    size_t capacity;

    capacity = buffer->capacity;

    if (capacity == buffer->ncharacters){
        // reallocate string buffer with exponential reallocation strategy.
        capacity = 2 * capacity;
        buffer->data = (char*) GDKrealloc(buffer->data, capacity * sizeof(char));
        buffer->capacity = capacity;
    }

    buffer->data[buffer->ncharacters++] = character;
}

static state_t* create_initial_state(char esc_char) {
    state_t* init_state;

    init_state = (state_t*) GDKmalloc(sizeof(state_t));

    set_initial_state(init_state, esc_char);

    return init_state;
}

void initial_handle_percentage(state_t* state, const char  character) {
    (void) character;

    state->current->card = GREATER_OR_EQUAL;
    assert(state->current->start == 0);

    set_mc_wildcard_state(state);
}

void initial_handle_underscore(state_t* state, const char  character) {
    (void) character;

    state->current->card = EQUAL;
    assert(state->current->start == 0);
    state->current->start++;

    set_sc_wildcard_state(state);
}

void initial_handle_escape(state_t* state, const char  character) {
    (void) character;

    state->current->card = EQUAL;
    assert(state->current->start == 0);

    set_escape_state(state);
}

// TODO all these const char* pointers can be normal copy parameters
void initial_handle_normal_character(state_t* state, const char  character) {
    searchstring_t* search_string;

    (void) character;

    state->current->card = EQUAL;
    assert(state->current->start == 0);

    search_string = state->current;

    append_character(&search_string->string_buffer, character);

    set_literal_state(state);
}

void mc_wildcard_handle_percentage(state_t* state, const char  character) {
    (void) character;

    CHECK_PERCENTAGE_STATE(state);
}

void mc_wildcard_handle_underscore(state_t* state, const char  character) {
    (void) character;

    CHECK_PERCENTAGE_STATE(state);
    state->current->start++;

    set_sc_wildcard_state(state);
}

void mc_wildcard_handle_escape_character(state_t* state, const char  character) {
    (void) character;

    CHECK_PERCENTAGE_STATE(state);

    set_escape_state(state);
}

void mc_wildcard_handle_normal_character(state_t* state, const char  character) {
    searchstring_t* search_string;

    CHECK_PERCENTAGE_STATE(state);

    search_string = state->current;

    append_character(&search_string->string_buffer, character);

    set_literal_state(state);
}

#define CHECK_UNDERSCORE_STATE(state) ({\
    assert(state->error_string == NULL); \
    assert(state->current->start > 0); \
    assert(state->handle_percentage == sc_wildcard_handle_percentage); \
    assert(state->handle_underscore == sc_wildcard_handle_underscore); \
    assert(state->handle_escape_character == sc_wildcard_handle_escape_character); \
    assert(state->handle_normal_character == sc_wildcard_handle_normal_character); \
    assert(state->to_error == NULL); \
})

void sc_wildcard_handle_percentage(state_t* state, const char  character) {
    (void) character;

    CHECK_UNDERSCORE_STATE(state);

    state->current->card = GREATER_OR_EQUAL;

    set_mc_wildcard_state(state);
}

void sc_wildcard_handle_underscore(state_t* state, const char  character) {
    (void) character;

    CHECK_UNDERSCORE_STATE(state);

    state->current->start++;

    set_sc_wildcard_state(state);
}

void sc_wildcard_handle_escape_character(state_t* state, const char  character) {
    (void) character;

    CHECK_UNDERSCORE_STATE(state);

    set_escape_state(state);
}

void sc_wildcard_handle_normal_character(state_t* state, const char  character) {
    searchstring_t* search_string;

    CHECK_UNDERSCORE_STATE(state);

    search_string = state->current;

    append_character(&search_string->string_buffer, character);

    set_literal_state(state);
}

#define CHECK_ESCAPE_STATE(state) ({\
    assert(state->error_string == NULL); \
    assert(state->handle_percentage == escape_handle_normal_character); \
    assert(state->handle_underscore == escape_handle_normal_character); \
    assert(state->handle_escape_character == escape_handle_normal_character); \
    assert(state->handle_normal_character == NULL); \
    assert(state->to_error == escape_handle_error); \
})

void escape_handle_normal_character(state_t* state, const char  character) {
    searchstring_t* search_string;

    CHECK_ESCAPE_STATE(state);

    search_string = state->current;

    append_character(&search_string->string_buffer, character);

    set_literal_state(state);
}

void escape_handle_error(state_t* state, const char  character) {
    // char character;
    char* message_buffer;

    CHECK_ESCAPE_STATE(state);

    (void) character;
    // TODO: think of how to insert character in error string.
    // character = *(char*) data;

    char* error = "Non-escapable character.";

    message_buffer = (char*) GDKmalloc(strlen(error) + 1);

    // Setting the error_string in the state signals an exception of the parsing process.
    state->error_string = message_buffer;
}

#define CHECK_LITERAL_STATE(state) ({\
    assert(state->error_string == NULL); \
    assert(state->handle_percentage == literal_handle_percentage); \
    assert(state->handle_underscore == literal_handle_underscore); \
    assert(state->handle_escape_character == literal_handle_escape_character); \
    assert(state->handle_normal_character == literal_handle_normal_character); \
    assert(state->to_error == NULL); \
})

static void increment_searchstring_list(state_t* state) {
    searchstring_t* new;

    /*
     * We have reached the end of the current searchstring.
     * So we finalize the current one with a null terminator and create a new searchstring.
     * Append new searchstring to linked list and update state's current accordingly.
     */
    append_character(&state->current->string_buffer, '\0');

    new = create_string_buffer();
    state->current->next = new;
    state->current = new;
}

void literal_handle_percentage(state_t* state, const char  character) {
    (void) character;

    CHECK_LITERAL_STATE(state);

    increment_searchstring_list(state);

    assert(state->current->start == 0);
    state->current->card = GREATER_OR_EQUAL;

    set_mc_wildcard_state(state);
}

void literal_handle_underscore(state_t* state, const char  character) {
    (void) character;
    searchstring_t* new;

    CHECK_LITERAL_STATE(state);

    increment_searchstring_list(state);

    assert(state->current->start == 0);
    state->current->card = EQUAL;
    state->current->start++;

    set_sc_wildcard_state(state);
}

void literal_handle_escape_character(state_t* state, const char  character) {
    (void) character;

    CHECK_LITERAL_STATE(state);

    set_escape_state(state);
}

void literal_handle_normal_character(state_t* state, const char  character) {
    searchstring_t* search_string;

    CHECK_LITERAL_STATE(state);

    search_string = state->current;

    append_character(&search_string->string_buffer, character);

    set_literal_state(state);
}

static void handle_character(state_t* state, const char cursor) {

    if ( state->esc_char ==  cursor)
        return state->handle_escape_character(state, cursor);

    switch (cursor) {
        case '%':
            return state->handle_percentage(state, cursor);
        case '_':
            return state->handle_underscore(state, cursor);
        default:
            return state->handle_normal_character(state, cursor);
    }
}

searchstring_t* create_searchstring_list(const char* pattern, size_t length, char esc_char) {
    assert(length > 0);

    const char* cursor = pattern;

    state_t* state = create_initial_state(esc_char);

    searchstring_t* search_strings = state->current; // head of linked list of search string objects.

    for (size_t i = 0; i < length && state->error_string == NULL; i++) {
        handle_character(state, *cursor);
        ++cursor;
    }

    // Finalize final pattern chunk. Perhaps we need to a bit more here later on...
    append_character(&state->current->string_buffer, '\0');

    return search_strings;
}
