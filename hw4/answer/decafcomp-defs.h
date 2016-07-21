
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

typedef map<string, llvm::Value*> symbol_table;

typedef list<symbol_table> symbol_table_list;

extern int lineno;

extern int tokenpos;

extern symbol_table_list symtbl;

extern llvm::Value* access_symtbl(string id);

#endif

