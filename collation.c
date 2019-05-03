/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.  If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2013-2018 MonetDB B.V.
 */

/* monetdb_config.h must be included as the first include file */
#include <monetdb_config.h>

#include <blob.h>
#include <mal_exception.h>

/* system include files */
#include <string.h>
#include <locale.h>

/*ICU includes*/
#include <unicode/ucol.h>
#include <unicode/usearch.h>
#include <unicode/ustring.h>

#include "dfa.h"

/* __declspec() must be used on Windows, but not on other systems */
#ifndef _MSC_VER
/* not Windows */
#define __declspec(x)	/* nothing */
#endif

extern __declspec(dllexport) char *UDFstrxfrm(blob **result, const char **input, const char **locale_id);
extern __declspec(dllexport) char *UDFBATstrxfrm(bat *result, const bat *input, const bat * locale);
extern __declspec(dllexport) char *UDFlikematch(bit* result, const char **pattern, const char **u_target, const char** locale_id);
extern __declspec(dllexport) char *UDFsearch(bit* result, const char **pattern, const char **u_target, const char** locale_id);

static char* first_search(UStringSearch** search, int offset, const char* pattern, const UChar* u_target, UCollator* col) {
	UErrorCode status = U_ZERO_ERROR;
	char* return_status;

	size_t pattern_capacity = strlen(pattern) + 1;
	UChar u_pattern[pattern_capacity];
	u_strFromUTF8Lenient(u_pattern, pattern_capacity, NULL, pattern, -1, &status);

	if (!U_SUCCESS(status)){
		throw(MAL, "icu.collationlike", "Could not transform pattern string from utf-8 to utf-16.");
	}

	*search = usearch_openFromCollator(u_pattern, -1, u_target, -1, col, NULL, &status);

	if (!U_SUCCESS(status)){
		usearch_close(*search);
		throw(MAL, "icu.collationlike", "Could not instantiate ICU string search.");
	}

	usearch_following(*search, offset, &status);

	if (!U_SUCCESS(status)){
		usearch_close(*search);
		throw(MAL, "icu.collationlike", "ICU string search failed.");
	}

	return MAL_SUCCEED;
}

static char* next_search(UStringSearch* search) {
	UErrorCode status = U_ZERO_ERROR;
	usearch_next(search, &status);

	if (!U_SUCCESS(status)){
		usearch_close(search);
		throw(MAL, "icu.collationlike", "ICU next string search failed.");
	}

	return MAL_SUCCEED;
}

static char * collationlike(bit* found, const char *pattern, const char *target, UCollator* coll) {
	UErrorCode status = U_ZERO_ERROR;
	UStringSearch* search;
	char* return_status;

	size_t target_capacity = strlen(target) + 1;
	UChar u_target[target_capacity];
	u_strFromUTF8Lenient(u_target, target_capacity, NULL, target, -1, &status);

	if (!U_SUCCESS(status)){
		throw(MAL, "icu.collationlike", "Could not transform target string from utf-8 to utf-16.");
	}

	if (return_status = first_search(&search, 0, pattern, u_target, coll))
		return return_status;

	*found =  usearch_getMatchedStart(search) != USEARCH_DONE;

	usearch_close(search);

	return MAL_SUCCEED;
}

char *
UDFsearch(bit* result, const char** pattern, const char** target, const char** locale_id) {
	UErrorCode status = U_ZERO_ERROR;
	UCollator* coll;
	UStringSearch* search;
	char* return_status;

	if (pattern == NULL) // NULL pattern implies false match
		return MAL_SUCCEED;

	if (target == NULL) // NULL target implies false match
		return MAL_SUCCEED;

	if (!strlen(*pattern)) // empty pattern implies false match
		return MAL_SUCCEED;

	coll = ucol_open(*locale_id, &status);

	if (!U_SUCCESS(status)){
		ucol_close(coll);
		throw(MAL, "icu.collationlike", "Could not create ICU collator.");
	}

	ucol_setStrength(coll, UCOL_PRIMARY);

	return_status = collationlike(result, *pattern, *target, coll);

	ucol_close(coll);

	return return_status;
}

static char* likematch_recursive(bit* found, searchcriterium_t* current, int offset, const UChar* target, const int nunits, UCollator* coll) {
	UStringSearch* search;
	int pos;
	char* return_status;

	*found = false;

	if (strlen(current->search_string.data)) {
		if (return_status = first_search(&search, offset, current->search_string.data, target, coll)) {
			return return_status;
		}
	}
	else {
		// last part of pattern.
		*found = nunits >= (offset + current->start);

		if (current->card == EQUAL) {
			// Expect this to happen less frequent hence it is in a branch.
			*found = nunits == (offset + current->start);
		}

		return MAL_SUCCEED;
	}

	do {
		pos = usearch_getMatchedStart(search);

		if (pos == USEARCH_DONE) {
			// Current part of pattern not found.
			break;
		}

		*found = pos >= (offset + current->start);

		if (current->card == EQUAL) {
			// Expect this to happen less frequent hence it is in a branch.
			*found = pos == (offset + current->start);
		}

		if (*found && current->next) {
			int next_offset =  pos + usearch_getMatchedLength(search);

			if (return_status = likematch_recursive(found, current->next, next_offset, target, nunits, coll)) {
				usearch_close(search);
				return return_status;
			}
		}

		if(return_status = next_search(search))
			return return_status;

	} while(!*found);

	usearch_close(search);

	return MAL_SUCCEED;
}

char *
UDFlikematch(bit* result, const char** pattern, const char** target, const char** locale_id) {
	UErrorCode status = U_ZERO_ERROR;
	UCollator* coll;
	UStringSearch* search;
	char* return_status;
	searchcriterium_t* head;

	*result = false;

	if (pattern == NULL) // NULL pattern implies false match
		return MAL_SUCCEED;

	if (target == NULL) // NULL target implies false match
		return MAL_SUCCEED;

	if (!strlen(*pattern)) // empty pattern implies false match
		return MAL_SUCCEED;

	coll = ucol_open(*locale_id, &status);

	if (!U_SUCCESS(status)) {
		ucol_close(coll);
		throw(MAL, "icu.collationlike", "Could not create ICU collator.");
	}

	ucol_setStrength(coll, UCOL_PRIMARY);

	 if (return_status = create_searchcriteria(&head, *pattern, '\\')) {
		 return return_status;
	 }

	size_t target_capacity = strlen(*target) + 1;
	UChar u_target[target_capacity];
	int nunits;
	u_strFromUTF8Lenient(u_target, target_capacity, &nunits, *target, -1, &status);

	if (!U_SUCCESS(status)){
		throw(MAL, "icu.collationlike", "Could not transform target string from utf-8 to utf-16.");
	}

	return_status = likematch_recursive(result, head, 0, u_target, nunits, coll);

	destroy_searchcriteria(head);

	return return_status;
}

const size_t DEFAULT_MAX_STRING_KEY_SIZE = 128;

static size_t
do_strxfrm(char *dest, const char *source, size_t len, locale_t locale) {
    
    return strxfrm_l(dest, source, len, locale)  + /* for null terminator*/ 1;
}

char *
UDFstrxfrm(blob **result, const char **input, const char **locale_id)
{
	blob* dest;
	size_t len, nbytes;
	locale_t locale;

	locale = newlocale(LC_COLLATE_MASK, *locale_id,( locale_t)0);

	len = do_strxfrm(NULL, *input, 0, locale);
	nbytes = blobsize(len);
	dest = *result = GDKmalloc(nbytes);
	if (dest == NULL)
		throw(MAL, "collation.strxfrm", MAL_MALLOC_FAIL);

	dest->nitems =len;

	do_strxfrm(dest->data, *input, len, locale);

	return MAL_SUCCEED;
}

char *
UDFBATstrxfrm_cst(bat *result, const bat *input, const char **locale_str)
{
	BAT *input_bat, *result_bat;
	BATiter bi;
	blob *dest;
	BUN p, q;
	const char *source;
	size_t len, max_len, nbytes;
	locale_t locale;

	locale = newlocale(LC_COLLATE_MASK, *locale_str, (locale_t)0);
	/* allocate temporary space for transformed strings; we grow this
	 * if we need more */
	max_len = DEFAULT_MAX_STRING_KEY_SIZE;
	nbytes = blobsize(max_len);
	dest = GDKmalloc(nbytes);
	if (dest == NULL)
		throw(MAL, "collation.strxfrm", MAL_MALLOC_FAIL);

	input_bat = BATdescriptor(*input);
	if (input_bat == NULL) {
		GDKfree(dest);
		throw(MAL, "collation.strxfrm", RUNTIME_OBJECT_MISSING);
	}

	/* we should only get called for string BATs */
	assert(input_bat->ttype == TYPE_str);

	/* allocate result BAT */
	result_bat = COLnew(input_bat->hseqbase, TYPE_blob, BATcount(input_bat), TRANSIENT);
	if (result_bat == NULL) {
		BBPunfix(input_bat->batCacheid);
		GDKfree(dest);
		throw(MAL, "collation.strxfrm", MAL_MALLOC_FAIL);
	}

	/* loop through BAT input_bat; p is index of the entry we're working
	 * on, q is used internally by BATloop to do the iterating */
	bi = bat_iterator(input_bat);
	BATloop(input_bat, p, q) {
		source = (const char *) BUNtail(bi, p);

		size_t len = do_strxfrm(dest->data, source, max_len, locale);

		/* make sure dest is large enough */
		if (len >= max_len) {
			blob *ndest;

			max_len = len + DEFAULT_MAX_STRING_KEY_SIZE;
			nbytes = blobsize(max_len);
			ndest = GDKrealloc(dest, nbytes);
			if (ndest == NULL) {
				/* if GDKrealloc fails, dest is still
				 * allocated */
				goto bailout;
			}
			dest = ndest;

			do_strxfrm(dest->data, source, max_len, locale);
		}

		dest->nitems = len;

		if (BUNappend(result_bat, dest, false) != GDK_SUCCEED) {
			/* BUNappend can fail since it may have to
			 * grow memory areas--especially true for
			 * string BATs */
			goto bailout;
		}
	}
	GDKfree(dest);
	BBPunfix(input_bat->batCacheid);
	*result = result_bat->batCacheid;
	BBPkeepref(result_bat->batCacheid);

	return MAL_SUCCEED;

  bailout:
	/* we only get here in the case of an allocation error; clean
	 * up the mess we've created and throw an exception */
	GDKfree(dest);
	BBPunfix(input_bat->batCacheid);
	BBPunfix(result_bat->batCacheid);
	throw(MAL, "collation.strxfrm", MAL_MALLOC_FAIL);
}

char *
UDFBATstrxfrm(bat *result, const bat *input, const bat * locale_id)
{
	BAT *locale_bat;
	BATiter bi;
	const char *locale_str;
	char *res;

	/*START OF UGLY hack to get the bulk version recognized*/
	locale_bat = BATdescriptor(*locale_id);
	assert(locale_bat->ttype == TYPE_str);
	bi = bat_iterator(locale_bat);
	locale_str = (const char *) BUNtail(bi, 0); 
	/*END OF UGLY hack to get the bulk version recognized*/

	res =  UDFBATstrxfrm_cst(result, input, &locale_str); 
	BBPunfix(locale_bat->batCacheid);
	return res;
}
