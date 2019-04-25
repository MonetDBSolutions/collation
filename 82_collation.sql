CREATE FUNCTION strxfrm(src STRING, locale STRING) RETURNS BLOB
       EXTERNAL NAME collation.strxfrm;
