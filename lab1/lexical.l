%{
    #define YYSTYPE node_t*

    #include "tree.h"
    #include "syntax.tab.h"

    int yycolumn = 1;
    #define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; \
    yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;
%}

%option yylineno
digit [0-9]
type int|float
character [a-zA-Z]
id ((_|{character})+)((_|{character}|{digit})*)
relop >|<|>=|<=|==|!=
int ([1-9]{digit}*|0)|(0[0-7]+)|((0x|0X)[0-9a-fA-F]+)
float ([0-9]*\.[0-9]+|[0-9]+\.)(((e|E)([+-]?)([0-9]+))?)
white [\n\t\r ]
%%
{int} { 
    char *text=(char*)malloc(strlen(yytext+1));
    strcpy(text,yytext);
    yylval = establish_leaf_node(INT_T,text);
    return INT; 
}
{float} { 
    char *text=(char*)malloc(strlen(yytext+1));
    strcpy(text,yytext);
    yylval = establish_leaf_node(FLOAT_T,text);
    return FLOAT; 
}
";" {
    yylval=establish_leaf_node(COMMON_T,"SEMI"); 
    return SEMI; 
}
"," { 
    yylval=establish_leaf_node(COMMON_T,"COMMA"); 
    return COMMA; 
}
"=" {
    yylval=establish_leaf_node(COMMON_T,"ASSIGNOP"); 
    return ASSIGNOP;
}
{relop} { 
    yylval=establish_leaf_node(COMMON_T,"RELOP"); 
    return RELOP;
}
"+" {
    yylval=establish_leaf_node(COMMON_T,"PLUS"); 
    return PLUS;
}
"-" {
    yylval=establish_leaf_node(COMMON_T,"MINUS"); 
    return MINUS;
}
"*" {
    yylval=establish_leaf_node(COMMON_T,"STAR"); 
    return STAR;
}
"/" {
    yylval=establish_leaf_node(COMMON_T,"DIV"); 
    return DIV;
}
"&&" {
    yylval=establish_leaf_node(COMMON_T,"AND"); 
    return AND;
}
"||" {
    yylval=establish_leaf_node(COMMON_T,"OR"); 
    return OR;
}
"." {
    yylval=establish_leaf_node(COMMON_T,"DOT"); 
    return DOT;
}
"!" {
    yylval=establish_leaf_node(COMMON_T,"NOT"); 
    return NOT;
}
{type} {
    char *text=(char*)malloc(strlen(yytext+1));
    strcpy(text,yytext);
    yylval=establish_leaf_node(TYPE_T,text); 
    return TYPE;
}
"(" {
    yylval=establish_leaf_node(COMMON_T,"LP"); 
    return LP;
}
")" {
    yylval=establish_leaf_node(COMMON_T,"RP"); 
    return RP;
}
"[" {
    yylval=establish_leaf_node(COMMON_T,"LB"); 
    return LB;
}
"]" {
    yylval=establish_leaf_node(COMMON_T,"RB"); 
    return RB;
}
"{" {
    yylval=establish_leaf_node(COMMON_T,"LC"); 
    return LC;
}
"}" {
    yylval=establish_leaf_node(COMMON_T,"RC"); 
    return RC;
}
"struct" {
    yylval=establish_leaf_node(COMMON_T,"STRUCT"); 
    return STRUCT;
}
"return" {
    yylval=establish_leaf_node(COMMON_T,"RETURN"); 
    return RETURN;
}
"if" {
    yylval=establish_leaf_node(COMMON_T,"IF"); 
    return IF;
}
"else" {
    yylval=establish_leaf_node(COMMON_T,"ELSE"); 
    return ELSE;
}
"while" {
    yylval=establish_leaf_node(COMMON_T,"WHILE"); 
    return WHILE;
}
"\n" {yycolumn=1;}
{id} {
    char *text=(char*)malloc(strlen(yytext+1));
    strcpy(text,yytext);
    yylval=establish_leaf_node(ID_T,text);
    return ID; 
}
{white}+ {}
"//" {
    char c = input();
    while (c != '\n') c = input();
}
"/*" {
    char last=' ';
    char now=input();
    while((last!='*')||(now!='/')) {
        last=now;
        now=input();
    }
}

. {printf("Error type A at Line %d: Mysterious characters \'%s\'\n",yylineno, yytext);}
%%