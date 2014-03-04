open 'testddl'; -- database should be initialized by "testddl" program

delete from Record;

insert into Record values ((('year 1998', 1998), ('year 1999', 1999),
('year 2000', 2000)), ('Merry Christmas', 'Happy New Year'), (1, 2, 3),
((1, 2, 3), (4, 5), (6, 7, 8, 9)),
1, 2, 0.5, true, 3, 1.5, 0, ('hello world', -1), (65, 66, 67, 68, 69, 70));

insert into Record values ((('Old Year 1998', 1998), ('New Year 1999', 1999),
('Last Year 2000', 2000)), ('Best wishes', 'Happy birthday to you', 'Regards'),
(), (), 4, 5, 0.05, false, 6, 0.005, 0, ('hi', +1), (52, 51, 50, 49)),

((('3', 3), ('2', 2), ('1', 1)), ('Start'), (-1, -2, -3), 
((), (1), (2, 3)),
7, 8, 0.01, true, 9, 0.001, 0, ('hello', 0), ()),

(((':-)', 1), (':-|', 0), (':-(', -1)), ('good', 'normal', 'bad'), (0), 
((1), (), (2)),
10, 11, -1.2, false, 12, 1.0e5, 0, ('ok', 100), (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 ,14, 15));

select * from Record;
exit
