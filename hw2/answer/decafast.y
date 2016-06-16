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
  int ival;
  array_info arrinfo;

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
%token <sval> T_TRUE
%token <sval> T_FALSE
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
%token <sval> T_INTCONSTANT
%token <sval> T_CHARCONSTANT
%token <sval> T_STRINGCONSTANT
%token <sval> T_ID
%token T_WHITESPACE
%token T_UNARYMINUS
%token T_ERROR_1
%token T_ERROR_2
%token T_ERROR_3

%type <ast> extern_list extern_def extern_type_comma_list decafpackage

%type <ast> field_decls field_decl 

%type <ast> method_decls method_decl param_list id_type_comma_list
 
%type <ast> block var_decls var_decl

%type <ast> statements statement

%type <ast> expr value constant assign method_call method_arg_list

%type <deque_ptr> id_comma_list 

%type <sval> extern_type decaf_type method_type binary_op arithmatic_op boolean_op bool_constant

%type <arrinfo> array_type

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
           { $$ = NULL; }
           | extern_def extern_list
           {
             decafStmtList* slist;
	     if($2 == NULL)
	     {
	       slist = new decafStmtList();
             }
             else
	     {
               slist = (decafStmtList*)$2;
	     }
	     slist->push_front($1);
             $$ = slist;
	   }  
           ;

extern_def: T_EXTERN T_FUNC T_ID T_LPAREN extern_type_comma_list T_RPAREN method_type T_SEMICOLON
          {
            ExternAST* e = new ExternAST(*$3, (decafStmtList*)$5, *$7);
             $$ = e;
             delete $3;
             delete $7;
          }      
          | T_EXTERN T_FUNC T_ID T_LPAREN T_VOID T_RPAREN method_type T_SEMICOLON
	  {
            ExternAST* e = new ExternAST(*$3, NULL, *$7); // no argument 
            $$ = e;
            delete $3;
            delete $7;
	  }  
          ;
extern_type_comma_list: extern_type T_COMMA extern_type_comma_list
                      {
                        decafStmtList* slist = (decafStmtList*)$3;
                        VarDefAST* e = new VarDefAST(string(""), *$1);
                        slist->push_front(e);
                        $$ = slist;
                        delete $1;
                      } 
                      | extern_type
                      {
                        decafStmtList* slist = new decafStmtList();
                        VarDefAST* e = new VarDefAST(string(""),*$1);
                        slist->push_front(e);
                        $$ = slist;
                        delete $1;     
		      }
                      ;

decafpackage: T_PACKAGE T_ID T_LCB field_decls method_decls T_RCB
            { 
              $$ = new PackageAST(*$2, (decafStmtList*)$4, (decafStmtList*)$5 ); 
              delete $2; 
            }
            ;

field_decls: /* Empty(zero or more) */
           { $$ = NULL; }
           | field_decl field_decls 
           {            
             decafStmtList* slist;
             if($2 == NULL)
	     {
               slist = new decafStmtList();
	     }
             else
	     {
               slist = (decafStmtList*)$2;
	     }

             slist->push_front($1);
             $$ = slist;
           }                     
           ;
field_decl: T_VAR id_comma_list decaf_type T_SEMICOLON   
            { 
              decafStmtList* slist = new decafStmtList();
              FieldAST* e;
  
              for(int x = 0; x < $2->size(); ++x)
              {
                e = new FieldAST((*$2)[x],*$3,"Scalar");
                slist->push_back(e);
              }    
  
              delete $2;
              delete $3;
              $$ = slist; 
            } 

          | T_VAR id_comma_list array_type T_SEMICOLON  
            {
              decafStmtList* slist = new decafStmtList();
              string FieldType;
              string FieldSize; 
              FieldAST* e;
               
              FieldType = *($3.type);
              FieldSize = string("Array(") + *($3.size) + ")";

              for(int x = 0; x < $2->size(); ++x)
              {
                e = new FieldAST((*$2)[x],FieldType, FieldSize);
                slist->push_back(e);
              }    

              delete $2; 
              delete $3.type;
              delete $3.size;
              $$ = slist;
	    }
	    | T_VAR id_comma_list decaf_type T_ASSIGN expr T_SEMICOLON
	    {
              decafStmtList* slist = new decafStmtList();
              FieldAST* e;
  
              for(int x = 0; x < $2->size(); ++x)
              {    
                e = new FieldAST((*$2)[x],*$3,(ConstantAST*)$5);
                e->setAssignment(true);
                slist->push_back(e);
              }    
  
              delete $2;
              delete $3;
              $$ = slist; 
            }
  ;
id_comma_list: T_ID T_COMMA id_comma_list
{
  deque<string>* ilist = $3;
  ilist->push_front(*$1);
  delete $1;
  $$ = ilist;
}
              | T_ID   
{
  deque<string>* ilist = new deque<string>;
  ilist->push_front(*$1);
  delete $1;
  $$ = ilist; 
}          
  ;   
method_decls: /* Empty(zero or more) */
            { $$ = NULL; }
            | method_decl method_decls
            {
              decafStmtList* slist;
              if($2 == NULL)
              {
                slist = new decafStmtList();
	      }
              else
              {
                slist = (decafStmtList*)$2;
	      }
           
              slist->push_front($1);
              $$ = slist;
            }                     
            ;
method_decl: T_FUNC T_ID T_LPAREN param_list T_RPAREN method_type block
            { 
              decafStmtList* slist = new decafStmtList();
              decafStmtList* q;

              string MethodType;
              MethodAST* e;
              MethodType = *$6;
              
              ((BlockAST*)$7)->setMethodBlock(true);
                 
              e = new MethodAST((*$2),MethodType,
                                (decafStmtList*)$4, 
                                (BlockAST*)$7);

              slist->push_back(e);
               
              delete $2;  // free T_OD
              delete $6;  // free method_type
              $$ = slist; 
            } 
            ;
param_list: /* Empty(zero or more) */
             { $$ = NULL; }
             | id_type_comma_list 
             { $$ = $1; }
  ;           
id_type_comma_list: T_ID decaf_type T_COMMA id_type_comma_list
                  {
		    VarDefAST* e;
                    e = new VarDefAST(*$1,*$2);
                    ((decafStmtList*)$4)->push_front(e);

                    $$ = $4;
                    delete $1;
                    delete $2;
                  }
                  | T_ID decaf_type
                  {
                    decafStmtList* slist = new decafStmtList();
                    VarDefAST* e;

                    e = new VarDefAST(*$1, *$2);
                      
                    slist->push_front(e);

                    delete $1; // free T_ID string 
                    delete $2; // free decaf_type string 
                    $$ = slist;
                  }  
  ;
block: T_LCB var_decls statements T_RCB
     {   
       BlockAST* e;
       e = new BlockAST((decafStmtList*)$2,(decafStmtList*)$3);
       $$ = e;
     }

  ;
var_decls: /* Empty(zero or more) */
         { $$ = NULL;}
         | var_decl var_decls 
         {    
           decafStmtList* slist;
           if( $2 == NULL )
           {
             slist = new decafStmtList();
           }   
           else // if( $2 != nullptr )
           { 
             slist = (decafStmtList*)$2;      
           }
           slist->push_front($1);
           $$ = slist;
         }        
         ;
var_decl: T_VAR id_comma_list decaf_type T_SEMICOLON
        {
          decafStmtList* slist = new decafStmtList();
          VarDefAST* e;
           
          for(int x = 0; x < $2->size(); ++x)
	  {
            e = new VarDefAST((*$2)[x], *$3);
            slist->push_back(e);
	  }

          delete $2;
          delete $3;
          $$ = slist;
        }   
  ;

statements: // Empty(zero or more)  
         { $$ = NULL; } 
         | statement statements 
         { 
           decafStmtList* slist;
           if($2 == NULL)
           {
             slist = new decafStmtList();
	   }
           else
           {
             slist = (decafStmtList*)$2;
	   }
           
           slist->push_front($1);
           $$ = slist;
         }
         ;
statement: block
         { $$ = $1; }
         | assign T_SEMICOLON
         { $$ = $1; }
         | method_call T_SEMICOLON
         { $$ = $1; }
/*        
         | if_stmt T_SEMICOLON
         | while_stmt T_SEMICOLON
         | for_stmt T_SEMICOLON
         | return_stmt T_SEMICOLON
         | break_stmt T_SEMICOLON
         | continue_stmt T_SEMICOLON
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

*/
;
assign: value T_ASSIGN expr  
  { 
    AssignAST* e = new AssignAST((ValueAST*)$1,$3);
    $$ = e;
  }  
  ;
expr: value
    {
      $$ = $1; // ValueAST*
    } 
    | method_call
    {
      $$ = $1; // MethodCallAST*
    }
    | constant  
    {
      $$ = $1; // ConstantAST*
    }
    | expr binary_op expr
    {
      BinaryExprAST* e = new BinaryExprAST(*$2, (decafStmtList*)$1, (decafStmtList*)$3);
      $$ = e;
      delete $2;
    } 
    //| unary_op expr
    | T_LPAREN expr T_RPAREN  
    { $$ = $2; }
    ;
value: T_ID 
     { 
       ValueAST* e = new ValueAST(*$1);
       delete $1;
       $$ = e;
     }   
     | T_ID T_LSB expr T_RSB
     {         
       ValueAST* e = new ValueAST(*$1, (decafStmtList*) $3);
       delete $1;
       $$ = e;
     }
method_call: T_ID T_LPAREN method_arg_list T_RPAREN
           {
             MethodCallAST* e;
             $$ = new MethodCallAST(*$1, $3);
             delete $1;
           }  
           ;
method_arg_list: T_STRINGCONSTANT
               {
                 $$ = new ConstantAST("StringType",*$1);
                 delete $1;
               }
               | expr
               { $$ = $1; }
               ;
binary_op: arithmatic_op
	   // | boolean_op
  ;
arithmatic_op: T_PLUS
{ $$ = new string("Plus");} 
             | T_MINUS
{ $$ = new string("Minus");}
             | T_MULT 
{ $$ = new string("Mult");}
             | T_DIV
{ $$ = new string("Div");}
             | T_LEFTSHIFT
{ $$ = new string("LeftShift");}
             | T_RIGHTSHIFT
{ $$ = new string("Rightshift");}
             | T_MOD
{ $$ = new string("Mod");}
  ;

boolean_op: T_EQ
{ $$ = new string("Eq");}
          | T_NEQ
{ $$ = new string("Neq");}
          | T_LT
{ $$ = new string("Lt");}
          | T_LEQ
{ $$ = new string("Leq");}
          | T_RT
{ $$ = new string("Gt");}
          | T_GEQ
{ $$ = new string("Geq");}
          | T_AND
{ $$ = new string("And");}
          | T_OR
{ $$ = new string("Or");}
  ;

/*
unary_op: T_UNARYMINUS
        | T_NOT  
  ;
*/

constant: T_INTCONSTANT 
        {
          $$ = new ConstantAST(string("IntTyoe"), *$1);
          delete $1;
        } 
        | T_CHARCONSTANT
        { 
          $$ = new ConstantAST(string("IntType"), char_to_ascii_string(*$1));
          delete $1;
	}
        | bool_constant
        {
          $$ = new ConstantAST(string("BoolType"), *$1);
	  delete $1;
        }
        ;

bool_constant: T_TRUE 
             { $$ = new string("True"); }
             | T_FALSE
             { $$ = new string("False"); }
             ;
decaf_type: T_INTTYPE
{  
  $$ = new string("IntType");
 }     
          | T_BOOLTYPE
{
  $$ = new string("BoolType"); 
}
  ;
array_type: T_LSB T_INTCONSTANT T_RSB decaf_type
{
  array_info info;
  info.type = $4;
  info.size = $2;
  
  //cout<<"array_type address"<< &info<<endl;
  $$ = info;
}
;
method_type: T_VOID
           { $$ = new string("VoidType");}   
           | decaf_type
           { $$ = $1; }
           ;
extern_type: T_STRINGTYPE 
           { $$ = new string("StringType");}
           | decaf_type
           { $$ = $1;}
           ;
%%  

int main()
{
  // parse the input and create the abstract syntax tree
  int retval = yyparse();
  return(retval >= 1 ? EXIT_FAILURE : EXIT_SUCCESS);
}
