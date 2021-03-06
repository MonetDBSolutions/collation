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

#ifndef _MSC_VER
#define __declspec(x)	/* nothing */
#endif

// START INCLUDE HACK TO OBTAIN THE NECESSARY BLOB DECLARATION AND TYPES instead of #include <blob.h>.
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

const size_t DEFAULT_MAX_STRING_KEY_SIZE = 128;

static size_t
do_get_sort_key(char* dest, const UChar* source, size_t len, const UCollator* coll) {
    return ucol_getSortKey(coll, source, -1, (uint8_t*)/*TODO: perhaps our blob type should use uint8_t instead of char*/ dest, len);
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
	size_t max_len, nbytes;
	UCollator* coll;

	if (GDK_STRNIL(*locale_str))
		throw(MAL, "collation.get_locales", "locale identifier cannot be null.");

	coll = ucol_open(*locale_str, &status);

	if (!U_SUCCESS(status)) {
		ucol_close(coll);
		throw(MAL, "collation.collationlike", "Could not create ICU collator.");
	}

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

	assert(input_bat->ttype == TYPE_str);

	result_bat = COLnew(input_bat->hseqbase, TYPE_blob, BATcount(input_bat), TRANSIENT);
	if (result_bat == NULL) {
		BBPunfix(input_bat->batCacheid);
		GDKfree(dest);
		throw(MAL, "collation.get_sort_key", MAL_MALLOC_FAIL);
	}

	bi = bat_iterator(input_bat);
	BATloop(input_bat, p, q) {
		source = (const char *) BUNtail(bi, p);

		if (GDK_STRNIL(source)) {
			blob *ndest;

			if ( (ndest = GDKrealloc(dest, sizeof(nullval))) == NULL) {
				goto bailout;
			}
			dest = ndest;
			*dest = nullval;

			if (BUNappend(result_bat, dest, false) != GDK_SUCCEED) {
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

		if (len >= max_len) {
			blob *ndest;

			max_len = len + DEFAULT_MAX_STRING_KEY_SIZE;
			nbytes = blobsize(max_len);
			ndest = GDKrealloc(dest, nbytes);
			if (ndest == NULL) {
				goto bailout;
			}
			dest = ndest;

			do_get_sort_key(dest->data, u_source, max_len, coll);
		}

		dest->nitems = len;

		if (BUNappend(result_bat, dest, false) != GDK_SUCCEED) {
			goto bailout;
		}
	}
	GDKfree(dest);
	BBPunfix(input_bat->batCacheid);
	*result = result_bat->batCacheid;
	BBPkeepref(result_bat->batCacheid);

	return MAL_SUCCEED;

  bailout:
	GDKfree(dest);
	BBPunfix(input_bat->batCacheid);
	BBPunfix(result_bat->batCacheid);
	throw(MAL, "collation.get_sort_key", MAL_MALLOC_FAIL);
}
