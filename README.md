This repository contains prototypes for UDF's to accomodate new collation aware string functionality in MonetDB.

In particular we want to match and sort according to collation and other settings like (accent|case)-[in]sensitiveness.

Installation details

the make file assumes that Monetdb and ICU4C are installed in directories that can be found by pkg-config

I have the issue on my ubuntu machine that I have to remove the bzip2 dependency from
<prefix>/lib/pkgconfig/monetdb-stream.pc

For the future we need both blob.h in the installed include directory and it needs to contain the blob null value.

Make sure icu4c binaries are in the load-library-path, e.g. by using the proper default library installation locatios, LD_LIBRARY_PATH or use rpath in the makefile.
