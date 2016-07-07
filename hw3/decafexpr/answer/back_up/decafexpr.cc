
#include "decafexpr-defs.h"
#include <list>
#include <ostream>
#include <iostream>
#include <sstream>


#ifndef YYTOKENTYPE
#include <deque>
#include "decafexpr.tab.h"
#endif

using namespace std;

// empty list of symbol tables 
symbol_table_list symtbl;


descriptor* access_symtbl(string id)
{
  for(symbol_table_list::iterator i = symtbl.begin(); i != symtbl.end(); ++i)
  {
    symbol_table::iterator find_id;
    if((find_id = i->find(id)) != i->end())
    {
      return find_id->second; // second refers to descriptor* in map<string, descriptor*> 
    }
  } 
  return NULL;
}


descriptor* access_current_scope(string id)
{
  symbol_table i = symtbl.front();
  symbol_table::iterator find_id;
  if((find_id = i.find(id)) != i.end())
  {
    return find_id->second; // second refers to descriptor* in map<string, descriptor*> 
  }
  return NULL;
}

void scope_check(string id)
{
  if(access_current_scope(id) != NULL)
  {
    //cerr<<"Warning: redefining previously defined identifier: "<< id <<endl;
  }
}

void print_descriptor(string id)
{
  descriptor* d = access_symtbl(id);
  cerr<<"defined variable: "<< id
      <<", with type: "     << d->type
      <<", on line number: "<< d->lineno <<endl;
} 

void free_descriptors(symbol_table table)
{
  for(symbol_table::iterator i = table.begin(); i != table.end(); ++i)
  {
    delete(i->second);
  } 
} 


/// decafAST - Base class for all abstract syntax tree nodes.
class decafAST 
{
public:
  virtual ~decafAST() {}
  virtual string str() { return string(""); }
  virtual llvm::Value *Codegen() = 0;
};

string char_to_ascii_string(string str)
{
  if(str.empty())
  {
    return str;
  }
  
  // str[0] the single quote character '
  // str[1] could be the character or backslash
  
  stringstream ss;
  int ascii = 0;

  // if the character is not an escaped character 
  // then str[1] is the character 
  if(str[1] != '\\')
  {
    //cout<<"str[1] "<<str[1]<<endl;
    ascii = int(str[1]);
  } 
  // if the character is an escaped character with backslash
  // then str[2] is the character
  else
  {
    // cout<<"str[2] "<<str[2]<<endl;
    switch(str[2])
    {
     
    case 'a': ascii = 7; break;
    case 'b': ascii = 8; break;
    case 't': ascii = 9; break;
    case 'n': ascii = 10; break;
    case 'v': ascii = 11; break;
    case 'f': ascii = 12; break;
    case 'r': ascii = 13; break;
    case '\\': ascii = 92; break;
    case '\'': ascii = 39; break;
    case '\"': ascii = 34; break;
    }
  }
  //cout<<"ascii: "<<ascii<<endl;
  ss << ascii;
  //cout<<"ss.str(): "<<ss.str()<<endl;
  return string(ss.str());
}

string getString(decafAST *d) 
{
  if (d != NULL) 
  {
    return d->str();
  } 
  else 
  {
    return string("None");
  }
}

llvm::Type* getType(string type)
{
  llvm::Type* LType;
  if(type == "IntType")         { LType = Builder.getInt32Ty();  } // 32 bit int
  else if(type == "BoolType")   { LType = Builder.getInt1Ty();   } // 1 bit int  
  else if(type == "VoidType")   { LType = Builder.getVoidTy();   } // void 
  else if(type == "StringType") { LType = Builder.getInt8PtrTy();} // ptr to array of bytes
}

template <class T>
string commaList(list<T> vec) 
{
  string s("");
  for (typename list<T>::iterator i = vec.begin(); i != vec.end(); i++)
  { 
    s = s + (s.empty() ? string("") : string(",")) + (*i)->str(); 
  }   
    
  if (s.empty()) 
  {
    s = string("None");
  }
   
  return s;
}


template <class T>
llvm::Value *listCodegen(list<T> vec)
{
  llvm::Value *val = NULL;
  for (typename list<T>::iterator i = vec.begin(); i != vec.end(); i++)
  { 
    llvm::Value *j = (*i)->Codegen();
    if (j != NULL) { val = j; }
  }	
  return val;
}


/// decafStmtList - List of Decaf statements
class decafStmtList : public decafAST {
  list<decafAST *> stmts;
public:
  decafStmtList() {}
  ~decafStmtList()
  {
    //cout<<"releasing decafStmtList"<<endl;
    for (list<decafAST *>::iterator i = stmts.begin(); i != stmts.end(); i++)
    { 
      //cout<<"About to free: "<<(*i)->str()<<endl;
      delete *i;
    }
  }
  int size() { return stmts.size(); }
  void push_front(decafAST *e) { stmts.push_front(e); }
  void push_back(decafAST *e)  { stmts.push_back(e);  }


  decafAST* pop_front()       
  { 
    decafAST* e = stmts.front();
    stmts.pop_front(); 
    return e;
  }


  decafAST* pop_back()       
  { 
    decafAST* e = stmts.back();
    stmts.pop_back(); 
    return e;
  }

  list<decafAST*> return_list()
  {
    return stmts;
  }
  string str() { return commaList<class decafAST *>(stmts); }
  llvm::Value *Codegen() { return listCodegen<decafAST *>(stmts); }
};

class VarDefAST : public decafAST
{
  string Name;
  string VarType;

public:
  
  VarDefAST(string name, string type) : Name(name), VarType(type)
  {
    scope_check(name);
    descriptor* d = new descriptor;
    d->type = type;
    d->lineno = lineno;
    (symtbl.front())[name] = d;
  }
  ~VarDefAST(){};
  
  string str()
  {
    if(VarType.empty())
    {
      return string(""); // no argument       
    }
    else if(Name.empty())
    {
      return string("VarDef") + "(" + VarType + ")";
    }
    else
    {
      return string("VarDef") + "(" + Name + "," + VarType + ")";
    }
  }
  
  string getVarType()
  {
    return VarType;
  }
  
  llvm::Value *Codegen()
  {
    llvm::Value *Val; 
    llvm::Type  *LType = getType(VarType);
    llvm::AllocaInst *Alloca = Builder.CreateAlloca(LType, nullptr, Name);
    descriptor* d = access_current_scope(Name); 
    d->alloca_ptr = Alloca;
    return (llvm::Value*)Alloca;
  }  
};

class ConstantAST : public decafAST
{
  string Type;
  string Value;

public:
  ConstantAST(string type, string value) : Type(type), Value(value){} 
  
  string str()
  {
    string Name;
    if(Type == string("IntType"))
    {
      Name = string("NumberExpr");
    }
    else if(Type == string("StringType"))
    {
      Name = string("StringConstant");
    }
    else if(Type == string("BoolType"))
    {
      Name = string("BoolExpr");
    }
    
    return Name + "(" + Value + ")";
  }

  llvm::Value *Codegen()
  {
    llvm::Constant *Const;

    if(Type == "IntType")
    { 
      Const = Builder.getInt32(atoi(Value.c_str()));
    }
    else if(Type == "BoolType")
    { 
      if(Value == "True" ) { Const = Builder.getInt1(1);}
      if(Value == "False") { Const = Builder.getInt1(0);}
    }
    return (llvm::Value*)Const;
  }
};


class ExternAST : public decafAST
{
  string Name;
  decafStmtList* ExternTypeList;
  string MethodType; 

public: 
  ExternAST(string name, decafStmtList* elist, string mtype) 
  : Name(name), ExternTypeList(elist), MethodType(mtype){}  
  ~ExternAST()  
  {
     if(ExternTypeList != NULL) { delete ExternTypeList; }
  }
  string str()
  {
    return string("ExternFunction") + "(" + Name + "," + MethodType + "," + getString(ExternTypeList)+ ")";
  }

  llvm::Value *Codegen() 
  {
    llvm::Value *Val = NULL;
    TheModule->setModuleIdentifier(llvm::StringRef(Name)); 
    if (NULL != ExternTypeList) 
    {
      llvm::Type* returnTy = getType(MethodType);
      std::vector<llvm::Type*> args;
      list<decafAST*> stmts = ExternTypeList->return_list();  
      for (list<decafAST*>::iterator i = stmts.begin(); i != stmts.end(); i++)
      { 
        llvm::Type* ArgType = getType(((VarDefAST*)(*i))->getVarType()); 
        args.push_back(ArgType);    
      }                     
      llvm::Function *func = llvm::Function::Create(llvm::FunctionType::get(returnTy,args,false),
                                                    llvm::Function::ExternalLinkage,
                                                    Name,
                                                    TheModule
    	 					   );
      Val = (llvm::Value*)func;
    }
    return Val; 
  }
  
};

class PackageAST : public decafAST 
{
  string Name;
  decafStmtList *FieldDeclList;
  decafStmtList *MethodDeclList;
public:
  PackageAST(string name, decafStmtList *fieldlist, decafStmtList *methodlist) 
	    : Name(name), FieldDeclList(fieldlist), MethodDeclList(methodlist) {}
  ~PackageAST() 
  { 
    if (FieldDeclList  != NULL) { delete FieldDeclList;  }
    if (MethodDeclList != NULL) { delete MethodDeclList; }
  }
  string str() 
  { 
    return string("Package") + "(" + Name + "," + getString(FieldDeclList) + "," + getString(MethodDeclList) + ")";
  }

  llvm::Value *Codegen() 
  { 
    llvm::Value *val = NULL;
    TheModule->setModuleIdentifier(llvm::StringRef(Name)); 
    if (NULL != FieldDeclList) 
    {
      val = FieldDeclList->Codegen();
    }
    if (NULL != MethodDeclList) 
    {
      val = MethodDeclList->Codegen();
    } 
    // Q: should we enter the class name into the symbol table?
    return val; 
  }
};

// ProgramAST - the decaf program
class ProgramAST : public decafAST 
{
  decafStmtList *ExternList;
  PackageAST *PackageDef;
public:
  ProgramAST(decafStmtList *externs, PackageAST *c) : ExternList(externs), PackageDef(c) {}
  ~ProgramAST() 
  { 
    if (ExternList != NULL)  { delete ExternList; } 
    if (PackageDef != NULL)  { delete PackageDef; }
  }
  string str() { return string("Program") + "(" + getString(ExternList) + "," + getString(PackageDef) + ")"; }

  llvm::Value *Codegen() 
  { 
    llvm::Value *val = NULL;
    if (NULL != ExternList)
    {
      val = ExternList->Codegen();
    }
    if (NULL != PackageDef) 
    {
      val = PackageDef->Codegen();
    } 
    else 
    {
       throw runtime_error("no package definition in decaf program");
    }
    return val; 
  }
};


class FieldAST : public decafAST
{
  string Name;
  string FieldType;
  string FieldSize;
  decafAST* Expr;
  bool Assignment;
public:
  
  FieldAST(string name, string type, string size, bool isAssign) 
    : Name(name), FieldType(type), FieldSize(size), Assignment(isAssign) {}
  
  FieldAST(string name, string type, decafAST* argument, bool isAssign) 
    : Name(name), FieldType(type), Expr(argument), Assignment(isAssign) {}

  ~FieldAST()
  {
    if(Expr != NULL) { delete Expr; }
  }
 
  string str()
  {
    if(Assignment == true)
    {
      return string("AssignGlobalVar") + "(" + Name + "," + FieldType + "," + getString(Expr) +")"; 
    } 
    else
    {
      return string("FieldDecl") + "(" + Name + "," + FieldType + "," + FieldSize + ")";
    }
  }

  llvm::Value *Codegen() 
  {   
    
    llvm::Constant* Initializer;
    llvm::Type* GVType = getType(FieldType); 
    
    if(Assignment)
    {
      Initializer = (llvm::Constant*)Expr->Codegen();
    } 
    else
    {
      if(GVType->isIntegerTy(32))     { Initializer = Builder.getInt32(0); }
      else if(GVType->isIntegerTy(1)) { Initializer = Builder.getInt1(0) ; }
      else if(GVType->isVoidTy())     { Initializer = NULL; }
    }
    llvm::GlobalVariable *GV = new llvm::GlobalVariable(*TheModule,
                                                        GVType,
                                                        false,
                                                        llvm::GlobalValue::InternalLinkage,
                                                        Initializer,
                                                        Name
						        )
                                                        ;
   
    scope_check(Name);
    descriptor* d = new descriptor;
    d->type  = FieldType;
    d->lineno = lineno;
    d->gv_ptr = GV;
    (symtbl.front())[Name] = d;
    
  }
};


class BlockAST : public decafAST
{
  decafStmtList* VarDeclList;
  decafStmtList*  StmtList;
  bool MethodBlock;
   
public:
  BlockAST(decafStmtList* varlist, decafStmtList* stmtlist)
         : VarDeclList(varlist), StmtList(stmtlist),MethodBlock(false){}
  ~BlockAST()
  {
    if(VarDeclList != NULL) { delete VarDeclList; }
    if(StmtList != NULL)    { delete StmtList;    }
  }
    
  void setMethodBlock(bool flag)
  {
    MethodBlock = flag;
  }
  
  string str()
  {
    string Name;
    if(MethodBlock == true)
    {
      Name = string("MethodBlock");
    }
    else
    {
      Name = string("Block");
    } 
    return Name + "(" + getString(VarDeclList) + "," + getString(StmtList) + ")";
  }

  llvm::Value *Codegen() 
  {
    llvm::Value* Val = NULL;
    if(VarDeclList != NULL) { Val = VarDeclList->Codegen(); }
    if(StmtList    != NULL) { Val = StmtList->Codegen();    } 
  }
};

class MethodAST : public decafAST
{
  string Name;
  string MethodType;
  decafStmtList *ArgList;
  BlockAST *Block;
  
public:
  MethodAST(string name, string type, decafStmtList* alist, BlockAST* block) 
    : Name(name), MethodType(type), ArgList(alist), Block(block){}
  ~MethodAST()
  {
    if(ArgList != NULL) { delete ArgList;}
    if(Block != NULL)   { delete Block;  }
  }

  string str()
  {
    return string("Method") 
                  + "(" 
                  + Name + "," + MethodType + "," + getString(ArgList) + "," + getString(Block)
                  + ")";
  }

  llvm::Value *Codegen() 
  {
    llvm::Value *Val = NULL;
    llvm::Type *returnTy = getType(MethodType);
    
    if(ArgList != NULL) { Val = ArgList->Codegen();}
    
    // fill in the vector with parameter types 
    vector<llvm::Type *> args;
    list<decafAST*> stmts = ArgList->return_list();  
    for (list<decafAST*>::iterator i = stmts.begin(); i != stmts.end(); i++)
    { 
      llvm::Type* ArgType = getType(((VarDefAST*)(*i))->getVarType()); 
      args.push_back(ArgType);    
    }                     

    llvm::Function *func = llvm::Function::Create(llvm::FunctionType::get(returnTy, args, false),
                                                  llvm::Function::ExternalLinkage,
                                                  Name,
                                                  TheModule
    						  );
    
    // create a new basic block which contains a sequence of LLVM instructions 
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", func);
    
    // insert "entry" into symbol table (will be used in hw4)
         
    // all subsequent calls to IRBuilder wlil place instructions in this location 
    Builder.SetInsertPoint(BB);
    
    if(Block != NULL) { Val = Block->Codegen(); }
    
    scope_check(Name);
    descriptor* d = new descriptor;
    d->type       = MethodType;
    d->lineno     = lineno;
    d->func_ptr   = func;
    (symtbl.front())[Name] = d;    
  }
};

class MethodCallAST : public decafAST
{
  string Name;
  decafAST *ArgList;

public: 
  MethodCallAST(string name, decafAST *alist) : Name(name), ArgList(alist){}  
  ~MethodCallAST()
  {
    if(ArgList != NULL) { delete ArgList; }
  }

  string str()
  {
    return string("MethodCall") + "(" + Name + "," + getString(ArgList) +")"; 
  }

  llvm::Value *Codegen() 
  {
    llvm::Value* Val = NULL;
    llvm::Function *call;  
    vector<llvm::Value*> args;
    descriptor* d = access_symtbl(Name);
    if( d != NULL) 
    {
      
      call = d->func_ptr; 
      bool isVoid = call->getReturnType()->isVoidTy();
      Val = Builder.CreateCall(call, args, isVoid ? "" : "calltmp");
      
    }
    return Val;
};

class ValueAST : public decafAST
{
  string Name;
  decafStmtList* IndexExpr;
  bool ArrayFlag;

public: 
  ValueAST(string name) : Name(name), ArrayFlag(false), IndexExpr(NULL) {}
  ValueAST(string name, decafStmtList* index) : Name(name), IndexExpr(index), ArrayFlag(true){}
  ~ValueAST()
  {
    if(IndexExpr != NULL) { delete IndexExpr; }
  }
   
  string getID() { return Name; }  
  decafStmtList* getIndexExpr() { return IndexExpr; }
  bool isArray() { return ArrayFlag; }	
	   
  string str()
  {
    if(ArrayFlag == false)
    {
      return string("VariableExpr") + "(" + Name + ")";
    } 
    else
    {
      return string("ArrayLocExpr") + "(" + Name + "," + getString(IndexExpr) +")";
    }  
  }

  llvm::Value *Codegen() 
  { 
    // TODO:  1. handle error when d is null 
    //        2. handle error when it is array_type (not needed for HW3)
    descriptor* d = access_current_scope(Name);
    llvm::Value *Val = (llvm::Value*) d->alloca_ptr;         
    return Val;
  }
};  

class AssignAST : public decafAST
{
  ValueAST* Value;
  decafAST* Expr;
   
public: 
  AssignAST(ValueAST* value, decafAST* expr) : Value(value), Expr(expr){}
  ~AssignAST()
  {
    if(Value != NULL) { delete Value; }
    if(Expr  != NULL) { delete Expr;  }
  }
  
  string str()
  {
    string Name;
    if(!(Value->isArray()))
    {
      Name = string("AssignVar");
      return Name + "(" + Value->getID() + "," + getString(Expr) + ")";
    }
    else
    {
      Name = string("AssignArrayLoc");
      return Name + "(" + Value->getID() + "," + getString(Value->getIndexExpr()) + ","+ getString(Expr) + ")";
    } 
  }

  llvm::Value *Codegen() 
  {
    llvm::Value *Val; 
    llvm::AllocaInst *Alloca = (llvm::AllocaInst*)Value->Codegen();
    llvm::Value *RValue = Expr->Codegen();
    const llvm::PointerType *ptrTy = RValue->getType()->getPointerTo();
    if(Alloca->getType() == ptrTy)
    {
      Val = Builder.CreateStore(RValue, Alloca);
    }  
    return Val;
  }
};

class IfStmtAST : public decafAST
{
  decafAST* Condition;
  BlockAST* IfBlock;
  BlockAST* ElseBlock;

public: 
  IfStmtAST(decafAST* condition, BlockAST* if_block, BlockAST* else_block)
           : Condition(condition), IfBlock(if_block), ElseBlock(else_block){} 
  ~IfStmtAST()
  {
    if(Condition != NULL) { delete Condition; }
    if(IfBlock   != NULL) { delete IfBlock;   }
    if(ElseBlock != NULL) { delete ElseBlock; }
  }  

  string str()
  {
    return string("IfStmt") + "(" + getString(Condition) + "," + getString(IfBlock) + "," + getString(ElseBlock) + ")";
  }

  llvm::Value *Codegen() {}
};

class WhileStmt : public decafAST
{
  decafAST* Condition;  
  BlockAST* WhileBlock;

public:
  WhileStmt(decafAST* condition, BlockAST* while_block) : Condition(condition), WhileBlock(while_block){}
  ~WhileStmt()
  {
    if(Condition  != NULL) { delete Condition;  }
    if(WhileBlock != NULL) { delete WhileBlock; }
  }
  
  string str()
  { 
    return string("WhileStmt") + "(" + getString(Condition) + "," + getString(WhileBlock) + ")";
  }

  llvm::Value *Codegen() {}
};

class ForStmtAST : public decafAST
{
  AssignAST* PreAssign;
  decafAST*  Condition;
  AssignAST* LoopAssign;
  BlockAST*  ForBlock;
    
public:
  ForStmtAST(AssignAST* pre_assign, decafAST* condition, AssignAST* loop_assign, BlockAST* for_block)
           : PreAssign(pre_assign), Condition(condition), LoopAssign(loop_assign), ForBlock(for_block){}  
  ~ForStmtAST()
  {
    if(PreAssign  != NULL) { delete PreAssign;  }
    if(Condition  != NULL) { delete Condition;  }
    if(LoopAssign != NULL) { delete LoopAssign; }
    if(ForBlock   != NULL) { delete ForBlock;   }
  }

  string str()
  { 
    return string("ForStmt") + "(" + getString(PreAssign)  + "," 
                                   + getString(Condition)  + "," 
                                   + getString(LoopAssign) + "," 
                                   + getString(ForBlock)   +  ")";
  }  

  llvm::Value *Codegen() {} 
};

class ReturnStmtAST : public decafAST
{
  decafAST* Expr;
   
public: 
  ReturnStmtAST(decafAST* expr) : Expr(expr){}
  ~ReturnStmtAST()
  {
    if(Expr != NULL) { delete Expr;}
  }
  
  string str()
  {
    return string("ReturnStmt") + "(" + getString(Expr) +")";
  }

  llvm::Value *Codegen() 
  { 
    llvm::Value* Val;
    llvm::BasicBlock *CurBB = Builder.GetInsertBlock();
    llvm::Function *func    = CurBB->getParent();
    llvm::Type* returnTy    = func->getReturnType();
   
    if(Expr == NULL) 
    { 
      // default return for Int  is 0 
      // default return for Bool is true 
      if(returnTy->isIntegerTy(32))    { Val = Builder.CreateRet(Builder.getInt32(0)); }
      else if(returnTy->isIntegerTy(1)){ Val = Builder.CreateRet(Builder.getInt1(1)) ; } 
    }
    else
    { 
      Val = Expr->Codegen();
      Val = Builder.CreateRet(Val);
    }
    return Val;
  }
};

class BreakStmtAST : public decafAST
{
public: 

  string str()
  {
    return string("BreakStmt");
  }

  llvm::Value *Codegen() {}
};

class ContinueStmtAST : public decafAST
{  
public: 
  string str()
  {
    return string("ContinueStmt");
  }
  llvm::Value *Codegen() {}
};

class BinaryExprAST : public decafAST
{
  string BinaryOp;
  decafStmtList* LeftValue;
  decafStmtList* RightValue; 

public: 
  BinaryExprAST(string op, decafStmtList* left, decafStmtList* right) 
               : BinaryOp(op), LeftValue(left), RightValue(right){}
  ~BinaryExprAST()
   {
     if(LeftValue != NULL)  { delete LeftValue; }
     if(RightValue != NULL) { delete RightValue; }
   }

  string str()
  {
    return string("BinaryExpr") + "(" + BinaryOp + "," + getString(LeftValue) + "," + getString(RightValue) + ")";
  }
  llvm::Value *Codegen() {}
};

class UnaryExprAST : public decafAST
{
  string UnaryOp;
  decafStmtList* RightValue; 

public: 
  UnaryExprAST(string op,  decafStmtList* right) : UnaryOp(op), RightValue(right){}
  ~UnaryExprAST()
   {
     if(RightValue != NULL) { delete RightValue; }
   }

  string str()
  {
    return string("UnaryExpr") + "(" + UnaryOp + "," + getString(RightValue) + ")";
  }

  llvm::Value *Codegen() {}
};
