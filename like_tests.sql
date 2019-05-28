START TRANSACTION;

SELECT 'start collationlike tests';

create table tests (target string, pattern string, locale string, match bool, correct bool);

with strings as (select 'c%de' as pattern, 'cbde' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = true) as correct from strings, matches;

with strings as (select '\\_%\\%\\_' as pattern, '_a%_' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = true) as correct from strings, matches;

with strings as (select 'FUSSBALL' as pattern, 'fußball' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = true) as correct from strings, matches;

with strings as (select 'x' as pattern, 'fußball' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = false) as correct from strings, matches;

with strings as (select 'fußball' as pattern, 'FUSSBALL' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = true) as correct from strings, matches;

with strings as (select 'A' as pattern, 'bcäbc' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = false) as correct from strings, matches;

with strings as (select '%A' as pattern, 'bcäbc' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = false) as correct from strings, matches;

with strings as (select '_A' as pattern, 'bcäbc' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = false) as correct from strings, matches;

with strings as (select '__A' as pattern, 'bcäbc' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = false) as correct from strings, matches;

with strings as (select '_%A' as pattern, 'bcäbc' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = false) as correct from strings, matches;

with strings as (select '%_A' as pattern, 'bcäbc' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = false) as correct from strings, matches;

with strings as (select '__%_A' as pattern, 'bcäbc' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = false) as correct from strings, matches;

with strings as (select '__%_A' as pattern, 'bcäbc' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = false) as correct from strings, matches;

with strings as (select '' as pattern, 'bcäbc' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = false) as correct from strings, matches;

with strings as (select '%c%b' as pattern, 'bcäbc' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = false) as correct from strings, matches;

with strings as (select '%b_A' as pattern, 'bcäbc' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = false) as correct from strings, matches;

with strings as (select '%b__A' as pattern, 'bcäbc' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = false) as correct from strings, matches;

with strings as (select 'b___c' as pattern, 'bcäbc' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = true) as correct from strings, matches;

with strings as (select '_' as pattern, 'bcäbc' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = false) as correct from strings, matches;

with strings as (select '__' as pattern, 'bcäbc' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = false) as correct from strings, matches;

with strings as (select '%' as pattern, 'bcäbc' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = true) as correct from strings, matches;

with strings as (select '%b_' as pattern, 'bcäbc' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = true) as correct from strings, matches;

with strings as (select '%b__' as pattern, 'bcäbc' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = false) as correct from strings, matches;

with strings as (select '%b____' as pattern, 'bcäbc' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = true) as correct from strings, matches;

with strings as (select '%b_%_' as pattern, 'bcäbc' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = true) as correct from strings, matches;

with strings as (select '__ß____' as pattern, 'FUSSBALL' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = true) as correct from strings, matches;

with strings as (select '__ß%' as pattern, 'FUSSBALL' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = true) as correct from strings, matches;

with strings as (select '_A%' as pattern, 'bcäbc' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = false) as correct from strings, matches;

with strings as (select '__A__' as pattern, 'bcäbc' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = true) as correct from strings, matches;

with strings as (select '_%A_%' as pattern, 'bcäbc' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = true) as correct from strings, matches;

with strings as (select '__%_bc' as pattern, 'bcäbc' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = true) as correct from strings, matches;

with strings as (select '\\_\\_\\_\\_' as pattern, 'xxxx' as target, 'de_DE' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = false) as correct from strings, matches;

with strings as (select '%A%' as pattern, 'bcäbc' as target, 'en_US' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = true) as correct from strings, matches;

with strings as (select '%A%' as pattern, 'bcäbc' as target, 'en_US_POSIX' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = false) as correct from strings, matches;

with strings as (select 'FUSSBALL' as pattern, 'fußball' as target, 'en_US' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = true) as correct from strings, matches;

with strings as (select 'FUSSBALL' as pattern, 'fußball' as target, 'en_US_POSIX' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = false) as correct from strings, matches;

with strings as (select null as pattern, '' as target, 'en_US' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = false) as correct from strings, matches;

with strings as (select '' as pattern, '' as target, 'en_US' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = true) as correct from strings, matches;

with strings as (select '%' as pattern, '' as target, 'en_US' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = true) as correct from strings, matches;

with strings as (select 'd' as pattern, '' as target, 'en_US' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = false) as correct from strings, matches;

with strings as (select '' as pattern, 'd' as target, 'en_US' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = false) as correct from strings, matches;

with strings as (select '%_' as pattern, '' as target, 'en_US' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = false) as correct from strings, matches;

with strings as (select '%x' as pattern, '' as target, 'en_US' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = false) as correct from strings, matches;

with strings as (select '' as pattern, '' as target, 'en_US' as locale), matches as (select collationlike(target, pattern, locale) as match from strings)
    insert into tests select target, pattern, locale, match, (match = true) as correct from strings, matches;

select * from tests;

ROLLBACK;