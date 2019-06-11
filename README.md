This repository contains prototypes for MonetDB UDF's to accomodate new collation aware string functionality in MonetDB. In particular we want to match and sort according to collation and other settings like (accent|case)-[in]sensitiveness.

# ICU

The repository has a third pary library dependency on icu4c which is the C version of the International Components for Unicode. This library offers all kinds of tools to deal with text in a locale sensitive and very customizable manner.

icu4c is available on all major platforms and most package managers should be able to install it from their respective package repository.

#MonetDB-extend's regexp

The repository has a functional dependency on the MonetDB-extend library which provides amongst others regular expression support for monetdb. To build the regexp library clone the associated Mercurial repository https://dev.monetdb.org/hg/MonetDB-extend/.  Make sure you have installed MonetDB distribution. After that just call `make install` from the root of the regexp subdirectory. Finally start your database server with an empty database. This should make the regexp functionality available. See the README.rst file in the regexp source directory for more details.


# Build and installation details

the make file assumes that MonetDB, icu4c and regexp are installed in directories that can be found by pkg-config.

I have the issue on my ubuntu machine that I have to remove the bzip2 dependency from
<prefix>/lib/pkgconfig/monetdb-stream.pc

For the future we need both blob.h in the installed include directory and it needs to contain the blob null value.

Make sure icu4c binaries are in the load-library-path, e.g. by using the proper default library installation locations, LD_LIBRARY_PATH or rpath.

To build the udf library, simply execute `make` in the root of the source directory. To build and install it, execute `make install`. 

# Getting started

This repository introduces five sql functions which are declared in 82_collation.sql.

## `FUNCTION get_sort_key(input STRING, locale STRING) RETURNS BLOB`

This function computes a sort key for each string value in `input` such that it respects the collator with primary strength associated with the `locale` identifier. Its main purpose is to order string columns according to their collation order, e.g.

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

## `FUNCTION get_re(pattern STRING) RETURNS STRING`

This function takes a string with the semantics of a `LIKE`-pattern and transforms it into a PCRE compatible regular expression. It can be used in collaboration with the regexp MAL extension to obtain accent insensitive (I)LIKE behavior:

```sql
select count(*) from orders where rematch(o_comment, get_re('%request%'));
```

Notice how the 'ɋ' character (small 'q' with hook tail) is expended to the regular expression that matches a number of variants and cases of the letter 'q' in this example of transformation behavior:


```sql
 get_re('%☺ɋ_') = '^.*☺[q\\x{024B}\\x{02A0}\\x{A757}\\x{A759}Q\\x{A756}\\x{A758}][\\x{0300}-\\x{036F}]*.$';
 ```

## `FUNCTION get_ai_sort_key(input STRING) RETURNS STRING`

This function transforms an arbitrary string into an accent insensitive normalized form. It has both a scalar and vectorized implementation so that it can transform both single string values and entire string columns. It can be used as a tool to create sort key indices for accent insensitive matching and ordering use cases. For instance if the user has two string columns that he wants to match in an accent insensitive way, he can create a transformed column for each original column which can then be used in a join condition. Example of transformation behavior:

```sql
 get_ai_sort_key('%Ðôõç☺å_') = '%dooc☺a_';
 ```
