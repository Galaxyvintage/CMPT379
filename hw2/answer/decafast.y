%{
#include <iostream>
#include <ostream>
#include <string>
#include <cstdlib>
#include "decafast-defs.h"

int yylex(void);
int yyerror(char *); 

// print AST?
bool printAST = true;

#include "decafast.cc"

using namespace std;
%}

%union
{
  class decafAST *ast;
  std::string *sval;
  std::deque<string> *deque_ptr;
  int  ival;
}

%token T_FUNC
%token T_PACKAGE
%token T_VAR
%token T_INTTYPE 
%token T_STRINGTYPE 
%token T_BOOLTYPE 
%token T_VOID
%token T_NULL
%token T_BREAK 
%token T_CONTINUE 
%token T_EXTERN 
%token T_TRUE
%token T_FALSE
%token T_IF 
%token T_ELSE
%token T_FOR 
%token T_WHILE
%token T_RETURN 

%token T_LCB
%token T_RCB
%token T_LPAREN
%token T_RPAREN
%token T_LSB
%token T_RSB
%token T_COMMA
%token T_SEMICOLON

%token T_EQ
%token T_LEQ
%token T_GEQ
%token T_NEQ
%token T_LEFTSHIFT
%token T_RIGHTSHIFT
%token T_AND
%token T_OR

%token T_PLUS
%token T_MINUS
%token T_MULT
%token T_DIV
%token T_NOT
%token T_ASSIGN
%token T_LT
%token T_RT
%token T_MOD
%token T_DOT

%token T_COMMENT
%token T_INTCONSTANT
%token T_CHARCONSTANT
%token T_STRINGCONSTANT
%token T_BOOLCONSTANT
%token <sval> T_ID
%token T_WHITESPACE
%token T_UNARYMINUS
%token T_ERROR_1
%token T_ERROR_2
%token T_ERROR_3

%type <ast> extern_list decafpackage field_decls field_decl
%type <deque_ptr> id_comma_list
%type <ival> decaf_type
%%

/* start */
start: program

program: extern_list decafpackage
   { 
    ProgramAST *prog = new ProgramAST((decafStmtList *)$1, (PackageAST *)$2); 
    if (printAST) 
    {
      cout << getString(prog) << endl;
    }
    delete prog;
  }
  ;
extern_list: /* extern_list can be empty */
{ decafStmtList *slist = new decafStmtList(); $$ = slist; }
  ;
decafpackage: T_PACKAGE T_ID T_LCB field_decls method_decls T_RCB
{ 
  $$ = new PackageAST(*$2, (decafStmtList*)$4, new decafStmtList()); 
  delete $2; 
}
  ;

field_decls: /* Empty(zero or more) */
{ $$ = NULL; }
           | field_decl field_decls 
{
  decafStmtList* slist;
  decafStmtList* p;
  decafStmtList* q;
  p = (decafStmtList*)$1;
  if( $2 == NULL )
  { 
    slist = p;
  }   
  
  else // if( $2 != nullptr )
  {
    q = (decafStmtList*)$2;
    int size = p->size();
    for(int x = 0; x < size; x++)
    { q->push_front(p->pop_back());}
    slist = q;
    delete p;
  }
  $$ = slist;
}                
  ;
field_decl: T_VAR id_comma_list decaf_type T_SEMICOLON   
{ 
  decafStmtList* slist = new decafStmtList();
  string FieldType;
  FieldAST* e;
  
  //cout<<"decaf_type:"<<$3<<endl;;
  if($3 == 0)
  {
    FieldType = string("IntType");
  }
  else
  {
    FieldType = string("BoolType");
  }
  for(int x = 0; x < $2->size(); ++x)
  {
    e = new FieldAST((*$2)[x],FieldType,"Scalar");
    slist->push_back(e);
  }    
  $$ = slist;
  delete $2;
} 
            // | T_VAR id_comma_list array_type T_SEMICOLON  
            // | T_VAR id_comma_list decaf_type T_ASSIGN constant T_SEMICOLON 
  ;
id_comma_list: T_ID T_COMMA id_comma_list
{
  //cout<<"T_ID is"<<*$1<<endl;
  deque<string>* ilist = $3;
  ilist->push_front(*$1);
  delete $1;
  $$ = ilist;
}
             | T_ID   
{
  //cout<<"T_ID is"<<*$1<<endl;
  deque<string>* ilist = new deque<string>;
  ilist->push_front(*$1);
  delete $1;
  $$ = ilist; 
}          
  ;   
argument_list: /* Empty(zero or more) */
             | id_type_comma_list 
  ;
id_type_comma_list: T_ID decaf_type T_COMMA id_type_comma_list
                  | T_ID decaf_type
  ;
method_decls: /* Empty(zero or more) */
            | method_decl method_decls
  ;
method_decl: T_FUNC T_ID T_LCB argument_list T_RCB method_type block
  ;
var_decls: /* Empty(zero or more) */
         | var_decl var_decls 
  ;
var_decl: T_VAR id_comma_list decaf_type T_SEMICOLON 
  ;
block: T_LCB var_decls T_RCB
  ;

/*
statements: // Empty(zero or more)  
          | statement statements 
  ;
statement: block
         | assign T_SEMICOLON
         | method_call T_SEMICOLON
         | if_stmt T_SEMICOLON
         | while_stmt T_SEMICOLON
         | for_stmt T_SEMICOLON
         | return_stmt T_SEMICOLON
         | break_stmt T_SEMICOLON
         | continue_stmt T_SEMICOLON
  ; 
block: T_LCB var_decls statements T_RCB
  ;
assign: lvalue T_EQ expr  
  ;
lvalue: T_ID 
      | T_ID T_LSB expr T_RSB
  ;
method_call: T_ID T_LPAREN method_arg_list T_RPAREN
  ;
method_arg_list: // Empty(zero or more) 
               | method_arg_comma_list 
  ;
method_arg_comma_list: method_arg T_COMMA method_arg_comma_list
                     | method_arg
  ; 
method_arg: expr | T_STRINGCONSTANT
  ;
if_stmt: T_IF T_LPAREN expr T_RPAREN block T_ELSE block
       | T_IF T_LPAREN expr T_RPAREN block 
  ;
while_stmt: T_WHILE T_LPAREN expr T_RPAREN block
  ;   
for_stmt: T_FOR T_LPAREN pre_assign_list T_SEMICOLON expr T_SEMICOLON post_assign_list T_RPAREN block
  ;
pre_assign_list: assign_comma_list
  ;
post_assign_list: assign_comma_list
  ;
assign_comma_list: assign T_COMMA assign_comma_list
                   | assign
  ;
return_stmt: T_RETURN T_LPAREN expr T_RPAREN
           | T_RETURN 
  ;
break_stmt: T_BREAK 
  ;
continue_stmt: T_CONTINUE 
  ;
expr: T_ID
    | method_call
    | constant 
    | expr binary_op expr
    | unary_op expr
    | T_LPAREN expr T_RPAREN  
    | T_ID T_LSB expr T_RSB
  ;   
binary_op: arithmatic_op
         | boolean_op
  ;
arithmatic_op: T_PLUS
             | T_MINUS
             | T_MULT
             | T_DIV
             | T_LEFTSHIFT
             | T_RIGHTSHIFT
             | T_MOD
  ;
boolean_op: T_EQ
          | T_NEQ
          | T_LT
          | T_LEQ
          | T_RT
          | T_GEQ
          | T_AND
          | T_OR
  ;
unary_op: T_UNARYMINUS
        | T_NOT  
  ;
*/
constant: T_INTCONSTANT 
        | T_CHARCONSTANT
        | T_BOOLCONSTANT
  ;
decaf_type: T_INTTYPE
{  
   //cout<<"int here"<<endl;
   $$ = 0;
 }     
          | T_BOOLTYPE
{
  //cout<<"bool here"<<endl; 
  $$ = 1; 
}
  ;
array_type: T_LSB T_INTCONSTANT T_RSB decaf_type
  ;
method_type: T_VOID | decaf_type
  ;
extern_type: T_STRINGTYPE | decaf_type
  ;

%%  

int main()
{
  // parse the input and create the abstract syntax tree
  int retval = yyparse();
  return(retval >= 1 ? EXIT_FAILURE : EXIT_SUCCESS);
}
