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
    (null);

explain select get_sort_key('asdsad', 'en_US');

explain select get_sort_key(s, 'en_US') from foo;
select get_sort_key(s, 'en_US') from foo;

select * from foo order by get_sort_key(s, 'en_US');

select * from foo order by s;

select * from foo where s like '%Mu%';
select * from foo where s like '%Mü%';
select * from foo where s ilike '%SS%';
select * from foo where s ilike '%ß%';


with strings as (select 'c%de' as pattern, 'cbde' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select '\\_%\\%\\_' as pattern, '_a%_' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select 'FUSSBALL' as pattern, 'fußball' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select 'x' as pattern, 'fußball' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') from strings;

with strings as (select 'fußball' as pattern, 'FUSSBALL' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select 'A' as pattern, 'bcäbc' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select '%A' as pattern, 'bcäbc' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select '_A' as pattern, 'bcäbc' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select '__A' as pattern, 'bcäbc' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select '_%A' as pattern, 'bcäbc' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select '%_A' as pattern, 'bcäbc' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select '__%_A' as pattern, 'bcäbc' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select '__%_A' as pattern, 'bcäbc' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select '' as pattern, 'bcäbc' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select '%c%b' as pattern, 'bcäbc' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select '%b_A' as pattern, 'bcäbc' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select '%b__A' as pattern, 'bcäbc' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select 'b___c' as pattern, 'bcäbc' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select '_' as pattern, 'bcäbc' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select '__' as pattern, 'bcäbc' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select '%' as pattern, 'bcäbc' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select '%b_' as pattern, 'bcäbc' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select '%b__' as pattern, 'bcäbc' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select '%b____' as pattern, 'bcäbc' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select '%b_%_' as pattern, 'bcäbc' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select '__ß____' as pattern, 'FUSSBALL' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select '__ß%' as pattern, 'FUSSBALL' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select 'A' as pattern, 'bcäbc' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select '%A' as pattern, 'bcäbc' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select '_A%' as pattern, 'bcäbc' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select '__A__' as pattern, 'bcäbc' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select '_%A_%' as pattern, 'bcäbc' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select '__%_bc' as pattern, 'bcäbc' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select '\\_\\_\\_\\_' as pattern, 'xxxx' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

with strings as (select 'mu%' as pattern)
    select strings.pattern, foo.s, collationlike(strings.pattern, foo.s, 'en_US') as match from strings, foo;

with strings as (select '%A%' as pattern, 'bcäbc' as target)
    select target, pattern,  collationlike(target, pattern, 'en_US') as matches from strings;
with strings as (select '%A%' as pattern, 'bcäbc' as target)
    select target, pattern,  collationlike(target, pattern, 'en_US_POSIX') as matches from strings;

with strings as (select 'FUSSBALL' as pattern, 'fußball' as target)
    select target, pattern,  collationlike(target, pattern, 'en_US') as matches from strings;
with strings as (select 'FUSSBALL' as pattern, 'fußball' as target)
    select target, pattern,  collationlike(target, pattern, 'en_US_POSIX') as matches from strings;

-- check if scalar and bulk versions are properly called
explain with strings as (select 'FUSSBALL' as pattern, 'fußball' as target)
    select target, pattern,  collationlike(target, pattern, 'en_US') as matches from strings;

explain select s, collationlike(s, 'mu%', 'en_US') as matches from foo;
select s, collationlike(s, 'mu%', 'en_US') as matches from foo;

explain select s from foo where collationlike(s, 'mu%', 'en_US');
select s from foo where collationlike(s, 'mu%', 'en_US');

with strings as (select 'FUSSBALL' as pattern, 'fußball' as target)
    select target, pattern as matches from strings where collationlike(target, pattern, 'en_US');

select s from foo where s like null;

select s from foo where collationlike(s, null, 'en_US');

select s from foo where s like 'mu%';

select s from foo where collationlike(null, 'mu%', 'en_US');

ROLLBACK;

select 'TESTING NULL VALUE FOR LOCALE IN BULK VERSION.';
select get_sort_key('asdsad', null);

select 'TESTING NULL VALUE FOR LOCALE IN BULK VERSION.';
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

select 'TESTING NON-ESCAPABLE CHARACTER 1';
with strings as (select '\\x' as pattern, 'xxxx' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

select 'TESTING NON-ESCAPABLE CHARACTER 2';
with strings as (select 'abc\\x' as pattern, 'xxxx' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;

select 'TESTING MISSING ESCAPE CHARACTER';
with strings as (select '\\' as pattern, 'xxxx' as target)
    select target, pattern,  collationlike(target, pattern, 'de_DE') as matches from strings;
