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

/* __declspec() must be used on Windows, but not on other systems */
#ifndef _MSC_VER
/* not Windows */
#define __declspec(x)	/* nothing */
#endif

extern __declspec(dllexport) char *UDFstrxfrm(blob **result, const char **input);
extern __declspec(dllexport) char *UDFBATstrxfrm(bat *result, const bat *input);

#define SOME_LOCALE_ID "de_DE.utf8"
#define DEFAULT_MAX_STRING_KEY_SIZE 128

static size_t
do_strxfrm(char *dest, const char *source, size_t len, locale_t locale) {
    
    return strxfrm_l(dest, source, len, locale)  + /* for null terminator*/ 1;
}

char *
UDFstrxfrm(blob **result, const char **input)
{
	blob* dest;
	size_t len, nbytes;
	locale_t locale;

	locale = newlocale(LC_COLLATE_MASK, SOME_LOCALE_ID,( locale_t)0);

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
UDFBATstrxfrm(bat *result, const bat *input)
{
	BAT *input_bat, *result_bat;
	BATiter bi;
	blob *dest;
	BUN p, q;
	const char *source;
	locale_t locale;
	size_t len, max_len, nbytes;

	locale = newlocale(LC_COLLATE_MASK, SOME_LOCALE_ID,( locale_t)0);

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
