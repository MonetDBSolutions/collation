CREATE FUNCTION strxfrm(src STRING) RETURNS STRING
       EXTERNAL NAME collation.strxfrm;