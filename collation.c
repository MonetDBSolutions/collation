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

/* __declspec() must be used on Windows, but not on other systems */
#ifndef _MSC_VER
/* not Windows */
#define __declspec(x)	/* nothing */
#endif

extern __declspec(dllexport) char *UDFstrxfrm(blob **result, const char **input, const char **locale_id);
extern __declspec(dllexport) char *UDFBATstrxfrm(bat *result, const bat *input, const bat * locale);
extern __declspec(dllexport) char *UDFlikematch(bit* result, const char **pattern, const char **u_target, const char** locale_id);

char *
UDFlikematch(bit* result, const char **pattern, const char **target, const char** locale_id)
{
	UErrorCode status = U_ZERO_ERROR;
    int pos = USEARCH_DONE;
    UStringSearch *search = NULL;

	UChar u_pattern[strlen(*pattern)];
    UChar u_target[strlen(*target)];
    u_uastrcpy(u_pattern, *pattern);
    u_uastrcpy(u_target, *target);

	UCollator *coll = ucol_open(*locale_id, &status);

	if (!U_SUCCESS(status)){
		ucol_close(coll);
		throw(MAL, "icu.likematch", "Could not create ICU collator.");
	}

	ucol_setStrength(coll, UCOL_PRIMARY);
	search = usearch_openFromCollator(u_pattern, -1, u_target, -1, coll, NULL, &status);

	if (!U_SUCCESS(status)){
		ucol_close(coll);
		usearch_close(search);
		throw(MAL, "icu.likematch", "Could not instantiate ICU string search.");
	}

	pos = usearch_first(search, &status);

	printf("position: %d\n", pos);
	*result = pos != USEARCH_DONE;

	usearch_close(search);
	ucol_close(coll);

	if (!U_SUCCESS(status)){
		throw(MAL, "icu.likematch", "ICU string search failed.");
	}

	return MAL_SUCCEED;
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
UDFBATstrxfrm(bat *result, const bat *input, const bat * locale_id)
{
	BAT *input_bat, *result_bat, *locale_bat;
	BATiter bi;
	blob *dest;
	BUN p, q;
	const char *source;
	locale_t locale;
	size_t len, max_len, nbytes;

	/*START OF UGLY hack to get the bulk version recognized*/
	locale_bat = BATdescriptor(*locale_id);
	assert(locale_bat->ttype == TYPE_str);
	bi = bat_iterator(locale_bat);
	locale = newlocale(LC_COLLATE_MASK, (const char *) BUNtail(bi, 0), ( locale_t)0);
	/*END OF UGLY hack to get the bulk version recognized*/

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
