CREATE FUNCTION strxfrm(src STRING, locale STRING) RETURNS BLOB
       EXTERNAL NAME collation.strxfrm;

CREATE FUNCTION likematch(input STRING, pattern STRING, locale STRING) RETURNS BOOLEAN
    EXTERNAL NAME collation.likematch;
