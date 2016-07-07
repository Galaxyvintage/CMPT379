%{
#include <iostream>
#include <ostream>
#include <string>
#include <cstdlib>
#include "decafexpr-defs.h"

int yylex(void);
int yyerror(char *); 

// print AST?
bool printAST = false;

using namespace std;

// this global variable contains all the generated code
static llvm::Module *TheModule;

// this is the method used to construct the LLVM intermediate code (IR)
static llvm::IRBuilder<> Builder(llvm::getGlobalContext());
// the calls to getGlobalContext() in the init above and in the
// following code ensures that we are incrementally generating
// instructions in the right order

// dummy main function
// should be replaced with actual codegen for the main method 
// using the full Decaf grammar
static llvm::Function *TheFunction = 0;

// we have to create a main function 
llvm::Function *gen_main_def() 
{
  // create the top-level definition for main
  llvm::FunctionType *FT = llvm::FunctionType::get(llvm::IntegerType::get(llvm::getGlobalContext(), 32), false);
  llvm::Function *TheFunction = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "main", TheModule);
  if (TheFunction == 0)
  {
    throw runtime_error("empty function block"); 
  }
  // Create a new basic block which contains a sequence of LLVM instructions
  llvm::BasicBlock *BB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", TheFunction);
  // All subsequent calls to IRBuilder will place instructions in this location
  Builder.SetInsertPoint(BB);
  return TheFunction;
}

#include "decafexpr.cc"
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

%token <sval> T_EQ
%token <sval> T_LEQ
%token <sval> T_GEQ
%token <sval> T_NEQ
%token <sval> T_LEFTSHIFT
%token <sval> T_RIGHTSHIFT
%token <sval> T_AND
%token <sval> T_OR

%token <sval> T_PLUS
%token <sval> T_MINUS
%token <sval> T_MULT
%token <sval> T_DIV
%token <sval> T_NOT
%token T_ASSIGN
%token <sval> T_LT
%token <sval> T_RT
%token <sval> T_MOD
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

%left  T_OR
%left  T_AND
%left  T_GEQ T_LEQ T_LT T_RT T_EQ T_NEQ
%left  T_PLUS T_MINUS 
%left  T_MULT T_DIV T_MOD T_LEFTSHIFT T_RIGHTSHIFT
%left  T_UNOT
%right T_UMINUS 

%type <ast> extern_list extern_def extern_type_comma_list decafpackage
%type <ast> field_decls field_decl
%type <ast> method_decls method_decl param_list id_type_comma_list
%type <ast> var_decls var_decl
%type <ast> statements statement
%type <ast> block assign method_call if_stmt while_stmt for_stmt return_stmt break_stmt continue_stmt
%type <ast> expr value constant method_arg_list method_arg_comma_list method_arg assign_list optional_expr
%type <deque_ptr> id_comma_list 
%type <sval> extern_type decaf_type method_type bool_constant
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

         try 
         {
           prog->Codegen();
         } 
         catch (std::runtime_error &e)
         {
           cout << "semantic error: " << e.what() << endl;
           //cout << prog->str() << endl; 
           exit(EXIT_FAILURE);
         }
         delete prog;
       }
       ;

extern_list: /* extern_list can be empty */ { $$ = NULL; }
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
                      | /* Empty */
                      {
                        decafStmtList* slist = new decafStmtList();
                        VarDefAST* e = new VarDefAST(string(""), string(""));
                        slist->push_front(e);
                        $$ = slist;
		      }
                      ;

decafpackage: T_PACKAGE T_ID begin_block field_decls method_decls end_block
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
                e = new FieldAST((*$2)[x],*$3,"Scalar", false);
                slist->push_back(e);
              }    
  
              for(int x = $2->size() - 1; x >= 0; --x)
              {
		// print_descriptor((*$2)[x]);
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
                e = new FieldAST((*$2)[x],FieldType, FieldSize,false);
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
 
	      // assigning more than one variable in the same line is invalid
              if($2->size() > 1)  
              {
                return 1;
	      }              
             
              e = new FieldAST((*$2)[0],*$3,$5,true);
              slist->push_back(e);
	      //print_descriptor((*$2)[0]);
                   
              delete $2;
              delete $3;
         
              $$ = slist; 
            }
            ;

id_comma_list: T_ID T_COMMA id_comma_list 
             {
               deque<string>* ilist;
               ilist = $3;
               ilist->push_front(*$1);
               delete $1;
               $$ = ilist;
             }
             | T_ID
             {
               deque<string>* ilist;
               ilist = new deque<string>;
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
                    //print_descriptor(*$1); 
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

                    //print_descriptor(*$1);

                    delete $1; // free T_ID string 
                    delete $2; // free decaf_type string 
                    $$ = slist;
                  }  
                  ;

block: begin_block var_decls statements end_block
     {   
       BlockAST* e;
       e = new BlockAST((decafStmtList*)$2,(decafStmtList*)$3);
       $$ = e;
     }
  ;


begin_block : T_LCB
            { symtbl.push_front(symbol_table()); };
  
end_block   : T_RCB
            {
              symbol_table sym_table = symtbl.front();
              free_descriptors(sym_table);
              symtbl.pop_front();          
            };

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

          for(int x = $2->size() - 1; x >= 0; --x)
          {
            //print_descriptor((*$2)[x]);
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
         | if_stmt 
         { $$ = $1; }
         | while_stmt
         { $$ = $1; }
         | for_stmt 
         { $$ = $1; } 
         | return_stmt   T_SEMICOLON
         { $$ = $1; }
         | break_stmt    T_SEMICOLON  
         { $$ = $1; } 
         | continue_stmt T_SEMICOLON 
         { $$ = $1; }
         ; 

assign: value T_ASSIGN expr 
      { 
        AssignAST* e = new AssignAST((ValueAST*)$1,$3);
        $$ = e;
      }  
      ;
method_call: T_ID T_LPAREN method_arg_list T_RPAREN 
           {
             MethodCallAST* e;
             $$ = new MethodCallAST(*$1, $3);
             delete $1;
           }  
           ;
if_stmt: T_IF T_LPAREN expr T_RPAREN block 
       {
         $$ = new IfStmtAST($3,(BlockAST*)$5,NULL);  
       }  
       | T_IF T_LPAREN expr T_RPAREN block T_ELSE block
       {
         $$ = new IfStmtAST($3,(BlockAST*)$5,(BlockAST*)$7);
       }
       ;
while_stmt: T_WHILE T_LPAREN expr T_RPAREN block
          {
            $$ = new WhileStmt($3,(BlockAST*)$5);
          }   
          ;
for_stmt: T_FOR T_LPAREN assign_list T_SEMICOLON expr T_SEMICOLON assign_list T_RPAREN block
        {   
          $$ = new ForStmtAST((AssignAST*)$3,$5,(AssignAST*)$7, (BlockAST*)$9);
        }  
        ; 

return_stmt: T_RETURN T_LPAREN optional_expr T_RPAREN
           { $$ = new ReturnStmtAST($3);    }   
           | T_RETURN
           { $$ = new ReturnStmtAST(NULL);  }
;

break_stmt: T_BREAK
{ $$ =  new BreakStmtAST(); } ;

continue_stmt: T_CONTINUE
{ $$ = new ContinueStmtAST(); };

optional_expr: /* Empty */
             { $$ = NULL; } 
             | expr
             { $$ = $1;} 
             ;
assign_list: assign T_COMMA assign_list
           {
             decafStmtList* slist = (decafStmtList*)$3;
             slist->push_front($1);
             $$ = slist;
           } 
           | assign 
           {
             decafStmtList* slist = new decafStmtList();
             slist->push_front($1); 
             $$ = slist;
	   }
           ;
method_arg_list: /* Empty */ 
               { $$ = NULL;} 
               | method_arg_comma_list
               { $$ = $1; }
               ;
method_arg_comma_list: method_arg T_COMMA method_arg_comma_list
               { 
                 decafStmtList* slist = (decafStmtList*)$3;
                 slist->push_front($1);
                 $$ = slist;
                }
               | method_arg
	       {
                 decafStmtList* slist = new decafStmtList();
                 slist->push_front($1);
                 $$ = slist;;
	       }
               ;
method_arg: T_STRINGCONSTANT
          {
            $$ = new ConstantAST("StringType",*$1);
            descriptor* d = access_symtbl(*$1);
            if(d != NULL)
              //cerr<<"using decl on line number: "<< d->lineno <<endl;
            
            delete $1;
          }
          | expr
          { $$ = $1; }   
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
    | expr T_PLUS expr
    {
      BinaryExprAST* e = new BinaryExprAST(*$2, (decafStmtList*)$1, (decafStmtList*)$3);
      $$ = e;
      delete $2;
    }
    | expr T_MINUS expr
    {
      BinaryExprAST* e = new BinaryExprAST(*$2, (decafStmtList*)$1, (decafStmtList*)$3);
      $$ = e;
      delete $2;
    }
    | expr T_MULT expr
    {
      BinaryExprAST* e = new BinaryExprAST(*$2, (decafStmtList*)$1, (decafStmtList*)$3);
      $$ = e;
      delete $2;
    }
    | expr T_DIV expr
    {
      BinaryExprAST* e = new BinaryExprAST(*$2, (decafStmtList*)$1, (decafStmtList*)$3);
      $$ = e;
      delete $2;
    }
    | expr T_LEFTSHIFT expr
    {
      BinaryExprAST* e = new BinaryExprAST(*$2, (decafStmtList*)$1, (decafStmtList*)$3);
      $$ = e;
      delete $2;
    }
    | expr T_RIGHTSHIFT expr
    {
      BinaryExprAST* e = new BinaryExprAST(*$2, (decafStmtList*)$1, (decafStmtList*)$3);
      $$ = e;
      delete $2;
    }
    | expr T_MOD expr
    {
      BinaryExprAST* e = new BinaryExprAST(*$2, (decafStmtList*)$1, (decafStmtList*)$3);
      $$ = e;
      delete $2;
    }
    | expr T_EQ expr
    {
      BinaryExprAST* e = new BinaryExprAST(*$2, (decafStmtList*)$1, (decafStmtList*)$3);
      $$ = e;
      delete $2;
    }
    | expr T_NEQ expr
    {
      BinaryExprAST* e = new BinaryExprAST(*$2, (decafStmtList*)$1, (decafStmtList*)$3);
      $$ = e;
      delete $2;
    }
    | expr T_LT expr
    {
      BinaryExprAST* e = new BinaryExprAST(*$2, (decafStmtList*)$1, (decafStmtList*)$3);
      $$ = e;
      delete $2;
    }
    | expr T_LEQ expr
    {
      BinaryExprAST* e = new BinaryExprAST(*$2, (decafStmtList*)$1, (decafStmtList*)$3);
      $$ = e;
      delete $2;
    }
    | expr T_RT expr
    {
      BinaryExprAST* e = new BinaryExprAST(*$2, (decafStmtList*)$1, (decafStmtList*)$3);
      $$ = e;
      delete $2;
    }
    | expr T_GEQ expr
    {
      BinaryExprAST* e = new BinaryExprAST(*$2, (decafStmtList*)$1, (decafStmtList*)$3);
      $$ = e;
      delete $2;
    }
    | expr T_AND expr
    {
      BinaryExprAST* e = new BinaryExprAST(*$2, (decafStmtList*)$1, (decafStmtList*)$3);
      $$ = e;
      delete $2;
    }
    | expr T_OR expr
    {
      BinaryExprAST* e = new BinaryExprAST(*$2, (decafStmtList*)$1, (decafStmtList*)$3);
      $$ = e;
      delete $2;
    }   
    | T_LPAREN expr T_RPAREN  
    { $$ = $2; }
    | T_MINUS expr %prec T_UMINUS
    { 
      UnaryExprAST* e = new UnaryExprAST(string("UnaryMinus"), (decafStmtList*)$2);
      $$ = e;
      delete $1;
    }
    | T_NOT expr %prec T_UNOT
    { 
      UnaryExprAST* e = new UnaryExprAST(*$1, (decafStmtList*)$2);
      $$ = e;
      delete $1;
    }
    ;
value: T_ID T_LSB expr T_RSB
     {        
       descriptor* d = access_symtbl(*$1);
       if(d != NULL)
         cerr<<"using decl on line number: "<< d->lineno <<endl; 
       ValueAST* e = new ValueAST(*$1, (decafStmtList*) $3);
       delete $1;
       $$ = e;
     }
     | T_ID 
     { 
       descriptor* d = access_symtbl(*$1);
       if(d != NULL)
         cerr<<"using decl on line number: "<< d->lineno <<endl;
       ValueAST* e = new ValueAST(*$1);
       delete $1;
       $$ = e;
     }   
     ;

constant: T_INTCONSTANT 
        {
          $$ = new ConstantAST(string("IntType"), *$1);
          delete $1;
        } 
        | T_CHARCONSTANT
        { 
          descriptor* d = access_symtbl(*$1);
          if(d != NULL)
            //cerr<<"using decl on line number: "<< d->lineno <<endl;
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
            $$ = new string("int");
          }     
          | T_BOOLTYPE
          {
            $$ = new string("bool"); 
          }
          ;
array_type: T_LSB T_INTCONSTANT T_RSB decaf_type
{
  array_info info;
  info.type = $4;
  info.size = $2;
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

/* 
   TODO: Need a way to keep track of all the pointers and free them 
         when the parser encounters a syntax error    
*/
int main()
{
  // initialize LLVM
  llvm::LLVMContext &Context = llvm::getGlobalContext();

  // make the module, which holds all the code.
  TheModule = new llvm::Module("Test", Context);

  // set up symbol table
  symtbl.push_front(symbol_table());

  // set up dummy main function
  TheFunction = gen_main_def();

  // parse the input and create the abstract syntax tree
  int retval = yyparse();

  // free the extern scope symtol table
  symbol_table sym_table = symtbl.front();
  free_descriptors(sym_table);
  symtbl.pop_front();    

  // finish off the main function. (see the WARNING above)

  // return 0 from main, which is EXIT_SUCCESS
  Builder.CreateRet(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32, 0)));

  // Validate the generated code, checking for consistency.
  verifyFunction(*TheFunction);
  
  // Print out all of the generated code to stderr
  TheModule->dump();
  
  return(retval >= 1 ? EXIT_FAILURE : EXIT_SUCCESS);
}
