CREATE FUNCTION strxfrm(src STRING) RETURNS BLOB
       EXTERNAL NAME collation.strxfrm;
