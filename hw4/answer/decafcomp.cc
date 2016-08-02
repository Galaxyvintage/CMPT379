 
#include "decafcomp-defs.h"
#include <ostream>
#include <iostream>
#include <sstream>
#include <string>
#include <list>
#include <map>

#ifndef YYTOKENTYPE
#include "decafcomp.tab.h"
#endif

using namespace std;

// empty list of symbol tables
symbol_table_list symtbl;

// debug_flag
bool debug_flag = false;

// default return value
llvm::Value* returnValue;

void debug_print(bool flag, string output)
{
  if(flag == true) { cout<<output<<endl;}
}

llvm::Value* access_symtbl(string id)
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

llvm::Value* access_current_scope(string id)
{
  symbol_table i = symtbl.front();
  symbol_table::iterator find_id;
  if((find_id = i.find(id)) != i.end())
  {
    return find_id->second; // second refers to descriptor* in map<string, descriptor*> 
  }
  return NULL;
}

void free_element(symbol_table table)
{
  for(symbol_table::iterator i = table.begin(); i != table.end(); ++i)
  {
    delete(i->second);
  } 
} 

int string_to_int(string str)
{
  int result;
  stringstream ss;
  if(str.find("x") != string::npos)
  {
    ss<<hex<<str;
  }
  else
  {
    ss<<str;  
  }
  ss>>result;
  return result;
}

llvm::Type* getType(string type)
{
  llvm::Type* LType;
  if(type == "IntType")         
  { LType = Builder.getInt32Ty();  } // 32 bit int
  else if(type == "BoolType")  
  { LType = Builder.getInt1Ty();   } // 1 bit int  
  else if(type == "VoidType")  
  { LType = Builder.getVoidTy();   } // void 
  else if(type == "StringType") 
  { LType = Builder.getInt8PtrTy();} // ptr to array of bytes
  return LType;
}

int getOperator(string op)
{
  if(op == "Plus")       { return T_PLUS;  }
  if(op == "Minus")      { return T_MINUS; }
  if(op == "Mult")       { return T_MULT;  }
  if(op == "Div")        { return T_DIV;   }
  if(op == "Leftshift")  { return T_LEFTSHIFT;   }
  if(op == "Rightshift") { return T_RIGHTSHIFT;  }
  if(op == "Mod")        { return T_MOD;   } 
  if(op == "Eq")         { return T_EQ;    }
  if(op == "Neq")        { return T_NEQ ;  }
  if(op == "Lt")         { return T_LT;    }
  if(op == "Leq")        { return T_LEQ;   }
  if(op == "Gt")         { return T_GT;    }
  if(op == "Geq")        { return T_GEQ;   }
  if(op == "And")        { return T_AND;   }
  if(op == "Or")         { return T_OR;    }
  if(op == "Not")        { return T_NOT;   } 
  if(op == "UnaryMinus") { return T_MINUS; }
  return -1;
}

static llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Function *TheFunction, llvm::Type* VarType, const std::string &VarName)
{
  llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
  return TmpB.CreateAlloca(VarType, NULL, VarName.c_str());
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

/// decafAST - Base class for all abstract syntax tree nodes.
class decafAST 
{
public:
  virtual ~decafAST() {}
  virtual string str()  { return string(""); }
  virtual string str_2(){ return string(""); }
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

string yysval_to_string(string yy_str)
{
  if(yy_str.empty())
  {
    return yy_str;
  }
  
  // yy_str[0] the single quote character '
  // yy_str[1...n-2] could be the character or backslash
  // yy_str[n-1] the single quote character 
  // if the string is empty, yy_str would ""(including the quotation marks)

  string result;
  
  for(int x = 1; x < yy_str.length()-1; ++x)
  { 
    // if the character is not an escaped character 
    // then str[1] is the character 
    if(yy_str[x] != '\\')
    {
      result.push_back(yy_str[x]);
    } 
    // if the character is an escaped character with backslash
    // then str[2] is the character
    else
    {
      // cout<<"str[x] "<<str[x]<<endl;
      switch(yy_str[x+1])
      { 
      case 'a':  result.push_back('\a'); break;
      case 'b':  result.push_back('\b'); break;
      case 't':  result.push_back('\t'); break;
      case 'n':  result.push_back('\n'); break;
      case 'v':  result.push_back('\v'); break;
      case 'f':  result.push_back('\f'); break;
      case 'r':  result.push_back('\r'); break;
      case '\\': result.push_back('\\'); break;
      case '\'': result.push_back('\''); break;
      case '\"': result.push_back('\"'); break;
      }
      x++;
    }
  }

  return result;
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


/// decafStmtList - List of Decaf statements
class decafStmtList : public decafAST {
  list<decafAST *> stmts;
public:
  decafStmtList() {}
  ~decafStmtList()
  {
    for (list<decafAST *>::iterator i = stmts.begin(); i != stmts.end(); i++)
    { 
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
  string str()    { return commaList<class decafAST *>(stmts); }
  llvm::Value *Codegen() { return listCodegen<decafAST *>(stmts); }
};

class VarDefAST : public decafAST
{
  string Name;
  string VarType;
  bool   isParam;
public:
  
  VarDefAST(string name, string type, bool param_flag) : Name(name), VarType(type), isParam(param_flag)
  {
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

  string getName()
  {
    return Name;
  }

  llvm::Value *Codegen() 
  {
    debug_print(debug_flag,"...VarDef Codegen Begins...");

    if(Name.empty()) { return NULL; }

    llvm::Type  *LType = getType(VarType);
    llvm::AllocaInst *Alloca = NULL;

    if(isParam == false)
    { 
      Alloca = Builder.CreateAlloca(LType, NULL, Name);
      (symtbl.front())[Name] = Alloca;
    } 

    debug_print(debug_flag,"...VarDef Codegen Ends...");
    return (llvm::Value*)Alloca;
  }
};

class ConstantAST : public decafAST
{
  string Type;
  string Value;

public:
  ConstantAST(string type, string value) : Type(type), Value(value)
  {
   
  } 
  
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
      Const = Builder.getInt32(string_to_int(Value));
    }
    else if(Type == "BoolType")
    { 
      if(Value == "True" ) { Const = Builder.getInt1(1);}
      if(Value == "False") { Const = Builder.getInt1(0);}
    }
    else if(Type == "StringType")
    {
      llvm::GlobalVariable *GS = Builder.CreateGlobalString(Value.c_str(), "globalstring");
      return Builder.CreateConstGEP2_32(GS->getValueType(), GS, 0, 0, "cast");
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
    debug_print(debug_flag,"...Extern Codegen Begins...");

    llvm::Value *val = NULL;

    llvm::Type* returnTy = getType(MethodType);

    vector<llvm::Type*> args;
    if (NULL != ExternTypeList) 
    {
      list<decafAST*> stmts = ExternTypeList->return_list();  
      llvm::Type* ArgType;
      for (list<decafAST*>::iterator i = stmts.begin(); i != stmts.end(); i++)
      { 
        
        string type =  ((VarDefAST*)(*i))->getVarType();
        if(type.empty())
	{
          args.clear();
          break;      
        }
        else
        {
          ArgType = getType(type); 
	}
        args.push_back(ArgType);    
      }        
    }
  
    llvm::Function *func = llvm::Function::Create(llvm::FunctionType::get(returnTy,args,false),
                                                  llvm::Function::ExternalLinkage,
                                                  Name,
                                                  TheModule
 	 					 );
   
    verifyFunction(*func);
    val = (llvm::Value*)func;
    (symtbl.front())[Name] = val; 
    debug_print(debug_flag,"...Extern Codegen Ends...");
    return val; 
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
    if(StmtList    != NULL) { delete StmtList;    }
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
    debug_print(debug_flag, "...Block Codegen Begins...");

    symtbl.push_front(symbol_table());
    
    // if it's a method block
    if(MethodBlock)
    {
      llvm::BasicBlock* CurBB   = Builder.GetInsertBlock();
      llvm::Function* func      = CurBB->getParent();
      llvm::StringRef func_name = func->getName();
      llvm::AllocaInst* Alloca;

      unsigned int Idx = 0; 
      string arg_name;
      
      for(llvm::Function::arg_iterator i = func->arg_begin(); i != func->arg_end(); ++i)
      {
        arg_name = (*i).getName(); 
        //cout<<arg_name<<endl;

        Alloca = Builder.CreateAlloca((*i).getType() , NULL, (*i).getName());    
        Builder.CreateStore(&(*i), Alloca);  
        (symtbl.front())[arg_name] = (llvm::Value*)Alloca;
      }
    }

    if(VarDeclList != NULL) { VarDeclList->Codegen(); }
    if(StmtList    != NULL) { StmtList->Codegen();    } 
    
    symbol_table sym_table = symtbl.front();
    //free_element(sym_table);
    symtbl.pop_front(); 

    debug_print(debug_flag, "...Block Codegen Ends...");
    return NULL;
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
    if(Block   != NULL) { delete Block;  }
  }

  string getName()
  {
    return Name;
  }

  string str()
  {
    return string("Method") 
                  + "(" 
                  + Name + "," + MethodType + "," + getString(ArgList) + "," + getString(Block)
                  + ")";
  }

  llvm::Function* prototype()
  {
    //cout<<"func name:"<<Name<<endl;
    llvm::Function *func;
    llvm::Type *returnTy;
    list<decafAST*> stmts;

    returnTy = getType(MethodType);
    //cout<<"Return Type: "<<MethodType<<endl;

    if(ArgList != NULL) 
    {
      stmts = ArgList->return_list(); 
      ArgList->Codegen();
    }
   
    vector<llvm::Type *> arg_types;
    vector<string>arg_names;
    llvm::Type* arg_type;
    string arg_name;    
    VarDefAST* e;
    for (list<decafAST*>::iterator i = stmts.begin(); i != stmts.end(); i++)
    {  
      e = (VarDefAST*)(*i);
      arg_type = getType(e->getVarType());
      arg_name = e->getName(); 
      arg_types.push_back(arg_type);  
      arg_names.push_back(arg_name);  
    }   

    func = llvm::Function::Create(llvm::FunctionType::get(returnTy, arg_types, false),
                                  llvm::Function::ExternalLinkage,
                                  Name,
                                  TheModule
                     		 );
  
    unsigned int Idx = 0;
    for(auto &Arg : func->args())
    {
      Arg.setName(arg_names[Idx++]);
    }

    // assuming there are no function duplicates....
    (symtbl.front())[Name] = (llvm::Value*) func;

    return func;
  }

  llvm::Value *Codegen()
  {
    debug_print(debug_flag,"...Method Codegen Begins...");

    llvm::Function *func = (llvm::Function*)access_symtbl(Name);
    llvm::Type *returnTy = getType(MethodType);

    list<decafAST*> stmts;
 
    if(ArgList != NULL) 
    {
      stmts = ArgList->return_list(); 
      ArgList->Codegen();
    }
    
    // create a new basic block which contains a sequence of LLVM instructions 
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", func);

    // insert "entry" into symbol table (will be used in hw4)
       
    // all subsequent calls to IRBuilder wlil place instructions in this location 
    Builder.SetInsertPoint(BB);
    
    if(Block != NULL) 
    {
      Block->Codegen(); 
    }

    if(returnValue == NULL)    
    {
      if(returnTy->isVoidTy())
      { 
        Builder.CreateRet(NULL);
      }
      else
      {
        // create default return 
        if(returnTy->isIntegerTy(32)) 
        { returnValue = Builder.getInt32(0); }
        else //if(returnTy->isIntegerTy(1))  
        { returnValue = Builder.getInt1(1) ; } 
        Builder.CreateRet(returnValue);
        returnValue = NULL;
      }
    }

    verifyFunction(*func);
  
    debug_print(debug_flag,"...Method Codegen Ends..."); 
 
    return (llvm::Value*)func;
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
    debug_print(debug_flag,"...Package Codegen Begins...");

    llvm::Value *val = NULL;

    TheModule->setModuleIdentifier(llvm::StringRef(Name)); 

    if (NULL != FieldDeclList) 
    {
      val = FieldDeclList->Codegen();
    }
    if (NULL != MethodDeclList) 
    {
      
      list<decafAST*>stmts = MethodDeclList->return_list();
      //cout<<"stmts size "<<stmts.size()<<endl;
      for (list<decafAST*>::iterator i = stmts.begin(); i != stmts.end(); i++)
      {   
        MethodAST* e = (MethodAST*)(*i);
        //cout<<"func name(called from package): "<<((MethodAST*)(*i))->getName()<<endl;
        e->prototype();  
        //cout<<"here"<<endl;
      }   
      
      val = MethodDeclList->Codegen();
    } 
    
    debug_print(debug_flag,"...Package Codegen Ends...");

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
    : Name(name), FieldType(type), FieldSize(size), Assignment(isAssign) { }
   
  FieldAST(string name, string type, decafAST* argument, bool isAssign) 
    : Name(name), FieldType(type), Expr(argument), Assignment(isAssign) { } 
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
    debug_print(debug_flag, "...FieldDecl Codegen Begins...");
    llvm::Constant* Initializer;
    llvm::Type* GVType = getType(FieldType); 
    
    if(Assignment)
    {
      //cout<<"Getting Initializer"<<endl;
      Initializer =(llvm::Constant*)Expr->Codegen();
      
      if(Initializer == NULL)
      {
        //cout<<"Initializer is NULL"<<endl;  
      }
    } 
    else
    {
      if(GVType->isIntegerTy(32))     { Initializer = Builder.getInt32(0); }
      else if(GVType->isIntegerTy(1)) { Initializer = Builder.getInt1(0) ; }
      else if(GVType->isVoidTy())     { Initializer = NULL; }
    }
    

    llvm::GlobalVariable *GV;
 
    // only non-array variable can be assigned in this grammar
    if( Assignment || FieldSize == "Scalar" )
    {
      //cout<<"check"<<endl;
      GV  = new llvm::GlobalVariable(*TheModule,
                                     GVType,
                                     false,
                                     llvm::GlobalValue::InternalLinkage,
                                     Initializer,
                                     Name
				    ) ;
    }
    else // global array 
    {
      llvm::ArrayType *arrayi32 = llvm::ArrayType::get(GVType, 10);  
      llvm::Constant  *zeroInit = llvm::Constant::getNullValue(arrayi32);
      GV = new llvm::GlobalVariable(*TheModule, 
                                    arrayi32, 
                                    false, 
                                    llvm::GlobalValue::ExternalLinkage, 
                                    zeroInit, 
                                    Name);
    }

  
    if(GV == NULL) { cout<<"GV "<<Name<<" is NULL "<<endl;}
    (symtbl.front())[Name] = (llvm::Value*) GV;
    //cout<<"Storing name in symtbl: "<<Name<<endl;

    debug_print(debug_flag, "...FieldDecl Codegen Ends...");
    return GV;
  }
};

class MethodCallAST : public decafAST
{
  string Name;
  decafStmtList *ArgList;

public: 
  MethodCallAST(string name, decafStmtList *alist) : Name(name), ArgList(alist)
  {
    
  }  
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
    // CreateEntryBlockAlloca with the variable_name and type 
    debug_print(debug_flag, "...MethodCall Codegen Begins...");

    llvm::Value* val = NULL;
    llvm::Function* func = (llvm::Function*)access_symtbl(Name);
    list<decafAST*> stmts;
    vector<llvm::Value*> arg_values;
    vector<llvm::Type*> arg_types;
    llvm::Function *call;
    bool isVoid;

    if(ArgList != NULL)
    {
      stmts = ArgList->return_list();
    }
    
    if(func != NULL) 
    {
      llvm::Function::arg_iterator args;   
      call = func; 
   
      for (list<decafAST*>::iterator i = stmts.begin(); i != stmts.end(); i++, args++)
      {         
        args = call->arg_begin();
 	llvm::Value* arg_value = (*i)->Codegen();  
	llvm::Type*  arg_type  = (*args).getType();
        
        if(arg_value->getType()->isIntegerTy(1) && arg_type->isIntegerTy(32))
	{
	  arg_value = Builder.CreateZExt(arg_value, Builder.getInt32Ty(), "zexttmp");  
	}
        
        arg_values.push_back(arg_value);    
      }   
    }

    isVoid    = call->getReturnType()->isVoidTy();
    val       = Builder.CreateCall(call, arg_values, isVoid ? "" : "calltmp"); 
    debug_print(debug_flag, "...MethodCall Codegen Ends...");
    return val;
  }
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
   
  string getName() { return Name; }  
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
    debug_print(debug_flag,"...Value Codegen Begins...");
    
    //cout<<"Accessing name in symtbl: "<<Name<<endl;
    llvm::Value* val = access_symtbl(Name);

    if(ArrayFlag == false)
    {
      if(val != NULL)
      { 
        val = Builder.CreateLoad(val,Name);         
      }
    }
    else
    { 
      llvm::GlobalVariable * GV = (llvm::GlobalVariable*)val;
      llvm::ArrayType *arrayi32 = (llvm::ArrayType*)GV->getType();
      llvm::Value     *ArrayLoc = Builder.CreateStructGEP(arrayi32, GV, 0, "arrayloc");
      llvm::Value     *Index    = IndexExpr->Codegen() ;
      llvm::Value   *ArrayIndex = Builder.CreateGEP(arrayi32->getElementType(), ArrayLoc, Index, "arrayindex");
      val = Builder.CreateLoad(ArrayIndex, "loadtmp");
    }
    debug_print(debug_flag,"...Value Codegen Ends...");
    return val;
  }
};  

class AssignAST : public decafAST
{
  ValueAST* Value;
  decafAST* Expr;
   
public: 
  AssignAST(ValueAST* value, decafAST* expr) : Value(value), Expr(expr)
  {

  }
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
      return Name + "(" + Value->getName() + "," + getString(Expr) + ")";
    }
    else
    {
      Name = string("AssignArrayLoc");
      return Name + "(" + Value->getName() + "," + getString(Value->getIndexExpr()) + ","+ getString(Expr) + ")";
    } 
  }
  llvm::Value *Codegen() 
  {
    debug_print(debug_flag,"...Assign Codegen Begins...");
    llvm::Value *val = NULL; 
    llvm::Value *LValue;
    llvm::Value *RValue;

    LValue = access_symtbl(Value->getName());    
    if(Value->isArray())
    {   
      llvm::GlobalVariable * GV = (llvm::GlobalVariable*)LValue;
      llvm::ArrayType *arrayi32 = (llvm::ArrayType*)GV->getType();
      llvm::Value     *ArrayLoc = Builder.CreateStructGEP(arrayi32, GV, 0, "arrayloc");
      llvm::Value     *Index    = (Value->getIndexExpr())->Codegen() ;
      llvm::Value   *ArrayIndex = Builder.CreateGEP(arrayi32->getElementType(), ArrayLoc, Index, "arrayindex");
      LValue = ArrayIndex;
    }

    RValue = Expr->Codegen();  

    if((LValue->getType()->isIntegerTy(32) == true) &&
       (RValue->getType()->isIntegerTy(1)  == true))
    {  
      RValue = Builder.CreateZExt(RValue, Builder.getInt32Ty(), "zexttmp");    
    }

    const llvm::PointerType *ptrTy = RValue->getType()->getPointerTo();
    
    if(LValue->getType() == ptrTy)
    {
      // not clear whether RValue is the address or the actually value stored 
      val = Builder.CreateStore(RValue, LValue);
    }              
    
    debug_print(debug_flag,"...Assign Codegen Ends...");
    return val;
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
    return string("IfStmt")     + "(" +
           getString(Condition) + "," +
           getString(IfBlock)   + "," + 
           getString(ElseBlock) + ")";
   }
  llvm::Value *Codegen() 
  {
    llvm::BasicBlock *CurBB = Builder.GetInsertBlock();
    llvm::Function *func    = CurBB->getParent();
  
    llvm::BasicBlock* IfStartBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "0_ifstart", func);
    llvm::BasicBlock* IfTrueBB  = llvm::BasicBlock::Create(llvm::getGlobalContext(), "0_iftrue",  func);
    llvm::BasicBlock* IfFalseBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "0_iffalse", func);
    llvm::BasicBlock* IfEndBB   = llvm::BasicBlock::Create(llvm::getGlobalContext(), "0_ifend", func);     

    (symtbl.front())["0_ifstart"] = IfStartBB;
    (symtbl.front())["0_iftrue"]  = IfTrueBB; 
    (symtbl.front())["0_iffalse"] = IfFalseBB;
    (symtbl.front())["0_ifend"]   = IfEndBB;

    Builder.CreateBr(IfStartBB);

    // check if condition is true or false
    // if true:   Create branch to IfTrueBB
    // if false:  Create branch to EndBB

    Builder.SetInsertPoint(IfStartBB);
    llvm::Value* Cond = Condition->Codegen();   
    
    Builder.CreateCondBr(Cond, IfTrueBB, IfFalseBB);

    // Insert instruction to IfTrueBB
    Builder.SetInsertPoint(IfTrueBB);
    IfBlock->Codegen();
    Builder.CreateBr(IfEndBB);

    // Insert instruction to IfFalseBB 
    Builder.SetInsertPoint(IfFalseBB);

    if(ElseBlock != NULL)
    {
      ElseBlock->Codegen();
    }

    Builder.CreateBr(IfEndBB);
 
    // Insert instruction to IfEndBB(from MethodAST)
    Builder.SetInsertPoint(IfEndBB);         
    return NULL; 
  }
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
  llvm::Value *Codegen()
  { 
    llvm::BasicBlock *CurBB = Builder.GetInsertBlock();
    llvm::Function *func    = CurBB->getParent();
  
    llvm::BasicBlock* WhileStartBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "0_whilestart", func);
    llvm::BasicBlock* WhileTrueBB  = llvm::BasicBlock::Create(llvm::getGlobalContext(), "0_whiletrue",  func);
    llvm::BasicBlock* WhileEndBB   = llvm::BasicBlock::Create(llvm::getGlobalContext(), "0_whileend", func);     

    (symtbl.front())["0_loopstart"] = WhileStartBB;
    (symtbl.front())["0_looptrue"]  = WhileTrueBB; 
    (symtbl.front())["0_loopend"]   = WhileEndBB;
       
    Builder.CreateBr(WhileStartBB);
    
    Builder.SetInsertPoint(WhileStartBB);
    llvm::Value* Cond = Condition->Codegen(); 

    Builder.CreateCondBr(Cond, WhileTrueBB, WhileEndBB);
    
    Builder.SetInsertPoint(WhileTrueBB);
    WhileBlock->Codegen(); 
    Builder.CreateBr(WhileStartBB);   

    // Insert instruction to WhileEndBB
    Builder.SetInsertPoint(WhileEndBB);
   
    (symtbl.front()).erase("0_loopstart");
    (symtbl.front()).erase("0_looptrue") ;
    (symtbl.front()).erase("0_loopend");
   

    return NULL; 
  }
};

class ForStmtAST : public decafAST
{
  AssignAST* PreAssign;
  decafAST*  Condition;
  AssignAST* PostAssign;
  BlockAST*  ForBlock;
    
public:
  ForStmtAST(AssignAST* pre_assign, decafAST* condition, AssignAST* post_assign, BlockAST* for_block)
           : PreAssign(pre_assign), Condition(condition), PostAssign(post_assign), ForBlock(for_block){}  
  ~ForStmtAST()
  {
    if(PreAssign  != NULL) { delete PreAssign;  }
    if(Condition  != NULL) { delete Condition;  }
    if(PostAssign != NULL) { delete PostAssign; }
    if(ForBlock   != NULL) { delete ForBlock;   }
  }

  string str()
  { 
    return string("ForStmt") + "(" + getString(PreAssign)  + "," 
                                   + getString(Condition)  + "," 
                                   + getString(PostAssign) + "," 
                                   + getString(ForBlock)   +  ")";
  }   
  llvm::Value *Codegen() 
  {
    llvm::BasicBlock *CurBB = Builder.GetInsertBlock();
    llvm::Function *func     = CurBB->getParent();
    
    llvm::BasicBlock* ForStartBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "0_forstart", func);
    llvm::BasicBlock* ForTrueBB  = llvm::BasicBlock::Create(llvm::getGlobalContext(), "0_fortrue",  func);
    llvm::BasicBlock* ForPostBB  = llvm::BasicBlock::Create(llvm::getGlobalContext(), "0_forpost",  func);
    llvm::BasicBlock* ForEndBB   = llvm::BasicBlock::Create(llvm::getGlobalContext(), "0_forend",   func);     

    (symtbl.front())["0_loopstart"]  = ForStartBB;
    (symtbl.front())["0_looptrue"]   = ForTrueBB; 
    (symtbl.front())["0_loopassign"] = ForPostBB;
    (symtbl.front())["0_loopend"]    = ForEndBB;

    PreAssign->Codegen();
    
    // Condition check
    Builder.CreateBr(ForStartBB);
    Builder.SetInsertPoint(ForStartBB);

    llvm::Value* Cond = Condition->Codegen();
   
    // Condition branch
    Builder.CreateCondBr(Cond, ForTrueBB, ForEndBB);

    Builder.SetInsertPoint(ForTrueBB);
    ForBlock->Codegen();
    Builder.CreateBr(ForPostBB);
   
    Builder.SetInsertPoint(ForPostBB); 
    PostAssign->Codegen();
    Builder.CreateBr(ForStartBB);

    Builder.SetInsertPoint(ForEndBB);

    //(symtbl.front()).erase("0_loopstart");
    //(symtbl.front()).erase("0_looptrue") ;
    //(symtbl.front()).erase("0_loopassign");
    //(symtbl.front()).erase("0_loopend");
  }
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
    
    llvm::Value* val;
    /*
    llvm::BasicBlock *CurBB = Builder.GetInsertBlock();
    llvm::Function *func    = CurBB->getParent();
    llvm::Type* returnTy    = func->getReturnType();
    */

    if(Expr != NULL)
    { 
      val = Expr->Codegen();
      returnValue = val;
      Builder.CreateRet(returnValue);
      returnValue = NULL;
    }
    return val;
  }
};

class BreakStmtAST : public decafAST
{
public: 

  string str()
  {
    return string("BreakStmt");
  }
  llvm::Value *Codegen() 
  {
    llvm::BasicBlock* EndBB = (llvm::BasicBlock*)(access_symtbl("0_loopend")); 
    if(EndBB != NULL)
    {
      Builder.CreateBr(EndBB);
    }   
    else
    {
      //throw semantic error
    }   
  }
};

class ContinueStmtAST : public decafAST
{  
public: 
  string str()
  {
    return string("ContinueStmt");
  }
  llvm::Value *Codegen() 
  {
    llvm::BasicBlock* StartBB = (llvm::BasicBlock*)(access_symtbl("0_loopstart")); 
    if(StartBB != NULL)
    {
      Builder.CreateBr(StartBB);
    }   
    else
    {
      //throw semantic error
    }
  }
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
  llvm::Value *Codegen() 
  { 
    debug_print(debug_flag, "...BinaryOp Codegen Begins..."); 

    llvm::Value* val = nullptr;
    llvm::Value* LValue;
    llvm::Value* RValue;

    llvm::BasicBlock *CurBB;
    llvm::BasicBlock* RBB;
    llvm::Function *func;
    llvm::BasicBlock* MergeBB;
    llvm::PHINode* phi;

    int CurOp = getOperator(BinaryOp);
    if((CurOp != T_AND) && (CurOp != T_OR))
    {
      LValue = LeftValue->Codegen();
      RValue = RightValue->Codegen();
    }
    else
    {
      // control flow basic blocks for boolean short circuiting 
      CurBB   = Builder.GetInsertBlock();
      func    = CurBB->getParent();
      RBB     = llvm::BasicBlock::Create(llvm::getGlobalContext(), "rval", func); 
      MergeBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "merge", func); 
    }  

    switch(getOperator(BinaryOp))
    {  
      case T_AND: 
      {
        //Builder.CreateBr(LBB);
        //Builder.SetInsertPoint(LBB);
        LValue = LeftValue->Codegen();
        Builder.CreateCondBr(LValue, RBB, MergeBB);

        Builder.SetInsertPoint(RBB);
        RValue = RightValue->Codegen();
        RBB    = Builder.GetInsertBlock(); // update the current block 
        Builder.CreateBr(MergeBB);        
 
        Builder.SetInsertPoint(MergeBB);                     
 	phi = Builder.CreatePHI(LValue->getType(), 2, "phival"); 
        phi->addIncoming(LValue, CurBB);
        phi->addIncoming(RValue, RBB);
  
        val = (llvm::Value*)phi;
        break;
      }
      case T_OR:
      {
        //Builder.CreateBr(LBB);
        //Builder.SetInsertPoint(LBB);
        LValue = LeftValue->Codegen();
        Builder.CreateCondBr(LValue, MergeBB, RBB);

        Builder.SetInsertPoint(RBB);
        RValue = RightValue->Codegen();
        RBB    = Builder.GetInsertBlock(); // update the current block 
        Builder.CreateBr(MergeBB);        
         
        Builder.SetInsertPoint(MergeBB);                     
 	phi = Builder.CreatePHI(LValue->getType(), 2, "phival"); 
        phi->addIncoming(LValue, CurBB);
        phi->addIncoming(RValue, RBB);
        
        val = (llvm::Value*)phi;
        break;
      }
 
      case T_PLUS: 
      {  
        val = Builder.CreateAdd(LValue,RValue,"addtmp");
        break;
      }
      case T_MINUS: 
      {  
        val = Builder.CreateSub(LValue,RValue,"subtmp");
        break;
      }
      case T_MULT: 
      {  
        val = Builder.CreateMul(LValue,RValue,"multmp");
        break;
      }
      case T_DIV: 
      {  
        val = Builder.CreateSDiv(LValue,RValue,"divtmp");
        break;
      }
      case T_LEFTSHIFT:
      {
        val = Builder.CreateShl(LValue, RValue,"lstmp");
        break;
      }
      case T_RIGHTSHIFT:
      {
        val = Builder.CreateLShr(LValue, RValue,"rstmp");
        break;
      }
      case T_MOD:
      {
        val = Builder.CreateSRem(LValue, RValue,"modtmp");
        break;
      }
      case T_EQ:
      {
        val = Builder.CreateICmpEQ(LValue, RValue,"eqtmp");
        break;
      }
      case T_NEQ:
      {
        val = Builder.CreateICmpNE(LValue, RValue,"neqtmp");
        break;
      }
      case T_LT:
      {
        val = Builder.CreateICmpSLT(LValue, RValue,"lttmp");        
        break;
      }
      case T_GT:
      {
        val = Builder.CreateICmpSGT(LValue, RValue,"gttmp");
        break;
      }
      case T_LEQ:
      {
        val = Builder.CreateICmpSLE(LValue, RValue,"leqtmp");
        break;
      }
      case T_GEQ:
      {
        val = Builder.CreateICmpSGE(LValue, RValue,"geqtmp");
        break;
      }
      default: break;
    }
    debug_print(debug_flag, "...BinaryOp Codegen Ends...");
    return val;
  }
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
  llvm::Value *Codegen() 
  {
    debug_print(debug_flag, "...UnaryOp Codegen Begins...");
    llvm::Value* val = NULL;
    llvm::Value* RValue = RightValue->Codegen();
    
    switch(getOperator(UnaryOp))
    {
      case T_NOT:
      {
        val = Builder.CreateNot(RValue,"unottmp");
        break;
      }
      case T_MINUS:
      {
        val = Builder.CreateNeg(RValue,"unegtmp");
        break;
      }
      default: break;
    }
    debug_print(debug_flag, "...UnaryOp Codegen Ends...");
    return val;
  }
};
