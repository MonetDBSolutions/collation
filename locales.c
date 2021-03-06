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

#ifndef _MSC_VER
#define __declspec(x)	/* nothing */
#endif

const size_t DEFAULT_MAX_STRING_LOCALE_ID_SIZE = 64;

extern __declspec(dllexport) char *UDFlocales(bat *result);

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



	while ( (locale = uenum_next(locales, &len, &status)) ) {

		if (!U_SUCCESS(status)){
			goto bailout;
		}

		if (len >= max_len) {

			max_len = len + DEFAULT_MAX_STRING_LOCALE_ID_SIZE;
			new_dest = GDKrealloc(dest, max_len);
			if (new_dest == NULL) {
				goto bailout;
			}
			dest = new_dest;
		}

		strcpy(dest, locale);

		if (BUNappend(result_bat, dest, false) != GDK_SUCCEED) {
			goto bailout;
		}
	}

	*result = result_bat->batCacheid;
	BBPkeepref(result_bat->batCacheid);

	uenum_close(locales);

	return MAL_SUCCEED;

  bailout:
	GDKfree(dest);
	BBPunfix(result_bat->batCacheid);
	throw(MAL, "collation.get_locales", MAL_MALLOC_FAIL);
}
