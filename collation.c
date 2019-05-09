/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.  If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2013-2018 MonetDB B.V.
 */

/* monetdb_config.h must be included as the first include file */
#include <monetdb_config.h>

//#include <blob.h> // should be included in the installation/include directory
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

// TODO: empty pattern should be allowed to match.
// TODO: split out this file into several source code files.
// TODO: turn collationlike in a true filter function, i.e. implement collationlikeselect and collationlikejoin.
// TODO: Clean up code base a bit.
// TODO: Create hardcoded en_US based get_sort_key and likematch.

// START INCLUDE HACK TO OBTAIN THE NECESSARY BLOB DECLARATION AND TYPES.
typedef struct blob {
	size_t nitems;
	/*unsigned */ char data[FLEXIBLE_ARRAY_MEMBER];
} blob;

mal_export int TYPE_blob;

static blob nullval = {~(size_t) 0};

mal_export var_t blobsize(size_t nitems);
// END INCLUDE HACK TO OBTAIN THE NECESSARY BLOB DECLARATION AND TYPES.

extern __declspec(dllexport) char *UDFget_sort_key(blob** result, const char **input, const char **locale_id);
extern __declspec(dllexport) char *UDFBATget_sort_key(bat *result, const bat *input, const char **locale_str);
extern __declspec(dllexport) char *UDFlikematch(bit* result, const char **u_target, const char **pattern, const char** locale_id);
extern __declspec(dllexport) char *UDFBATlikematch(bat* result, const bat *target, const char** pattern, const char **locale_str);
extern __declspec(dllexport) char *UDFlocales(bat *result);

static char* first_search(UStringSearch** search, int offset, const char* pattern, const UChar* u_target, UCollator* col) {
	UErrorCode status = U_ZERO_ERROR;
	char* return_status;

	size_t pattern_capacity = strlen(pattern) + 1;
	UChar u_pattern[pattern_capacity];
	u_strFromUTF8Lenient(u_pattern, pattern_capacity, NULL, pattern, -1, &status);

	if (!U_SUCCESS(status)){
		throw(MAL, "collation.collationlike", "Could not transform pattern string from utf-8 to utf-16.");
	}

	*search = usearch_openFromCollator(u_pattern, -1, u_target, -1, col, NULL, &status);

	if (!U_SUCCESS(status)){
		usearch_close(*search);
		throw(MAL, "collation.collationlike", "Could not instantiate ICU string search.");
	}

	usearch_following(*search, offset, &status);

	if (!U_SUCCESS(status)){
		usearch_close(*search);
		throw(MAL, "collation.collationlike", "ICU string search failed.");
	}

	return MAL_SUCCEED;
}

static char* next_search(UStringSearch* search) {
	UErrorCode status = U_ZERO_ERROR;
	usearch_next(search, &status);

	if (!U_SUCCESS(status)){
		usearch_close(search);
		throw(MAL, "collation.collationlike", "ICU next string search failed.");
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
		throw(MAL, "collation.collationlike", "Could not transform target string from utf-8 to utf-16.");
	}

	if (return_status = first_search(&search, 0, pattern, u_target, coll))
		return return_status;

	*found =  usearch_getMatchedStart(search) != USEARCH_DONE;

	usearch_close(search);

	return MAL_SUCCEED;
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

static char * likematch(bit* result, searchcriterium_t* head, const char** target, UCollator* coll) {
	UErrorCode status = U_ZERO_ERROR;
	UStringSearch* search;
	char* return_status;

	if (GDK_STRNIL(*target)) { // nil input implies false result.
		*result = false;
		return MAL_SUCCEED;
	}

	size_t target_capacity = strlen(*target) + 1;
	UChar u_target[target_capacity];
	int nunits;
	u_strFromUTF8Lenient(u_target, target_capacity, &nunits, *target, -1, &status);

	if (!U_SUCCESS(status)){
		throw(MAL, "collation.collationlike", "Could not transform target string from utf-8 to utf-16.");
	}

	return_status = likematch_recursive(result, head, 0, u_target, nunits, coll);

	return return_status;
}

char *
UDFlikematch(bit* result, const char** target, const char** pattern, const char** locale_id) {
	UErrorCode status = U_ZERO_ERROR;
	char* return_status;
	UCollator* coll;
	searchcriterium_t* head;

	if (GDK_STRNIL(*pattern) || !strlen(*pattern)) { // nil or empty pattern implies false result.
		*result = false;
		return MAL_SUCCEED;
	}

	coll = ucol_open(*locale_id, &status);

	if (return_status = create_searchcriteria(&head, *pattern, '\\')) {
		return return_status;
	}

	if (!U_SUCCESS(status)) {
		ucol_close(coll);
		throw(MAL, "collation.collationlike", "Could not create ICU collator.");
	}

	ucol_setStrength(coll, UCOL_PRIMARY);

	return_status = likematch(result, head, target, coll);

	destroy_searchcriteria(head);

	return return_status;
}

char *
UDFBATlikematch(bat* result, const bat *target, const char** pattern, const char **locale_str) {
	UErrorCode status = U_ZERO_ERROR;
	BAT *target_bat, *result_bat;
	BATiter bi;
	bit *result_iter;
	BUN p, q;
	const char *source;
	UCollator* coll;
	char* return_status;
	searchcriterium_t* head;

	coll = ucol_open(*locale_str, &status);

	if (!U_SUCCESS(status)) {
		ucol_close(coll);
		throw(MAL, "collation.collationlike", "Could not create ICU collator.");
	}

	ucol_setStrength(coll, UCOL_PRIMARY);

	if (return_status = create_searchcriteria(&head, *pattern, '\\')) {
		return return_status;
	}

	target_bat = BATdescriptor(*target);

	if (GDK_STRNIL(*pattern) || !strlen(*pattern)) { // nil or empty pattern implies false result.
		if ((result_bat = BATconstant(target_bat->hseqbase, TYPE_bit, &(bit) {false}, BATcount(target_bat), TRANSIENT)) == NULL)
			throw(MAL, "regexp.rematchselect", GDK_EXCEPTION);
		*result = result_bat->batCacheid;
		BBPkeepref(*result);
		return MAL_SUCCEED;
	}

	/* allocate result BAT */
	result_bat = COLnew(target_bat->hseqbase, TYPE_bit, BATcount(target_bat), TRANSIENT);
	result_iter = (bit *) Tloc(result_bat, 0);


	/* loop through BAT input_bat; p is index of the entry we're working
	 * on, q is used internally by BATloop to do the iterating */
	bi = bat_iterator(target_bat);

	BATloop(target_bat, p, q) {
		source = (const char *) BUNtail(bi, p);

		if (GDK_STRNIL(source)) { // nil source implies false result.
			*result_iter = false;
			result_iter++;
		}
		else if (return_status = likematch(result_iter++, head, &source, coll)) {
			goto bailout;
		}
	}

	/* set properties and size of result BAT */
	BATsetcount(result_bat, BATcount(target_bat));
	if (BATcount(result_bat) > 1) {
		/* if more than 1 result, it is not reverse sorted */
		result_bat->tsorted = false;	/* probably not sorted */
		result_bat->trevsorted = false;	/* probably not reverse sorted */
		result_bat->tkey = false;	/* probably not key */
	} else {
		/* if empty or a single result, it is sorted, reverse
		 * sorted, and key */
		result_bat->tsorted = true;
		result_bat->trevsorted = true;
		result_bat->tkey = true;
	}
	result_bat->tnosorted = 0;	/* we don't know for sure */
	result_bat->tnorevsorted = 0;	/* we don't know for sure */
	result_bat->tnokey[0] = result_bat->tnokey[1] = 0;
	result_bat->tnil = false;
	result_bat->tnonil = true;

	*result = result_bat->batCacheid;
	BBPkeepref(*result);

	destroy_searchcriteria(head);

	return MAL_SUCCEED;

  bailout:
	/* we only get here in the case of an allocation error; clean
	 * up the mess we've created and throw an exception */
	destroy_searchcriteria(head);
	BBPunfix(result_bat->batCacheid);
	throw(MAL, "collation.get_locales", MAL_MALLOC_FAIL);
}

const size_t DEFAULT_MAX_STRING_LOCALE_ID_SIZE = 64;

char *
UDFlocales(bat *result) {
	UErrorCode status = U_ZERO_ERROR;

	BAT* result_bat;

	UEnumeration* locales;
	int nlocales;
	const char* locale;

	char *dest;
	char *new_dest;

	int max_len;
	int len;

	locales = ucol_openAvailableLocales(&status);

	if (!U_SUCCESS(status)){
		throw(MAL, "collation.get_locales", "Could open handle to available locales.");
	}

	nlocales = uenum_count(locales, &status);

	if (!U_SUCCESS(status)){
		throw(MAL, "collation.get_locales", "Could not count available locales.");
	}

	result_bat = COLnew(0, TYPE_str, nlocales, TRANSIENT);

	max_len = DEFAULT_MAX_STRING_LOCALE_ID_SIZE;

	if ( (dest = GDKmalloc(max_len)) == NULL )
		goto bailout;



	while (locale = uenum_next(locales, &len, &status)) {

		if (!U_SUCCESS(status)){
			goto bailout;
		}

		/* make sure dest is large enough */
		if (len >= max_len) {

			max_len = len + DEFAULT_MAX_STRING_LOCALE_ID_SIZE;
			new_dest = GDKrealloc(dest, max_len);
			if (new_dest == NULL) {
				/* if GDKrealloc fails, dest is still
				 * allocated */
				goto bailout;
			}
			dest = new_dest;
		}

		strcpy(dest, locale);

		if (BUNappend(result_bat, dest, false) != GDK_SUCCEED) {
			/* BUNappend can fail since it may have to
			 * grow memory areas--especially true for
			 * string BATs */
			goto bailout;
		}
	}

	*result = result_bat->batCacheid;
	BBPkeepref(result_bat->batCacheid);

	uenum_close(locales);

	return MAL_SUCCEED;

  bailout:
	/* we only get here in the case of an allocation error; clean
	 * up the mess we've created and throw an exception */
	GDKfree(dest);
	BBPunfix(result_bat->batCacheid);
	throw(MAL, "collation.get_locales", MAL_MALLOC_FAIL);
}

const size_t DEFAULT_MAX_STRING_KEY_SIZE = 128;

static size_t
do_get_sort_key(char* dest, const UChar* source, size_t len, const UCollator* coll) {
    return ucol_getSortKey(coll, source, -1, dest, len);
}

char *
UDFget_sort_key(blob** result, const char **input, const char **locale_id)
{
	UErrorCode status = U_ZERO_ERROR;
	blob* dest;
	size_t len, nbytes;
	UCollator* coll;

	if (GDK_STRNIL(*input)) { // nil input implies nil result.
		*result = (blob*) GDKmalloc(sizeof(nullval));
		if (*result == NULL)
			throw(MAL, "collation.get_sort_key", MAL_MALLOC_FAIL);
		**result = nullval;
		return MAL_SUCCEED;
	}

	if (GDK_STRNIL(*locale_id))
		throw(MAL, "collation.get_sort_key", "locale identifier cannot be null.");

	coll = ucol_open(*locale_id, &status);

	if (!U_SUCCESS(status)) {
		ucol_close(coll);
		throw(MAL, "collation.get_sort_key", "Could not create ICU collator.");
	}

	ucol_setStrength(coll, UCOL_PRIMARY);

	size_t u_input_capacity = strlen(*input) + 1;
	UChar u_input[u_input_capacity];
	u_strFromUTF8Lenient(u_input, u_input_capacity, NULL, *input, -1, &status);

	if (!U_SUCCESS(status)){
		throw(MAL, "collation.get_sort_key", "Could not transform target string from utf-8 to utf-16.");
	}

	len = do_get_sort_key(NULL, u_input, 0, coll);
	nbytes = blobsize(len);
	dest = *result = GDKmalloc(nbytes);
	if (dest == NULL)
		throw(MAL, "collation.get_sort_key", MAL_MALLOC_FAIL);

	dest->nitems =len;

	do_get_sort_key(dest->data, u_input, len, coll);

	return MAL_SUCCEED;
}

char *
UDFBATget_sort_key(bat *result, const bat *input, const char **locale_str)
{
	UErrorCode status = U_ZERO_ERROR;
	BAT *input_bat, *result_bat;
	BATiter bi;
	blob *dest;
	BUN p, q;
	const char *source;
	size_t len, max_len, nbytes;
	UCollator* coll;

	if (GDK_STRNIL(*locale_str))
		throw(MAL, "collation.get_locales", "locale identifier cannot be null.");

	coll = ucol_open(*locale_str, &status);

	if (!U_SUCCESS(status)) {
		ucol_close(coll);
		throw(MAL, "collation.collationlike", "Could not create ICU collator.");
	}
	/* allocate temporary space for transformed strings; we grow this
	 * if we need more */
	max_len = DEFAULT_MAX_STRING_KEY_SIZE;
	nbytes = blobsize(max_len);
	dest = GDKmalloc(nbytes);
	if (dest == NULL)
		throw(MAL, "collation.get_sort_key", MAL_MALLOC_FAIL);

	input_bat = BATdescriptor(*input);
	if (input_bat == NULL) {
		GDKfree(dest);
		throw(MAL, "collation.get_sort_key", RUNTIME_OBJECT_MISSING);
	}

	/* we should only get called for string BATs */
	assert(input_bat->ttype == TYPE_str);

	/* allocate result BAT */
	result_bat = COLnew(input_bat->hseqbase, TYPE_blob, BATcount(input_bat), TRANSIENT);
	if (result_bat == NULL) {
		BBPunfix(input_bat->batCacheid);
		GDKfree(dest);
		throw(MAL, "collation.get_sort_key", MAL_MALLOC_FAIL);
	}

	/* loop through BAT input_bat; p is index of the entry we're working
	 * on, q is used internally by BATloop to do the iterating */
	bi = bat_iterator(input_bat);
	BATloop(input_bat, p, q) {
		source = (const char *) BUNtail(bi, p);

		if (GDK_STRNIL(source)) {
			blob *ndest;

			if ( (ndest = GDKrealloc(dest, sizeof(nullval))) == NULL) {
				/* if GDKrealloc fails, dest is still
				 * allocated */
				goto bailout;
			}
			dest = ndest;
			*dest = nullval;

			if (BUNappend(result_bat, dest, false) != GDK_SUCCEED) {
				/* BUNappend can fail since it may have to
				* grow memory areas--especially true for
				* string BATs */
				goto bailout;
			}

			break;
		}

		size_t u_source_capacity = strlen(source) + 1;
		UChar u_source[u_source_capacity];
		u_strFromUTF8Lenient(u_source, u_source_capacity, NULL, source, -1, &status);

		if (!U_SUCCESS(status)){
			throw(MAL, "collation.get_sort_key", "Could not transform target string from utf-8 to utf-16.");
		}

		size_t len = do_get_sort_key(dest->data, u_source, max_len, coll);

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

			do_get_sort_key(dest->data, u_source, max_len, coll);
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
	throw(MAL, "collation.get_sort_key", MAL_MALLOC_FAIL);
}

/*
// START OF UGLY hack to get the bulk version recognized
char *
UDFBATget_sort_key_hack(bat *result, const bat *input, const bat * locale_id)
{
	BAT *locale_bat;
	BATiter bi;
	const char *locale_str;
	char *res;

	locale_bat = BATdescriptor(*locale_id);
	assert(locale_bat->ttype == TYPE_str);
	bi = bat_iterator(locale_bat);
	locale_str = (const char *) BUNtail(bi, 0); 

	res =  UDFBATget_sort_key(result, input, &locale_str);
	BBPunfix(locale_bat->batCacheid);
	return res;
}
// END OF UGLY hack to get the bulk version recognized
 */