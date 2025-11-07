%code requires{
  #include <memory>
  #include <vector>
  #include <string>
  struct Expr; struct Stmt; struct Block; struct Function; struct Program;
}

%code{
  #include <cstdio>
  #include <cstdlib>
  #include <iostream>
  #include "ast.hpp"

  extern int yylineno;

  std::unique_ptr<Program> g_prog;
  int yylex();
  void yyerror(const char* s){ std::cerr << "Parse error: " << s << " at line " << yylineno << "\n"; }
}

/* === UNION === */
%union {
  int  ival;
  char* sval;
  Expr* expr;
  Stmt* stmt;
  Block* block;
  Function* func;
  std::vector<std::string>* idlist;
}

/* === ТОКЕНИ === */
%token T_INT T_RETURN T_IF T_ELSE T_WHILE
%token <sval> T_ID
%token <ival> T_NUM
%token T_EQ T_NEQ T_LE T_GE T_LT T_GT

/* === ПРІОРИТЕТИ === */
%left T_EQ T_NEQ T_LT T_LE T_GT T_GE
%left '+' '-'
%left '*' '/' '%'
%right '='

/* === ТИПИ НЕТЕРМІНАЛІВ === */
%type <expr>   expr
%type <stmt>   stmt decl assign optelse
%type <block>  block stmts
%type <func>   extdef
%type <idlist> params param

%start program
%expect 1   /* dangling else */

%%

program
  : /*empty*/                { g_prog = std::make_unique<Program>(); }
  | program extdef           { g_prog->funcs.emplace_back($2); }
  ;

extdef
  : T_INT T_ID '(' params ')' block
    {
        /* Function копіює name та params; тут звільняємо лише сирі ресурси */
        $$ = new Function($2, *$4, std::unique_ptr<Block>($6));
        free($2);      /* T_ID виділявся через _strdup у lexer.l */
        delete $4;     /* params створювались через new у rules param/params */
    }
  ;

params
  : /*empty*/                { $$ = new std::vector<std::string>(); }
  | param                    { $$ = $1; }
  | params ',' param         { $1->insert($1->end(), $3->begin(), $3->end()); delete $3; $$ = $1; }
  ;

param
  : T_INT T_ID               { $$ = new std::vector<std::string>(); $$->push_back($2); free($2); }
  ;

block
  : '{' stmts '}'            { $$ = $2; }
  ;

stmts
  : /*empty*/                { $$ = new Block(); }
  | stmts stmt               { $1->stmts.emplace_back(std::unique_ptr<Stmt>($2)); $$ = $1; }
  ;

stmt
  : decl ';'                 { $$ = $1; }
  | assign ';'               { $$ = $1; }
  | T_IF '(' expr ')' stmt optelse
      { $$ = new If(std::unique_ptr<Expr>($3), std::unique_ptr<Stmt>($5), std::unique_ptr<Stmt>($6)); }
  | T_WHILE '(' expr ')' stmt
      { $$ = new While(std::unique_ptr<Expr>($3), std::unique_ptr<Stmt>($5)); }
  | block                    { $$ = $1; }
  | T_RETURN expr ';'        { $$ = new Ret(std::unique_ptr<Expr>($2)); }
  ;

optelse
  : /*empty*/                { $$ = nullptr; }
  | T_ELSE stmt              { $$ = $2; }
  ;

decl
  : T_INT T_ID               { $$ = new Decl($2, nullptr); }
  | T_INT T_ID '=' expr      { $$ = new Decl($2, std::unique_ptr<Expr>($4)); }
  ;

assign
  : T_ID '=' expr            { $$ = new Assign($1, std::unique_ptr<Expr>($3)); }
  ;

expr
  : T_NUM                    { $$ = new Num($1); }
  | T_ID                     { $$ = new Var($1); }
  | '(' expr ')'             { $$ = $2; }
  | expr '+' expr            { $$ = new Bin("+", std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); }
  | expr '-' expr            { $$ = new Bin("-", std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); }
  | expr '*' expr            { $$ = new Bin("*", std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); }
  | expr '/' expr            { $$ = new Bin("/", std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); }
  | expr '%' expr            { $$ = new Bin("%", std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); }
  | expr T_EQ  expr          { $$ = new Bin("==", std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); }
  | expr T_NEQ expr          { $$ = new Bin("!=",  std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); }
  | expr T_LT  expr          { $$ = new Bin("<",   std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); }
  | expr T_LE  expr          { $$ = new Bin("<=",  std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); }
  | expr T_GT  expr          { $$ = new Bin(">",   std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); }
  | expr T_GE  expr          { $$ = new Bin(">=",  std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); }
  ;

%%
