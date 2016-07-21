
#ifndef _DECAF_DEFS
#define _DECAF_DEFS

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/IRBuilder.h"
#include <cstdio> 
#include <cstdlib>
#include <cstring> 
#include <string>
#include <stdexcept>
#include <vector>
#include <list>
#include <deque>

extern int lineno;
extern int tokenpos;

using namespace std;

extern "C"
{
	extern int yyerror(const char *);
	int yyparse(void);
	int yylex(void);  
	int yywrap(void);
}

typedef struct 
{ 
  std::string* type;
  std::string* size;
}array_info;

typedef struct descriptor 
{ 
  int lineno;
  string type;
  llvm::GlobalVariable* gv_ptr;
  llvm::ArrayType*      array_type;
  llvm::Value*          array_index;
  llvm::AllocaInst*     alloca_ptr;
  llvm::Function*       func_ptr;
  vector<llvm::Type*>   arg_types;
  vector<string>        arg_names;
}descriptor; 

typedef map<string, descriptor*> symbol_table;

typedef list<symbol_table> symbol_table_list;

extern int lineno;

extern int tokenpos;

extern symbol_table_list symtbl;

extern descriptor* access_symtbl(string id);

extern void print_descriptor(string id);


#endif

