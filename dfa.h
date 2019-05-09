/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.  If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2013-2018 MonetDB B.V.
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

char* create_searchcriteria(searchcriterium_t** searchcriteria, const char* pattern, char esc_char);

void destroy_searchcriteria(searchcriterium_t* searchcriteria);
