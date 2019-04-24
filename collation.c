/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.  If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2013-2018 MonetDB B.V.
 */

/* monetdb_config.h must be included as the first include file */
#include <monetdb_config.h>

/* mal_exception.h actually contains everything we need */
#include <mal_exception.h>

/* system include files */
#include <string.h>
#include <locale.h>

/* __declspec() must be used on Windows, but not on other systems */
#ifndef _MSC_VER
/* not Windows */
#define __declspec(x)	/* nothing */
#endif

extern __declspec(dllexport) char *UDFstrxfrm(char **ret, const char **arg);
extern __declspec(dllexport) char *UDFBATstrxfrm(bat *ret, const bat *arg);

#define SOME_LOCALE_ID "de_DE.utf8"

static size_t
do_strxfrm(char *dst, const char *src, size_t len, locale_t locale) {
    
    return strxfrm_l(dst, src, len, locale);
}

char *
UDFstrxfrm(char **ret, const char **arg)
{
	size_t len_arg = strlen(*arg);
	locale_t locale = newlocale(LC_COLLATE_MASK, SOME_LOCALE_ID,( locale_t)0);

	size_t len_ret = do_strxfrm(NULL, *arg, 0, locale);

	*ret = GDKmalloc(len_ret + 1);

	if (*ret == NULL)
		throw(MAL, "collation.strxfrm", MAL_MALLOC_FAIL);

	do_strxfrm(*ret, *arg, len_ret, locale);

	return MAL_SUCCEED;
}

char *
UDFBATstrxfrm(bat *ret, const bat *arg)
{
	BAT *b, *bn;
	BATiter bi;
	BUN p, q;
	const char *src;
	size_t len;
	char *dst;
	size_t dstlen;

	locale_t locale = newlocale(LC_COLLATE_MASK, SOME_LOCALE_ID,( locale_t)0);

	/* allocate temporary space for reversed strings; we grow this
	 * if we need more */
	dstlen = 128;
	dst = GDKmalloc(dstlen);
	if (dst == NULL)
		throw(MAL, "collation.strxfrm", MAL_MALLOC_FAIL);

	b = BATdescriptor(*arg);
	if (b == NULL) {
		GDKfree(dst);
		throw(MAL, "collation.strxfrm", RUNTIME_OBJECT_MISSING);
	}

	/* we should only get called for string BATs */
	assert(b->ttype == TYPE_str);

	/* allocate result BAT */
	bn = COLnew(b->hseqbase, TYPE_str, BATcount(b), TRANSIENT);
	if (bn == NULL) {
		BBPunfix(b->batCacheid);
		GDKfree(dst);
		throw(MAL, "collation.strxfrm", MAL_MALLOC_FAIL);
	}

	/* loop through BAT b; p is index of the entry we're working
	 * on, q is used internally by BATloop to do the iterating */
	bi = bat_iterator(b);
	BATloop(b, p, q) {
		src = (const char *) BUNtail(bi, p);

		size_t len = do_strxfrm(dst, src, dstlen, locale);

		/* make sure dst is large enough */
		if (len >= dstlen) {
			char *ndst;

			dstlen = len + 1024;
			ndst = GDKrealloc(dst, dstlen);
			if (ndst == NULL) {
				/* if GDKrealloc fails, dst is still
				 * allocated */
				goto bailout;
			}
			dst = ndst;

			do_strxfrm(dst, src, dstlen, locale);
		}

		if (BUNappend(bn, dst, false) != GDK_SUCCEED) {
			/* BUNappend can fail since it may have to
			 * grow memory areas--especially true for
			 * string BATs */
			goto bailout;
		}
	}
	GDKfree(dst);

	BBPunfix(b->batCacheid);

	*ret = bn->batCacheid;
	BBPkeepref(bn->batCacheid);

	return MAL_SUCCEED;

  bailout:
	/* we only get here in the case of an allocation error; clean
	 * up the mess we've created and throw an exception */
	GDKfree(dst);
	BBPunfix(b->batCacheid);
	BBPunfix(bn->batCacheid);
	throw(MAL, "collation.strxfrm", MAL_MALLOC_FAIL);
}
