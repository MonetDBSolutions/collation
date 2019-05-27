START TRANSACTION;

create table foo (s STRING);

insert into foo values
    ('Mvller'),
    ('Müller'),
    ('müller'),
    ('Houße'),
    ('Muller'),
    ('Housse'),
    ('House'),
    ('Fußball'),
    ('_VERY_LONG_STRING_ßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßß'),
    (null),
    ('');

SELECT 'TESTS FOR get_sort_key';
explain select get_sort_key('asdsad', 'en_US');

explain select get_sort_key(s, 'en_US') from foo;
select get_sort_key(s, 'en_US') from foo;

select * from foo order by get_sort_key(s, 'en_US');

select * from foo order by s;

SELECT 'TESTS FOR collationlike';
-- check if scalar and bulk versions are properly called
explain with strings as (select 'FUSSBALL' as pattern, 'fußball' as target)
    select target, pattern,  collationlike(target, pattern, 'en_US') as matches from strings;

explain select s, collationlike(s, 'mu%', 'en_US') as matches from foo;
select s, collationlike(s, 'mu%', 'en_US') as matches from foo;

explain select s from foo where collationlike(s, 'mu%', 'en_US');
select s from foo where collationlike(s, 'mu%', 'en_US');

with strings as (select 'FUSSBALL' as pattern, 'fußball' as target)
    select target, pattern as matches from strings where collationlike(target, pattern, 'en_US');

--null and empty string corner cases

select s from foo where s like null;

select s from foo where collationlike(s, null, 'en_US');

select s from foo where s like 'mu%';

select s from foo where collationlike(null, 'mu%', 'en_US');

with strings as (select null as pattern)
    select strings.pattern, foo.s, collationlike(foo.s, strings.pattern, 'en_US') as match from strings, foo;

with strings as (select '' as pattern)
    select strings.pattern, foo.s, collationlike(foo.s, strings.pattern, 'en_US') as match from strings, foo;

with strings as (select '%' as pattern)
    select strings.pattern, foo.s, collationlike(foo.s, strings.pattern, 'en_US') as match from strings, foo;

with strings as (select '%_' as pattern)
    select strings.pattern, foo.s, collationlike(foo.s, strings.pattern, 'en_US') as match from strings, foo;

with strings as (select 'mu%' as pattern)
    select strings.pattern, foo.s, collationlike(foo.s, strings.pattern, 'en_US') as match from strings, foo;

ROLLBACK;

select 'TESTING TABLE PRODUCING FUNCTION locales().';
select locale from locales();

select 'TESTING NULL VALUE FOR LOCALE PARAMETER FOR get_sort_key.';
select get_sort_key('asdsad', null);

select 'TESTING NULL VALUE FOR LOCALE PARAMETER FOR get_sort_key IN BULK VERSION.';
START TRANSACTION;
create table foo (s STRING);
insert into foo values
    ('Mvller'),
    ('Müller'),
    ('müller'),
    ('Houße'),
    ('Muller'),
    ('Housse'),
    ('House'),
    ('Fußball'),
    ('_VERY_LONG_STRING_ßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßß');
select get_sort_key(s, null) from foo;
ROLLBACK;

select 'TESTING NON-ESCAPABLE CHARACTER 1 FOR collationlike';
with strings as (select '\\x' as pattern, 'xxxx' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

select 'TESTING NON-ESCAPABLE CHARACTER 2 FOR collationlike';
with strings as (select 'abc\\x' as pattern, 'xxxx' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

select 'TESTING MISSING ESCAPE CHARACTER FOR collationlike';
with strings as (select '\\' as pattern, 'xxxx' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;
