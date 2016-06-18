
#include "decafast-defs.h"
#include <list>
#include <ostream>
#include <iostream>
#include <sstream>


#ifndef YYTOKENTYPE
#include <deque>
#include "decafast.tab.h"
#endif

using namespace std;

/// decafAST - Base class for all abstract syntax tree nodes.
class decafAST 
{
public:
  virtual ~decafAST() {}
  virtual string str() { return string(""); }
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
  decafAST* pop_back()       
  { 
    decafAST* e = stmts.back();
    stmts.pop_back(); 
    return e;
  }
  string str() { return commaList<class decafAST *>(stmts); }
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
};

class ConstantAST : public decafAST
{
  string Type;
  string Value;

public:
  ConstantAST(string type, string value) : Type(type), Value(value){}
  ~ConstantAST(){}  
  
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
};

class FieldAST : public decafAST
{
  string Name;
  string FieldType;
  string FieldSize;
  ConstantAST* AssignExpr;
  bool Assignment;
public:
  
  // Assignment == false 
  FieldAST(string name, string type, string size) 
    : Name(name), FieldType(type), FieldSize(size), Assignment(false), AssignExpr(NULL){}
  
  // Assignment == true 
  FieldAST(string name, string type, ConstantAST* expr) 
           : Name(name), FieldType(type), AssignExpr(expr), Assignment(true){}
  ~FieldAST()
  {
    if(AssignExpr != NULL) { delete AssignExpr; }  
  }
  
  void setAssignment(bool flag)
  {
    Assignment = flag;
  }
  
  string str()
  {
    if(Assignment == true)
    {
      return string("AssignGlobalVar") + "(" + Name + "," + FieldType + "," + getString(AssignExpr) +")"; 
    } 
    else
    {
      return string("FieldDecl") + "(" + Name + "," + FieldType + "," + FieldSize + ")";
    }
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
};

class VarDefAST : public decafAST
{
  string Name;
  string VarType;

public:
  
  VarDefAST(string name, string type) : Name(name), VarType(type){}
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
};

class ValueAST : public decafAST
{
  string Name;
  decafStmtList* IndexExpr;
  bool ArrayFlag;

public: 
  ValueAST(string name) : Name(name), ArrayFlag(false) {}
  ValueAST(string name, decafStmtList* index) : Name(name), IndexExpr(index), ArrayFlag(true){}
  ~ValueAST(){}
   
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
};

class BreakStmtAST : public decafAST
{
public: 

  string str()
  {
    return string("BreakStmt");
  }
};

class ContinueStmtAST : public decafAST
{  
public: 
  string str()
  {
    return string("ContinueStmt");
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
};
