%{
    #include<stdio.h>
    #include "lex.yy.c"
    #include "tree.h"

    int error_occured=0;
%}


%locations

/* declared tokens */
%token INT
%token FLOAT
%token SEMI COMMA
%token ASSIGNOP 
%token RELOP
%token PLUS MINUS STAR DIV
%token AND OR DOT NOT
%token TYPE
%token LP RP LB RB LC RC
%token STRUCT RETURN
%token IF ELSE
%token WHILE
%token ID

/* declared non-terminals */

%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left DOT
%left LB RB
%left LP RP 

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE


%%

/* high-level definitions */
Program : ExtDefList {$$=add_parent_node("Program",@$.first_line,1,$1);root=$$;}
    ;
ExtDefList : ExtDef ExtDefList {$$=add_parent_node("ExtDefList",@$.first_line,2,$1,$2);}
    | /* empty */{$$=NULL;}
    ; 
ExtDef : Specifier ExtDecList SEMI {$$=add_parent_node("ExtDef",@$.first_line,3,$1,$2,$3);}
    | Specifier SEMI {$$=add_parent_node("ExtDef",@$.first_line,2,$1,$2);}
    | Specifier FunDec CompSt {$$=add_parent_node("ExtDef",@$.first_line,3,$1,$2,$3);}
    /*add function declaration*/
    | Specifier FunDec SEMI {$$=add_parent_node("ExtDef",@$.first_line,3,$1,$2,$3);}
    /*new end*/
    | error SEMI { yyerrok; }
    | error ExtDef { yyerrok; }
    | error CompSt { yyerrok; }
    | Specifier error SEMI { yyerrok; }
    | Specifier error CompSt { yyerrok; }
    ;
ExtDecList : VarDec {$$=add_parent_node("ExtDecList",@$.first_line,1,$1);}
    | VarDec COMMA ExtDecList {$$=add_parent_node("ExtDecList",@$.first_line,3,$1,$2,$3);}
    ;

/* specifier */
Specifier : TYPE {$$=add_parent_node("Specifier",@$.first_line,1,$1);}
    | StructSpecifier {$$=add_parent_node("Specifier",@$.first_line,1,$1);}
    ;
StructSpecifier : STRUCT OptTag LC DefList RC {$$=add_parent_node("StructSpecifier",@$.first_line,5,$1,$2,$3,$4,$5);}
    | STRUCT Tag {$$=add_parent_node("StructSpecifier",@$.first_line,2,$1,$2);}
    | STRUCT OptTag LC DefList error RC { yyerrok; }
    ;
OptTag : ID {$$=add_parent_node("OptTag",@$.first_line,1,$1);}
    | /* empty */{$$=NULL;}
    ;
Tag : ID {$$=add_parent_node("Tag",@$.first_line,1,$1);}
    ;

/* declarators */
VarDec : ID {$$=add_parent_node("VarDec",@$.first_line,1,$1);}
    | VarDec LB INT RB {$$=add_parent_node("VarDec",@$.first_line,4,$1,$2,$3,$4);}
    ;
FunDec : ID LP VarList RP {$$=add_parent_node("FunDec",@$.first_line,4,$1,$2,$3,$4);}
    | ID LP RP {$$=add_parent_node("FunDec",@$.first_line,3,$1,$2,$3);}
    ;
VarList : ParamDec COMMA VarList {$$=add_parent_node("VarList",@$.first_line,3,$1,$2,$3);}
    | ParamDec {$$=add_parent_node("VarList",@$.first_line,1,$1);}
    ;
ParamDec : Specifier VarDec {$$=add_parent_node("ParamDec",@$.first_line,2,$1,$2);}
    ;

/* statements */
CompSt : LC DefList StmtList RC {$$=add_parent_node("CompSt",@$.first_line,4,$1,$2,$3,$4);}
    ;
StmtList : Stmt StmtList {$$=add_parent_node("StmtList",@$.first_line,2,$1,$2);}
    | /* empty */{$$=NULL;}
    ;
Stmt : Exp SEMI {$$=add_parent_node("Stmt",@$.first_line,2,$1,$2);}
    | CompSt {$$=add_parent_node("Stmt",@$.first_line,1,$1);}
    | RETURN Exp SEMI {$$=add_parent_node("Stmt",@$.first_line,3,$1,$2,$3);}
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {$$=add_parent_node("Stmt",@$.first_line,5,$1,$2,$3,$4,$5);}
    | IF LP Exp RP Stmt ELSE Stmt {$$=add_parent_node("Stmt",@$.first_line,7,$1,$2,$3,$4,$5,$6,$7);}
    | WHILE LP Exp RP Stmt {$$=add_parent_node("Stmt",@$.first_line,5,$1,$2,$3,$4,$5);}
    | IF LP error RP Stmt %prec LOWER_THAN_ELSE
    | IF LP error RP Stmt ELSE Stmt
    | Exp error { yyerrok; }
    | error SEMI { yyerrok; }
    | error Stmt { yyerrok; }
    | WHILE LP error RP Stmt { yyerrok; }
    ;

/* local definitions */
DefList : Def DefList {$$=add_parent_node("DefList",@$.first_line,2,$1,$2);}
    | /* empty */{$$=NULL;}
    ;
Def : Specifier DecList SEMI {$$=add_parent_node("Def",@$.first_line,3,$1,$2,$3);}
    | Specifier DecList error SEMI { yyerrok; }
    | Specifier error SEMI { yyerrok; }
    ;
DecList : Dec {$$=add_parent_node("DecList",@$.first_line,1,$1);}
    | Dec COMMA DecList {$$=add_parent_node("DecList",@$.first_line,3,$1,$2,$3);}
    ;
Dec : VarDec {$$=add_parent_node("Dec",@$.first_line,1,$1);}
    | VarDec ASSIGNOP Exp {$$=add_parent_node("Dec",@$.first_line,3,$1,$2,$3);}
    ;

/* expressions */
Exp : Exp ASSIGNOP Exp {$$=add_parent_node("Exp",@$.first_line,3,$1,$2,$3);}
    | Exp RELOP Exp {$$=add_parent_node("Exp",@$.first_line,3,$1,$2,$3);}
    | Exp OR Exp {$$=add_parent_node("Exp",@$.first_line,3,$1,$2,$3);}
    | Exp AND Exp {$$=add_parent_node("Exp",@$.first_line,3,$1,$2,$3);}
    | Exp PLUS Exp {$$=add_parent_node("Exp",@$.first_line,3,$1,$2,$3);}
    | Exp MINUS Exp {$$=add_parent_node("Exp",@$.first_line,3,$1,$2,$3);}
    | Exp STAR Exp {$$=add_parent_node("Exp",@$.first_line,3,$1,$2,$3);}
    | Exp DIV Exp {$$=add_parent_node("Exp",@$.first_line,3,$1,$2,$3);}
    | NOT Exp {$$=add_parent_node("Exp",@$.first_line,2,$1,$2);}
    | MINUS Exp {$$=add_parent_node("Exp",@$.first_line,2,$1,$2);}
    | Exp DOT ID {$$=add_parent_node("Exp",@$.first_line,3,$1,$2,$3);}
    | Exp LB Exp RB {$$=add_parent_node("Exp",@$.first_line,4,$1,$2,$3,$4);}
    | LP Exp RP {$$=add_parent_node("Exp",@$.first_line,3,$1,$2,$3);}
    | ID LP Args RP {$$=add_parent_node("Exp",@$.first_line,4,$1,$2,$3,$4);}
    | ID LP RP {$$=add_parent_node("Exp",@$.first_line,3,$1,$2,$3);}
    | ID {$$=add_parent_node("Exp",@$.first_line,1,$1);} 
    | INT {$$=add_parent_node("Exp",@$.first_line,1,$1);}
    | FLOAT {$$=add_parent_node("Exp",@$.first_line,1,$1);}
    | Exp ASSIGNOP error { yyerrok; }
    | Exp RELOP error { yyerrok; }
    | Exp PLUS error { yyerrok; }
    | Exp MINUS error { yyerrok; }
    | Exp STAR error { yyerrok; }  
    | Exp DIV error { yyerrok; }   
    | Exp AND error { yyerrok; }
    | Exp OR error { yyerrok; }
    | MINUS error  { yyerrok; }
    | LP Exp error { yyerrok; }
    | NOT error { yyerrok; }
    | ID LP error RP { yyerrok; }  
    | Exp LB error RB { yyerrok; }      
    | LP error RP { yyerrok; }   
    ;
Args : Exp COMMA Args {$$=add_parent_node("Args",@$.first_line,3,$1,$2,$3);}
    | Exp {$$=add_parent_node("Args",@$.first_line,1,$1);}
    | error COMMA Exp
;

%%



yyerror(char *msg) {
    fprintf(stdout, "Error type B at line %d: %s\n",yylineno,msg);
    error_occured=1;
}
