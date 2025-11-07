%option noyywrap
%option nounput noinput
%option yylineno
%{
#define YY_NO_UNISTD_H 1
#include "parser.hpp"
  #include <cstdlib>
  #include <io.h>         /* äëÿ Windows */
  #define fileno  _fileno
  #define isatty  _isatty
  #define read    _read
  #define write   _write
  #define close   _close
%}

/* === ØÀÁËÎÍÈ === */
ws      [ \t\r\f\v]+
id      [a-zA-Z_][a-zA-Z_0-9]*
num     [0-9]+
comment \/\*([^*]|\*+[^\/])*\*+\/
linec   \/\/.*

%%

{ws}            ;
{linec}         ;
{comment}       ;

"int"           { return T_INT; }
"return"        { return T_RETURN; }
"if"            { return T_IF; }
"else"          { return T_ELSE; }
"while"         { return T_WHILE; }

"=="            { return T_EQ; }
"!="            { return T_NEQ; }
"<="            { return T_LE; }
">="            { return T_GE; }
"<"             { return T_LT; }
">"             { return T_GT; }

"="             { return '='; }
"+"             { return '+'; }
"-"             { return '-'; }
"*"             { return '*'; }
"/"             { return '/'; }
"%"             { return '%'; }
"("             { return '('; }
")"             { return ')'; }
"{"             { return '{'; }
"}"             { return '}'; }
","             { return ','; }
";"             { return ';'; }

{num}           { yylval.ival = atoi(yytext); return T_NUM; }
{id}            { yylval.sval = _strdup(yytext); return T_ID; }

\n              ;
.               { return yytext[0]; }

%%
