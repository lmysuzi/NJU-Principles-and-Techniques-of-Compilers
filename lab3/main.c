#include <stdio.h>
#include "syntax.tab.h"
#include "tree.h"
#include "table.h"
#include "translate.h"
extern int error_occured;

void yyrestart(FILE *input_file);

int main(int argc, char **argv)
{
    extern FILE *yyin;
    if (argc > 1)
    {
        if (!(yyin = fopen(argv[1], "r")))
        {
            perror(argv[1]);
            return 1;
        }
    }
    yyrestart(yyin);
    yyparse();
    if (!error_occured)
    {
        // print_tree(root, 0);
        semantic_analyse(root);
        if (argc > 2)
        {
            translate(root, argv[2]);
        }
    }
    return 0;
}