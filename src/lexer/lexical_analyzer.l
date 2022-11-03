%option noyywrap
%{
/*****************声明和选项设置  begin*****************/
#include <stdio.h>
#include <stdlib.h>

#include "lexical_analyzer.h"

int lines;
int pos_start;
int pos_end;

/*****************声明和选项设置  end*****************/

%}
 

%%

 /******************TO DO*********************/
 /****请在此补全所有flex的模式与动作  start******/
 //STUDENT TO DO

 /*运算符*/
 /*ADD*/
\+ {pos_start = pos_end; pos_end++; return ADD;}
 /*这里的意思是pos_start下一个字符的开始位置等于上一个识别出来的字符的结束位置，我们可以保证上一个字符的结束位置一定是下一个字符的开始位置，因为我们可以识别空格，空格的结束位置，就是下一个字符的开始位置，这里因为只有一个符号，所以pos_end+1，下面的同理*/
 /*SUB*/
\- {pos_start = pos_end; pos_end++; return SUB;}
 /*MUL*/
\* {pos_start = pos_end; pos_end++; return MUL;}
 /*DIV*/
\/ {pos_start = pos_end; pos_end++; return DIV;}
 /*LT*/
\< {pos_start = pos_end; pos_end++; return LT;}
 /*LTE*/
\<\= {pos_start = pos_end; pos_end += 2; return LTE;}
 /*两个字符，加2*/
 /*GT*/
\> {pos_start = pos_end; pos_end++; return GT;}
 /*GTE*/
\>\= {pos_start = pos_end; pos_end += 2; return GTE;}
 /*EQ*/
\=\= {pos_start = pos_end; pos_end += 2; return EQ;}
 /*NEQ*/
\!\= {pos_start = pos_end; pos_end += 2; return NEQ;}
 /*ASSIN*/
\= {pos_start = pos_end; pos_end += 1; return ASSIN;}
 /*语法符号*/
 /*SEMICOLON*/
\; {pos_start = pos_end; pos_end += 1; return SEMICOLON;}
 /*和上面的运算符一样*/
 /*COMMA*/
\, {pos_start = pos_end; pos_end += 1; return COMMA;}
 /*LPARENTHESE*/
\( {pos_start = pos_end; pos_end += 1; return LPARENTHESE;}
 /*RPARENTHESE*/
\) {pos_start = pos_end; pos_end += 1; return RPARENTHESE;}
 /*LBRACKET*/
\[ {pos_start = pos_end; pos_end += 1; return LBRACKET;}
 /*RBRACKET*/
\] {pos_start = pos_end; pos_end += 1; return RBRACKET;}
 /*LBRACE*/
\{ {pos_start = pos_end; pos_end += 1; return LBRACE;}
 /*RBRACE*/
\} {pos_start = pos_end; pos_end += 1; return RBRACE;}
 /*关键字*/
 /*这里也跟上面一样，pos_end加上关键字的字符长度就行*/
 /*ELSE*/
else {pos_start = pos_end; pos_end += 4; return ELSE;}
 /*IF*/
if {pos_start = pos_end; pos_end += 2; return IF;}
 /*INT*/
int {pos_start = pos_end; pos_end += 3; return INT;}
 /*FLOAT*/
float {pos_start = pos_end; pos_end += 5; return FLOAT;}
 /*RETURN*/
return {pos_start = pos_end; pos_end += 6; return RETURN;}
 /*VOID*/
void {pos_start = pos_end; pos_end += 4; return VOID;}
 /*WHILE*/
while {pos_start = pos_end; pos_end += 5; return WHILE;}
 /*数字和字母
 IDENTIFIER*/
[a-zA-Z]+ {pos_start = pos_end; pos_end += strlen(yytext); return IDENTIFIER;}
 /*跟上面一样，pos_start等于上一个字符串的结束位置，pos_end要加上现在识别到的字符串的长度，由示例我们知道长度是strlen(yytext)*/
 /*INTGER*/
0|[1-9]+[0-9]* {pos_start = pos_end; pos_end += strlen(yytext); return INTEGER;}
 /*FLOATPOINT*/
(0|[1-9]+[0-9]*)\.[0-9]* {pos_start = pos_end; pos_end += strlen(yytext); return FLOATPOINT;}
 /*ARRAY*/
\[\] {pos_start = pos_end; pos_end += strlen(yytext); return ARRAY;}
 /*LETTER*/
[a-zA-Z] {pos_start = pos_end; pos_end += 1; return LETTER;}
 /*其他字符，下面的C代码已经给出了要处理的动作，现在这里我们直接返回对应的字符就好了*/
 /*EOL*/
\n {return EOL;}
 /*COMMENT*/
\/\*(?:[^\*]|\*+[^\/\*])*\*+\/ {return COMMENT;}
 /*BLANK*/
[ \f\n\r\t\v] {return BLANK;}
 /*ERROR*/
. {return ERROR;}

 /****请在此补全所有flex的模式与动作  end******/
%%
/****************C代码 start*************/

/// \brief analysize a *.cminus file
///
/// \param input_file, 需要分析的文件路径
/// \param token stream, Token_Node结构体数组，用于存储分析结果，具体定义参考lexical_analyer.h

void analyzer(char* input_file, Token_Node* token_stream){
    lines = 1;
    pos_start = 1;
    pos_end = 1;
    if(!(yyin = fopen(input_file,"r"))){
        printf("[ERR] No input file\n");
        exit(1);
    }
    printf("[START]: Read from: %s\n", input_file);

    int token;
    int index = 0;

    while(token = yylex()){
        switch(token){
            case COMMENT:
                //STUDENT TO DO，遇到注释的动作
                pos_start = pos_end;
                for(int i = 0; i < strlen(yytext); i++){
                	if(yytext[i] == '\n'){//注意，换行符的长度是1，只是用两个符号表示而已
                		pos_end = 1;
                		pos_start = 1;
                		lines++;
                	} else{
                		pos_end++;
                	}
                }
                break;
            case BLANK:
                //STUDENT TO DO
                pos_start = pos_end;
                pos_end += strlen(yytext);
                //strlen(yytext)获取文本长度，这里可以获取到全部的空格长度
                break;
            case EOL:
                //STUDENT TO DO
                pos_start = 1;
                pos_end = 1;
                lines += strlen(yytext);
                //获取有多少个换行符，换行之后重新计数
                break;
            case ERROR:
                printf("[ERR]: unable to analysize %s at %d line, from %d to %d\n", yytext, lines, pos_start, pos_end);
            default :
                if (token == ERROR){
                    sprintf(token_stream[index].text, "[ERR]: unable to analysize %s at %d line, from %d to %d", yytext, lines, pos_start, pos_end);
                } else {
                    strcpy(token_stream[index].text, yytext);
                }
                token_stream[index].token = token;
                token_stream[index].lines = lines;
                token_stream[index].pos_start = pos_start;
                token_stream[index].pos_end = pos_end;
                index++;
                if (index >= MAX_NUM_TOKEN_NODE){
                    printf("%s has too many tokens (> %d)", input_file, MAX_NUM_TOKEN_NODE);
                    exit(1);
                }
        }
    }
    printf("[END]: Analysis completed.\n");
    return;
}



/****************C代码 end*************/
