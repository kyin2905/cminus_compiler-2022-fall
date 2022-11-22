%option noyywrap
%{
#include <stdio.h>
#include <stdlib.h>

#include "syntax_tree.h"
#include "syntax_analyzer.h"

int files_count;
int lines;
int pos_start;
int pos_end;

void pass_node(char *text){
     yylval.node = new_syntax_tree_node(text);
}
/* 函数的意思是flex将数值存入yylval，而bison读取yylval之中的值，所以对于每个token，我们都应该创建其的结点，这里就是要创建节点 */
%}

/***************
 TO STUDENTS: Copy your Lab1 here. Make adjustments if necessary.

 Note: don't modify the prologue unless you know what you are doing.
***************/

/* Example for you :-) */
/* \+  { pos_start = pos_end; pos_end += 1; pass_node(yytext); return ADD; }*/
/* 这个例子的意思是对于lab1中正则定义即token保持不变，所以pos_start和pos_end的定义保持不变，然后对于每个token，都创建节点 */
%%
\+  { pos_start = pos_end; pos_end += 1; pass_node(yytext); return ADD; }
\- {pos_start=pos_end;pos_end=pos_start+1; pass_node(yytext); return SUB;}
\* {pos_start=pos_end;pos_end=pos_start+1; pass_node(yytext); return MUL;}
\/ {pos_start=pos_end;pos_end=pos_start+1; pass_node(yytext); return DIV;}
\< {pos_start=pos_end;pos_end=pos_start+1; pass_node(yytext); return LT;}
"<=" {pos_start=pos_end;pos_end=pos_start+2; pass_node(yytext); return LTE;}
\> {pos_start=pos_end;pos_end=pos_start+1; pass_node(yytext); return GT;}
">=" {pos_start=pos_end;pos_end=pos_start+2; pass_node(yytext); return GTE;}
"==" {pos_start=pos_end;pos_end=pos_start+2; pass_node(yytext); return EQ;}
"!=" {pos_start=pos_end;pos_end=pos_start+2; pass_node(yytext); return NEQ;}
\= {pos_start=pos_end;pos_end=pos_start+1; pass_node(yytext); return ASSIN;}
\; {pos_start=pos_end;pos_end=pos_start+1; pass_node(yytext); return SEMICOLON;}
\, {pos_start=pos_end;pos_end=pos_start+1; pass_node(yytext); return COMMA;}
\( {pos_start=pos_end;pos_end=pos_start+1; pass_node(yytext); return LPARENTHESE;}
\) {pos_start=pos_end;pos_end=pos_start+1; pass_node(yytext); return RPARENTHESE;}
\[ {pos_start=pos_end;pos_end=pos_start+1; pass_node(yytext); return LBRACKET;}
\] {pos_start=pos_end;pos_end=pos_start+1; pass_node(yytext); return RBRACKET;}
\{ {pos_start=pos_end;pos_end=pos_start+1; pass_node(yytext); return LBRACE;}
\} {pos_start=pos_end;pos_end=pos_start+1; pass_node(yytext); return RBRACE;}
else {pos_start=pos_end;pos_end=pos_start+4; pass_node(yytext); return ELSE;}
if {pos_start=pos_end;pos_end=pos_start+2; pass_node(yytext); return IF;}
int {pos_start=pos_end;pos_end=pos_start+3; pass_node(yytext); return INT;}
float {pos_start=pos_end;pos_end=pos_start+5; pass_node(yytext); return FLOAT;}
return {pos_start=pos_end;pos_end=pos_start+6; pass_node(yytext); return RETURN;}
void {pos_start=pos_end;pos_end=pos_start+4; pass_node(yytext); return VOID;}
while {pos_start=pos_end;pos_end=pos_start+5; pass_node(yytext); return WHILE;}
[a-zA-Z]+ {pos_start=pos_end;pos_end=pos_start+strlen(yytext); pass_node(yytext); return IDENTIFIER;}
[0-9]+ {pos_start=pos_end;pos_end=pos_start+strlen(yytext); pass_node(yytext); return INTEGER;}
[0-9]*\.[0-9]+ {pos_start=pos_end;pos_end=pos_start+strlen(yytext); pass_node(yytext); return FLOATPOINT;}
"[]" {pos_start=pos_end;pos_end=pos_start+2; pass_node(yytext); return ARRAY;}
[0-9]+\. {pos_start=pos_end;pos_end=pos_start+strlen(yytext); pass_node(yytext); return FLOATPOINT;}
\n { lines+=1;pos_start=1;pos_end=1; }//这是换行符，语法分析树不考虑，所以不用创建节点
\/\*([^\*]|(\*)*[^\*\/])*(\*)*\*\/ { 
	pos_start = pos_end;
        int num = count_num_enter(yytext);
        if ( num != 0 ) { //kua
        	lines += num;
                pos_end = strlen(strrchr(yytext, '\n'));
        }
       	else  {pos_end += strlen(yytext);}
}//这是注释符，语法分析树也不考虑，不用创建节点
" " {pos_start = pos_end; pos_end += 1;}//这是空格符，语法分析树也不考虑，不创建节点
\t {pos_start = pos_end; pos_end += 1;}//制表符，不创建
. {return 0;}//错误，不创建节点

%%
int count_num_enter(char *string) {
    int ret = 0;
    int index = 0;
    while(string[index] != '\0') {
        if(string[index] == '\n')
            ret++;
        index++;
    }
    return ret;
}