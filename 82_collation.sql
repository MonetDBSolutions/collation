CREATE FUNCTION get_sort_key(src STRING, locale STRING) RETURNS BLOB
       EXTERNAL NAME collation.get_sort_key;

CREATE FUNCTION collationlike(input STRING, pattern STRING, locale STRING) RETURNS BOOLEAN
    EXTERNAL NAME collation.collationlike;

CREATE FUNCTION search(input STRING, pattern STRING, locale STRING) RETURNS BOOLEAN
    EXTERNAL NAME collation.search;

CREATE FUNCTION locales() RETURNS TABLE (locale STRING)
    EXTERNAL NAME collation.get_locales;
