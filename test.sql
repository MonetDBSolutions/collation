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

explain select strxfrm('asdsad', 'de_DE.utf8');
select strxfrm('asdsad', 'de_DE.utf8');

explain select * from foo order by strxfrm(s, (select 'de_DE.utf8'));
select * from foo order by strxfrm(s, (select 'de_DE.utf8'));

select * from foo order by s;

select * from foo where s like '%Mu%';
select * from foo where s like '%Mü%';
select * from foo where s ilike '%SS%';
select * from foo where s ilike '%ß%';

ROLLBACK;
