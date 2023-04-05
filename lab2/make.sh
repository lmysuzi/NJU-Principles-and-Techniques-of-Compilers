flex lexical.l
bison -d syntax.y
gcc errorhandle.c list.c hash.c table.c syntax.tab.c tree.c main.c -lfl -ly -o parser

./parser test/selftest.cmm
./parser test/test.cmm
./parser test/test1.cmm
./parser test/test2.cmm
./parser test/test3.cmm
./parser test/test4.cmm
./parser test/test5.cmm
./parser test/test6.cmm
./parser test/test7.cmm
./parser test/test8.cmm
./parser test/test9.cmm
./parser test/test10.cmm
./parser test/test11.cmm
./parser test/test12.cmm
./parser test/test13.cmm
./parser test/test14.cmm
./parser test/test15.cmm
./parser test/test16.cmm