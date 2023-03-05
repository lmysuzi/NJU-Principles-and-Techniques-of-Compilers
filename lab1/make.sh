flex lexical.l
bison -d syntax.y
gcc syntax.tab.c tree.c main.c -lfl -ly -o parser
