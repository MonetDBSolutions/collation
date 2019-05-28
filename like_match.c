/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.  If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2013-2018 MonetDB B.V.
 */

#include <monetdb_config.h>
#include <mal_exception.h>

#include <string.h>

#include <unicode/ucol.h>
#include <unicode/usearch.h>
#include <unicode/ustring.h>

#include "dfa.h"

#ifndef _MSC_VER
#define __declspec(x)	/* nothing */
#endif

// TODO: turn collationlike in a true filter function, i.e. implement collationlikeselect and collationlikejoin.
// TODO: Clean up code base a bit.

extern __declspec(dllexport) char *UDFlikematch(bit* result, const char **u_target, const char **pattern, const char** locale_id);
extern __declspec(dllexport) char *UDFBATlikematch(bat* result, const bat *target, const char** pattern, const char **locale_str);

static char* first_search(UStringSearch** search, int offset, const UChar* u_pattern, const UChar* u_target, UCollator* col) {
	UErrorCode status = U_ZERO_ERROR;

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

static char* likematch_recursive(bit* found, searchcriterium_t* current, int offset, const UChar* target, const int nunits, UCollator* coll) {
	UStringSearch* search;
	int pos;
	char* return_status;

	*found = false;

	if (current->search_string.nbytes > 1 /*non-empty string*/) {
		if ( (return_status = first_search(&search, offset, current->search_string_16.data, target, coll)) ) {
			return return_status;
		}
	}
	else {
		// empty string signals end of pattern parts.
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

			if ( (return_status = likematch_recursive(found, current->next, next_offset, target, nunits, coll)) ) {
				usearch_close(search);
				return return_status;
			}
		}

		if( (return_status = next_search(search)) )
			return return_status;

	} while(!*found);

	usearch_close(search);

	return MAL_SUCCEED;
}

static char * likematch(bit* result, searchcriterium_t* head, const char** target, UCollator* coll) {
	UErrorCode status = U_ZERO_ERROR;
	char* return_status;

	if (GDK_STRNIL(*target)) { // nil input implies false result.
		*result = false;
		return MAL_SUCCEED;
	}

	/* unfortunately ICU cannot handle empty target strings,
	 * hence we have to check this ourselves.*/
	if (!strlen(*target)) {
		*result = (head->card == GREATER_OR_EQUAL && head->start == 0);
		return  MAL_SUCCEED;
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

	if (GDK_STRNIL(*pattern)) { // nil pattern implies false result.
		*result = false;
		return MAL_SUCCEED;
	}

	if (!strlen(*pattern)) { // empty pattern can only match empty target.
		*result = !strlen(*target);
		return MAL_SUCCEED;
	}

	coll = ucol_open(*locale_id, &status);

	if ( (return_status = create_searchcriteria(&head, *pattern, '\\')) ) {
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

	target_bat = BATdescriptor(*target);

	if (GDK_STRNIL(*pattern)) { // nil pattern implies false result.
		if ((result_bat = BATconstant(target_bat->hseqbase, TYPE_bit, &(bit) {false}, BATcount(target_bat), TRANSIENT)) == NULL)
			throw(MAL, "collation.UDFBATlikematch", GDK_EXCEPTION);
	}
	else if (!strlen(*pattern)) { // empty pattern
		if ((result_bat = COLnew(target_bat->hseqbase, TYPE_bit, BATcount(target_bat), TRANSIENT)) == NULL)
			throw(MAL, "collation.UDFBATlikematch", GDK_EXCEPTION);

		result_iter = (bit *) Tloc(result_bat, 0);

		bi = bat_iterator(target_bat);

		BATloop(target_bat, p, q) {
			source = (const char *) BUNtail(bi, p);

			*result_iter = !GDK_STRNIL(source) && !strlen(source);
			result_iter++;
		}
	}
	else {
		coll = ucol_open(*locale_str, &status);

		if (!U_SUCCESS(status)) {
			ucol_close(coll);
			throw(MAL, "collation.collationlike", "Could not create ICU collator.");
		}

		ucol_setStrength(coll, UCOL_PRIMARY);

		if ( (return_status = create_searchcriteria(&head, *pattern, '\\')) ) {
			return return_status;
		}

		target_bat = BATdescriptor(*target);

		result_bat = COLnew(target_bat->hseqbase, TYPE_bit, BATcount(target_bat), TRANSIENT);
		result_iter = (bit *) Tloc(result_bat, 0);


		bi = bat_iterator(target_bat);

		BATloop(target_bat, p, q) {
			source = (const char *) BUNtail(bi, p);

			if (GDK_STRNIL(source)) { // nil source implies false result.
				*result_iter = false;
				result_iter++;
			}
			else if ( (return_status = likematch(result_iter++, head, &source, coll)) ) {
				goto bailout;
			}
		}

		destroy_searchcriteria(head);
	}

	BATsetcount(result_bat, BATcount(target_bat));
	if (BATcount(result_bat) > 1) {
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

	return MAL_SUCCEED;

  bailout:
	destroy_searchcriteria(head);
	BBPunfix(result_bat->batCacheid);
	throw(MAL, "collation.get_locales", MAL_MALLOC_FAIL);
}
