This repository contains prototypes for UDF's to accomodate new collation aware string functionality in MonetDB.

In particular we want to match and sort according to collation and other settings like (accent|case)-[in]sensitiveness.

# Build and installation details

the make file assumes that Monetdb and ICU4C are installed in directories that can be found by pkg-config

I have the issue on my ubuntu machine that I have to remove the bzip2 dependency from
<prefix>/lib/pkgconfig/monetdb-stream.pc

For the future we need both blob.h in the installed include directory and it needs to contain the blob null value.

Make sure icu4c binaries are in the load-library-path, e.g. by using the proper default library installation locatios, LD_LIBRARY_PATH or use rpath in the makefile.

# Getting started

This repository introduces three sql functions which are declared in 82_collation.sql.

## get_sort_key

```sql
FUNCTION get_sort_key(src STRING, locale STRING) RETURNS BLOB
```

This function computes a sort key for each string value in src such that it respects the collator with primary strength associated with the locale identifier. Its main purpose is to order string columns according to their collation order, e.g.

```sql
SELECT s FROM foo ORDER BY get_sort_key(S, 'en_US');
```


## collationlike

```sql
FUNCTION collationlike(input STRING, pattern STRING, locale STRING) RETURNS BOOLEAN
```

This function performs a collation aware `LIKE`-based match between the `input` and `pattern` string. It will typically perform amongst others an accent and case insensitive match between the input and target. It uses the % and _ wildcards similar to their use in a normal `LIKE` predicate. With the backslash being the (currently hardcoded) escape character. An usage example:

```sql
SELECT s FROM foo where collationlike(s, 'mu%', 'en_US');
```

This should return results like 'Müller' for instance.

## locales

```sql
FUNCTION locales() RETURNS TABLE (locale STRING)
```

Any locale identifier to be used in `get_sort_key` and `collationlike` must be a recognized ICU locale. The available locales can be queried with

```sql
select * from locales();
```
