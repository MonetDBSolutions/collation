This repository contains prototypes for MonetDB UDF's to accomodate new collation aware string functionality in MonetDB. In particular we want to match and sort according to collation and other settings like (accent|case)-[in]sensitiveness.

# ICU

The repository has a third pary dependency on ICU4C which is the C version of the International Components for Unicode. This library offers all kinds of tools to deal with text in a locale sensitive and very customizable manner.

icu4c is available on all major platforms and most package managers should be able to install it from their respective package repository.

# Build and installation details

the make file assumes that MonetDB and ICU4C are installed in directories that can be found by pkg-config.

I have the issue on my ubuntu machine that I have to remove the bzip2 dependency from
<prefix>/lib/pkgconfig/monetdb-stream.pc

For the future we need both blob.h in the installed include directory and it needs to contain the blob null value.

Make sure icu4c binaries are in the load-library-path, e.g. by using the proper default library installation locations, LD_LIBRARY_PATH or rpath.

# Getting started

This repository introduces three sql functions which are declared in 82_collation.sql.

## `FUNCTION get_sort_key(src STRING, locale STRING) RETURNS BLOB`

This function computes a sort key for each string value in src such that it respects the collator with primary strength associated with the locale identifier. Its main purpose is to order string columns according to their collation order, e.g.

```sql
SELECT s FROM foo ORDER BY get_sort_key(S, 'en_US');
```


## `FUNCTION collationlike(input STRING, pattern STRING, locale STRING) RETURNS BOOLEAN`

This function performs a collation aware `LIKE`-based match between the `input` and `pattern` string. It will typically perform amongst others an accent and case insensitive match between the input and target. It uses the % and _ wildcards similar to their use in a normal `LIKE` predicate. With the backslash being the (currently hardcoded) escape character. An usage example:

```sql
SELECT s FROM foo where collationlike(s, 'mu%', 'en_US');
```

This should return results like 'Müller' for instance.

## `FUNCTION locales() RETURNS TABLE (locale STRING)`

Any locale identifier to be used in `get_sort_key` and `collationlike` must be a recognized ICU locale. The available locales can be queried with

```sql
select * from locales();
```
