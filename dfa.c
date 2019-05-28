/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.  If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2013-2018 MonetDB B.V.
 */

// LIKE pattern parser based on Deterministic Finite Automaton architecture.

#include "dfa.h"


#include <gdk.h>
#include <mal_exception.h>

typedef struct state_t state_t;

typedef void transition_function_t(state_t*, const char);

typedef void finalize_function_t(state_t*);


typedef enum state_type {
    MULTI_CHARACTER_WILDCARD,
    SINGLE_CHARACTER_WILDCARD,
    ESCAPE,
    LITERAL
} state_type_t;

struct state_t {
    state_type_t type; // TODO: Remove this redundant state enum.
    char esc_char;
    searchcriterium_t* current;
    char* error_string;
    transition_function_t* handle_percentage;
    transition_function_t* handle_underscore;
    transition_function_t* handle_escape_character;
    transition_function_t* handle_normal_character;
    transition_function_t* to_error;
    finalize_function_t*   finalize;
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

finalize_function_t wildcard_finalize;
finalize_function_t escape_finalize;
finalize_function_t literal_finalize;

const size_t INIT_BUFFER_SIZE = 128;

static searchcriterium_t* create_searchcriterium() {
    searchcriterium_t* first;

    if ( (first = (searchcriterium_t*) GDKmalloc(sizeof(searchcriterium_t)) ) == NULL)  {
        return NULL;
    }

    first->start = 0;
    first->search_string.data = (char*) GDKmalloc(INIT_BUFFER_SIZE * sizeof(char));
    first->search_string_16.data = NULL;
    first->search_string.capacity = INIT_BUFFER_SIZE;
    first->search_string.nbytes = 0;
    first->next = NULL;

    return first;
}

static void set_initial_state(state_t* state, char esc_char) {
    searchcriterium_t* first;

    first = create_searchcriterium();

    if (first == NULL) {
        state->error_string = createException(MAL, "collation.dfa", MAL_MALLOC_FAIL);
        return;
    }

    state->esc_char = esc_char;
    state->current = first;
    state->error_string = NULL;

    state->handle_percentage        = initial_handle_percentage;
    state->handle_underscore        = initial_handle_underscore;
    state->handle_escape_character  = initial_handle_escape;
    state->handle_normal_character  = initial_handle_normal_character;
    state->finalize                 = NULL;
}

static void set_mc_wildcard_state(state_t* state) {
    state->type                     = MULTI_CHARACTER_WILDCARD;

    state->handle_percentage        = mc_wildcard_handle_percentage;
    state->handle_underscore        = mc_wildcard_handle_underscore;
    state->handle_escape_character  = mc_wildcard_handle_escape_character;
    state->handle_normal_character  = mc_wildcard_handle_normal_character;
    state->finalize                 = wildcard_finalize;
}

static void set_sc_wildcard_state(state_t* state) {
    state->type                     = SINGLE_CHARACTER_WILDCARD;

    state->handle_percentage        = sc_wildcard_handle_percentage;
    state->handle_underscore        = sc_wildcard_handle_underscore;
    state->handle_escape_character  = sc_wildcard_handle_escape_character;
    state->handle_normal_character  = sc_wildcard_handle_normal_character;
    state->finalize                 = wildcard_finalize;
}

static void set_escape_state(state_t* state) {
    state->type                     = ESCAPE;

    state->handle_percentage        = escape_handle_normal_character;
    state->handle_underscore        = escape_handle_normal_character;
    state->handle_escape_character  = escape_handle_normal_character;
    state->handle_normal_character  = escape_handle_error;
    state->finalize                 = escape_finalize;
}

static void set_literal_state(state_t* state) {
    state->type                     = LITERAL;

    state->handle_percentage        = literal_handle_percentage;
    state->handle_underscore        = literal_handle_underscore;
    state->handle_escape_character  = literal_handle_escape_character;
    state->handle_normal_character  = literal_handle_normal_character;
    state->finalize                 = literal_finalize;
}

#define CHECK_PERCENTAGE_STATE(state) ({\
    assert(state->current->card == GREATER_OR_EQUAL); \
    assert(state->handle_percentage == mc_wildcard_handle_percentage); \
    assert(state->handle_underscore == mc_wildcard_handle_underscore); \
    assert(state->handle_escape_character == mc_wildcard_handle_escape_character); \
    assert(state->handle_normal_character == mc_wildcard_handle_normal_character); \
    assert(state->finalize == wildcard_finalize); \
})

static void append_character(state_t* state, search_string_t* buffer, const char character) {
    size_t capacity;

    capacity = buffer->capacity;

    if (capacity == buffer->nbytes){
        char* ndata;
        // reallocate string buffer with exponential reallocation strategy.
        capacity = 2 * capacity;
        ndata = GDKrealloc(buffer->data, capacity * sizeof(char));

        if (ndata == NULL) {
            state->error_string = createException(MAL, "collation.dfa", MAL_MALLOC_FAIL);
            return;
        }

        buffer->data = ndata;
        buffer->capacity = capacity;
    }

    buffer->data[buffer->nbytes++] = character;
}

static void compile_from_utf_8_to_utf_16(state_t* state, search_string_t* source, search_string_utf_16_t* destination) {
    UErrorCode status = U_ZERO_ERROR;

    destination->ncharacters = source->nbytes;

	destination->data = (UChar*) GDKmalloc(2 /*double just to be sure*/ * destination->ncharacters * sizeof(UChar));

	u_strFromUTF8Lenient(destination->data, destination->ncharacters, NULL, source->data, source->nbytes, &status);

	if (!U_SUCCESS(status)){
		state->error_string = createException(MAL, "collation.collationlike", "Could not transform pattern string from utf-8 to utf-16.");
	}
}

static void terminate_current_string(state_t* state) {
    append_character(state, &state->current->search_string, '\0');
    if (state->error_string) return;

    compile_from_utf_8_to_utf_16(state, &state->current->search_string, &state->current->search_string_16);
}


void initial_handle_percentage(state_t* state, const char character) {
    (void) character;

    state->current->card = GREATER_OR_EQUAL;
    assert(state->current->start == 0);

    set_mc_wildcard_state(state);
}

void initial_handle_underscore(state_t* state, const char character) {
    (void) character;

    state->current->card = EQUAL;
    assert(state->current->start == 0);
    state->current->start++;

    set_sc_wildcard_state(state);
}

void initial_handle_escape(state_t* state, const char character) {
    (void) character;

    state->current->card = EQUAL;
    assert(state->current->start == 0);

    set_escape_state(state);
}

void initial_handle_normal_character(state_t* state, const char character) {
    searchcriterium_t* searchcriterium;

    (void) character;

    state->current->card = EQUAL;
    assert(state->current->start == 0);

    searchcriterium = state->current;

    append_character(state, &searchcriterium->search_string, character);

    set_literal_state(state);
}

void mc_wildcard_handle_percentage(state_t* state, const char character) {
    (void) character;

    CHECK_PERCENTAGE_STATE(state);
}

void mc_wildcard_handle_underscore(state_t* state, const char character) {
    (void) character;

    CHECK_PERCENTAGE_STATE(state);
    state->current->start++;

    set_sc_wildcard_state(state);
}

void mc_wildcard_handle_escape_character(state_t* state, const char character) {
    (void) character;

    CHECK_PERCENTAGE_STATE(state);

    set_escape_state(state);
}

void mc_wildcard_handle_normal_character(state_t* state, const char character) {
    searchcriterium_t* searchcriterium;

    CHECK_PERCENTAGE_STATE(state);

    searchcriterium = state->current;

    append_character(state, &searchcriterium->search_string, character);

    set_literal_state(state);
}

#define CHECK_UNDERSCORE_STATE(state) ({\
    assert(state->current->start > 0); \
    assert(state->handle_percentage == sc_wildcard_handle_percentage); \
    assert(state->handle_underscore == sc_wildcard_handle_underscore); \
    assert(state->handle_escape_character == sc_wildcard_handle_escape_character); \
    assert(state->handle_normal_character == sc_wildcard_handle_normal_character); \
    assert(state->finalize == wildcard_finalize); \
})

void sc_wildcard_handle_percentage(state_t* state, const char character) {
    (void) character;

    CHECK_UNDERSCORE_STATE(state);

    state->current->card = GREATER_OR_EQUAL;

    set_mc_wildcard_state(state);
}

void sc_wildcard_handle_underscore(state_t* state, const char character) {
    (void) character;

    CHECK_UNDERSCORE_STATE(state);

    state->current->start++;

    set_sc_wildcard_state(state);
}

void sc_wildcard_handle_escape_character(state_t* state, const char character) {
    (void) character;

    CHECK_UNDERSCORE_STATE(state);

    set_escape_state(state);
}

void sc_wildcard_handle_normal_character(state_t* state, const char character) {
    searchcriterium_t* searchcriterium;

    CHECK_UNDERSCORE_STATE(state);

    searchcriterium = state->current;

    append_character(state, &searchcriterium->search_string, character);

    set_literal_state(state);
}

void wildcard_finalize(state_t* state) {

    terminate_current_string(state);
}

#define CHECK_ESCAPE_STATE(state) ({\
    assert(state->handle_percentage == escape_handle_normal_character); \
    assert(state->handle_underscore == escape_handle_normal_character); \
    assert(state->handle_escape_character == escape_handle_normal_character); \
    assert(state->handle_normal_character == escape_handle_error); \
    assert(state->finalize == escape_finalize); \
})

void escape_handle_normal_character(state_t* state, const char character) {
    searchcriterium_t* searchcriterium;

    CHECK_ESCAPE_STATE(state);

    searchcriterium = state->current;

    append_character(state, &searchcriterium->search_string, character);

    set_literal_state(state);
}

void escape_handle_error(state_t* state, const char character) {
    char* message_buffer;

    CHECK_ESCAPE_STATE(state);

    message_buffer = createException(MAL, "collation.dfa", "Non-escapable character: %s.", &character);

    // Setting the error_string in the state signals an exception of the parsing process.
    state->error_string = message_buffer;
}

void escape_finalize(state_t* state) {
    char* message_buffer;

    message_buffer = createException(MAL, "collation.dfa", "Missing actual escape character.");

    // Setting the error_string in the state signals an exception of the parsing process.
    state->error_string = message_buffer;
}

#define CHECK_LITERAL_STATE(state) ({\
    assert(state->handle_percentage == literal_handle_percentage); \
    assert(state->handle_underscore == literal_handle_underscore); \
    assert(state->handle_escape_character == literal_handle_escape_character); \
    assert(state->handle_normal_character == literal_handle_normal_character); \
    assert(state->finalize == literal_finalize); \
})

static void increment_searchcriterium_list(state_t* state) {
    searchcriterium_t* new;

    /*
     * We have reached the end of the current searchcriterium.
     * So we finalize the current one and create a new searchcriterium.
     * Append new searchcriterium to linked list and update state's current accordingly.
     */
    terminate_current_string(state);
    if (state->error_string) return;

    new = create_searchcriterium();

    if (new == NULL) {
        state->error_string = createException(MAL, "collation.dfa", MAL_MALLOC_FAIL);
        return;
    }

    state->current->next = new;
    state->current = new;
}

void literal_handle_percentage(state_t* state, const char character) {
    (void) character;

    CHECK_LITERAL_STATE(state);

    increment_searchcriterium_list(state);
    if (state->error_string) return;

    assert(state->current->start == 0);
    state->current->card = GREATER_OR_EQUAL;

    set_mc_wildcard_state(state);
}

void literal_handle_underscore(state_t* state, const char character) {
    (void) character;

    CHECK_LITERAL_STATE(state);

    increment_searchcriterium_list(state);
    if (state->error_string) return;

    assert(state->current->start == 0);
    state->current->card = EQUAL;
    state->current->start++;

    set_sc_wildcard_state(state);
}

void literal_handle_escape_character(state_t* state, const char character) {
    (void) character;

    CHECK_LITERAL_STATE(state);

    set_escape_state(state);
}

void literal_handle_normal_character(state_t* state, const char character) {
    searchcriterium_t* searchcriterium;

    CHECK_LITERAL_STATE(state);

    searchcriterium = state->current;

    append_character(state, &searchcriterium->search_string, character);

    set_literal_state(state);
}

void literal_finalize(state_t* state) {

    increment_searchcriterium_list(state);
    if (state->error_string) return;

    assert(state->current->start == 0);
    state->current->card = EQUAL;
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

char* create_searchcriteria(searchcriterium_t** searchcriteria, const char* pattern, char esc_char) {
    assert(strlen(pattern) > 0);

    state_t state;

    set_initial_state(&state, esc_char);

    *searchcriteria = state.current; // head of linked list of search string objects.

    for (const char* cursor = pattern; *cursor != '\0'; ++cursor) {
        handle_character(&state, *cursor);

        if (state.error_string) {
            destroy_searchcriteria(*searchcriteria);
            return state.error_string;
        }
    }

    state.finalize(&state);

    if (state.error_string) {
        destroy_searchcriteria(*searchcriteria);
        return state.error_string;
    }

    return MAL_SUCCEED;
}

void destroy_searchcriteria(searchcriterium_t* searchcriteria) {
    searchcriterium_t* next;

    do {
        next = searchcriteria->next;

        GDKfree(searchcriteria->search_string.data);
        GDKfree(searchcriteria->search_string_16.data);
        GDKfree(searchcriteria);

        searchcriteria = next;

    } while(next);
}
