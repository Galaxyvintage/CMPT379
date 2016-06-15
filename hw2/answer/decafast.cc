
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
    if (FieldDeclList  != NULL) 
    { 
      //cout<<"releasing FieldDeclList in Package"<<endl;  
      delete FieldDeclList; 
    }
    if (MethodDeclList != NULL)
    { 
      //cout<<"releasing MethodDeclList in Package"<<endl; 
      delete MethodDeclList;
    }
  }
  string str() 
  { 
    return string("Package") + "(" + Name + "," + getString(FieldDeclList) + "," + getString(MethodDeclList) + ")";
  }
};

/// ProgramAST - the decaf program
class ProgramAST : public decafAST 
{
  decafStmtList *ExternList;
  PackageAST *PackageDef;
public:
  ProgramAST(decafStmtList *externs, PackageAST *c) : ExternList(externs), PackageDef(c) {}
  ~ProgramAST() 
  { 
    if (ExternList != NULL) 
    { 
      // cout<<"releasing ExternList in Program"<<endl; 
      delete ExternList;
    } 
    if (PackageDef != NULL) 
    { 
      //cout<<"releasing PackageDef in Program"<<endl; 
      delete PackageDef; 
    }
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
      Name = string("NumExpr");
    }
    else if(Type == string("StringType"))
    {
      Name = string("StringConstant");
    }
    else if(Type == string("BoolExpr"))
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
  FieldAST(string name, string type, string size) 
    : Name(name), FieldType(type), FieldSize(size), Assignment(false), AssignExpr(NULL){}
  
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
  string Name;
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
    return string("MethodDecl") 
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
    if(Name.empty())
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

class RValueAST : public decafAST
{
  string Name;
  decafStmtList* Index;
  bool isArray;

public: 
  RValueAST(string name) : Name(name), isArray(false) {}
  RValueAST(string name, decafStmtList* index) : Name(name), Index(index), isArray(true){}
  ~RValueAST(){}
   
  string Str()
  {
    if(isArray == false)
    {
      return string("VariableExpr") + "(" + Name + ")";
    } 
    else
    {
      return string("ArrayLocExpr") + "(" + Name + "," + getString(Index) + "," +")";
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
               :BinaryOp(op), LeftValue(left), RightValue(right){}
  ~BinaryExprAST()
   {
     if(LeftValue != NULL)  { delete LeftValue; }
     if(RightValue != NULL) { delete RightValue; }
   }

  string Str()
  {
    return string("BinaryExpr") + "(" + BinaryOp + "," + getString(LeftValue) + "," + getString(RightValue) + ")";
  }
};

class ExprAST : public decafAST
{
  


};
