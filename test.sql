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

explain select get_sort_key('asdsad', 'de_DE.utf8');
select get_sort_key('asdsad', 'de_DE.utf8');

explain select * from foo order by get_sort_key(s, 'en_US');
select * from foo order by get_sort_key(s, 'en_US');

select * from foo order by s;

select * from foo where s like '%Mu%';
select * from foo where s like '%Mü%';
select * from foo where s ilike '%SS%';
select * from foo where s ilike '%ß%';



with strings as (select 'c%de' as pattern, 'cbde' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '\\_%\\%\\_' as pattern, '_a%_' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select 'FUSSBALL' as pattern, 'fußball' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select 'x' as pattern, 'fußball' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') from strings;

with strings as (select 'fußball' as pattern, 'FUSSBALL' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select 'A' as pattern, 'bcäbc' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '%A' as pattern, 'bcäbc' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '_A' as pattern, 'bcäbc' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '__A' as pattern, 'bcäbc' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '_%A' as pattern, 'bcäbc' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '%_A' as pattern, 'bcäbc' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '__%_A' as pattern, 'bcäbc' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '__%_A' as pattern, 'bcäbc' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '' as pattern, 'bcäbc' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '%c%b' as pattern, 'bcäbc' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '%b_A' as pattern, 'bcäbc' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '%b__A' as pattern, 'bcäbc' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select 'b___c' as pattern, 'bcäbc' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '_' as pattern, 'bcäbc' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '__' as pattern, 'bcäbc' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '%' as pattern, 'bcäbc' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '%b_' as pattern, 'bcäbc' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '%b__' as pattern, 'bcäbc' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '%b____' as pattern, 'bcäbc' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '%b_%_' as pattern, 'bcäbc' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '__ß____' as pattern, 'FUSSBALL' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '__ß%' as pattern, 'FUSSBALL' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select 'A' as pattern, 'bcäbc' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '%A' as pattern, 'bcäbc' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '_A%' as pattern, 'bcäbc' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '__A__' as pattern, 'bcäbc' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '_%A_%' as pattern, 'bcäbc' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '__%_bc' as pattern, 'bcäbc' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '\\_\\_\\_\\_' as pattern, 'xxxx' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select 'Ab' as pattern, 'bcäbc' as target)
    select pattern, target,  search(pattern, target, 'de_DE') as matches from strings;

with strings as (select 'Abd' as pattern, 'bcäbc' as target)
    select pattern, target,  search(pattern, target, 'de_DE') as matches from strings;

with strings as (select 'mu%' as pattern)
    select strings.pattern, foo.s, collationlike(strings.pattern, foo.s, 'en_US') as match from strings, foo;

ROLLBACK;

with strings as (select '\\x' as pattern, 'xxxx' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select 'abc\\x' as pattern, 'xxxx' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '\\' as pattern, 'xxxx' as target)
    select pattern, target,  collationlike(pattern, target, 'de_DE') as matches from strings;

with strings as (select '%A%' as pattern, 'bcäbc' as target)
    select pattern, target,  collationlike(pattern, target, 'en_US') as matches from strings;
with strings as (select '%A%' as pattern, 'bcäbc' as target)
    select pattern, target,  collationlike(pattern, target, 'en_US_POSIX') as matches from strings;

with strings as (select 'FUSSBALL' as pattern, 'fußball' as target)
    select pattern, target,  collationlike(pattern, target, 'en_US') as matches from strings;
with strings as (select 'FUSSBALL' as pattern, 'fußball' as target)
    select pattern, target,  collationlike(pattern, target, 'en_US_POSIX') as matches from strings;