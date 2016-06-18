%{
#include <cstring>
#include <string>
#include <deque>
#include <sstream>
#include <iostream>
#include "decafast-defs.h"
#include "decafast.tab.h"
using namespace std;

int lineno = 1;
int tokenpos = 1;

%}


  /* regular expression */
letter        [a-zA-Z\_]
decimal_digit [0-9]
hex_digit     [0-9a-fA-F]
digit         [0-9]

char_lit      [\a\b\h\v\f\r -\[\]-~]|\\(n|r|t|v|f|a|b|\\|\'|\")

%%
  /*
    Pattern definitions for all tokens
  */

 /*
  *  Note:
  *   1. maximal munch
  *   2. choose the one listed first if there is a conflict
  *   3. consider also that a single whitespace token could have multiple newlines
  */



  /*  Keywords (18 of them) */

func                       { return T_FUNC;       }
package                    { return T_PACKAGE;    }
var                        { return T_VAR;        }
int                        { return T_INTTYPE;    }
string                     { return T_STRINGTYPE; }
bool       		   { return T_BOOLTYPE;   }
void                       { return T_VOID;       }
null                       { return T_NULL;       }
break                      { return T_BREAK;      }
continue                   { return T_CONTINUE;   }
extern                     { return T_EXTERN;     }
true                       { return T_TRUE;       }
false                      { return T_FALSE;      }
if                         { return T_IF;         }
else                       { return T_ELSE;       }
for                        { return T_FOR ;       }
while                      { return T_WHILE;      }
return                     { return T_RETURN;     }

  /* Operators and Delimiters(26 of them) */

\{                         { return  T_LCB;       }
\}                         { return  T_RCB;       }
\(                         { return  T_LPAREN;    }
\)                         { return  T_RPAREN;    }
\[                         { return  T_LSB;       }
\]                         { return  T_RSB;       }
\,                         { return  T_COMMA;     }
\;                         { return  T_SEMICOLON; }

\=\=                       { yylval.sval = new string("Eq");  return  T_EQ;      }
\<\=                       { yylval.sval = new string("Leq"); return  T_LEQ;     }
\>\=                       { yylval.sval = new string("Geq"); return  T_GEQ;     }
\!\=                       { yylval.sval = new string("Neq"); return  T_NEQ;     }
\<\<                       { yylval.sval = new string("Leftshift");  return  T_LEFTSHIFT; }
\>\>                       { yylval.sval = new string("Rightshift");  return  T_RIGHTSHIFT;}
\&\&                       { yylval.sval = new string("And");  return  T_AND;   }
\|\|                       { yylval.sval = new string("Or");    return  T_OR;    }

\+                         { yylval.sval = new string("Plus");  return  T_PLUS;  }
\-                         { yylval.sval = new string("Minus"); return  T_MINUS; }
\*                         { yylval.sval = new string("Mult");  return  T_MULT;  }
\/                         { yylval.sval = new string("Div") ;  return  T_DIV;   }
\!                         { yylval.sval = new string("Not") ;  return  T_NOT;   }
\=                         { return  T_ASSIGN;    }
\<                         { yylval.sval = new string("Lt");    return  T_LT;    }
\>                         { yylval.sval = new string("Gt");    return  T_RT;    }
\%                         { yylval.sval = new string("Mod");   return  T_MOD;   }
\.                         { return  T_DOT;       }

  /* Others */

\/\/([\a|\b|\h|\v|\f|\r| -~]+)\n                               { lineno = lineno + 1; }
({decimal_digit}+)|(0(x|X){hex_digit}+)                        { 
                                                                 yylval.sval = new string(yytext);
                                                                 return  T_INTCONSTANT; 
                                                               }
\'{char_lit}\'                                                 {  
                                                                 yylval.sval = new string(yytext);
                                                                 return  T_CHARCONSTANT;  
                                                               }

\"([\a\b\h\v\f\r -\!\#-\[\]-~]|\\(n|r|t|v|f|a|b|\\|\'|\"))*\"  {
                                                                 yylval.sval = new string(yytext);
                                                                 return  T_STRINGCONSTANT; 
                                                               }

[a-zA-Z\_][a-zA-Z\_0-9]*   {
                             yylval.sval = new string(yytext); 
                             return  T_ID; 
                           }
[\t\r\v\f ]+                   //{ return  T_WHITESPACE;}
\n                             { lineno = lineno + 1; tokenpos = 1;} 

\'{char_lit}([{char_lit}]+)\'  { return T_ERROR_1; }
\'\'                           { return T_ERROR_2; }
.                              { return T_ERROR_3; }

%%


int yyerror(const char *s) 
{
  cerr <<"line "<< lineno << ": " << s << " at char " << tokenpos << endl;
  return 1;
}

