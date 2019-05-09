-- This Source Code Form is subject to the terms of the Mozilla Public
-- License, v. 2.0.  If a copy of the MPL was not distributed with this
-- file, You can obtain one at http://mozilla.org/MPL/2.0/.
--
-- Copyright 2013-2018 MonetDB B.V.

CREATE FUNCTION get_sort_key(src STRING, locale STRING) RETURNS BLOB
       EXTERNAL NAME collation.get_sort_key;

CREATE FUNCTION collationlike(input STRING, pattern STRING, locale STRING) RETURNS BOOLEAN
    EXTERNAL NAME collation.collationlike;

CREATE FUNCTION locales() RETURNS TABLE (locale STRING)
    EXTERNAL NAME collation.get_locales;
