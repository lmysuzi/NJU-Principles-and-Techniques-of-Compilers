flex lexical.l
bison -d syntax.y
gcc translate.c errorhandle.c list.c hash.c table.c syntax.tab.c tree.c main.c -lfl -ly -o ./parser

./parser Test/selftest.cmm out1.ir
echo 李明扬别卷了快点回来搞枪战啊！！！！！！！！！别勾八写了啊明天再搞
echo 速速打枪速速打枪！速速打枪速速打枪！速速打枪速速打枪！速速打枪速速打枪！速速打枪速速打枪！