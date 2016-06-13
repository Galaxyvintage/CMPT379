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
package                    {  return T_PACKAGE;    }
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

\=\=                       { return  T_EQ;        }
\<\=                       { return  T_LEQ;       }
\=\>                       { return  T_GEQ;       }
\!\=                       { return  T_NEQ;       }
\<\<                       { return  T_LEFTSHIFT; }
\>\>                       { return  T_RIGHTSHIFT;}
\&\&                       { return  T_AND;       }
\|\|                       { return  T_OR;        }

\+                         { return  T_PLUS;      }
\-                         { return  T_MINUS;     }
\*                         { return  T_MULT;      }
\/                         { return  T_DIV;       }
\!                         { return  T_NOT;       }
\=                         { return  T_ASSIGN;    }
\<                         { return  T_LT;        }
\>                         { return  T_RT;        }
\%                         { return  T_MOD;       }
\.                         { return  T_DOT;       }

  /* Others */

\/\/([\a|\b|\h|\v|\f|\r| -~]+)\n                               { return  T_COMMENT;        }
({decimal_digit}+)|(0(x|X){hex_digit}+)                        { return  T_INTCONSTANT;    }
\'{char_lit}\'                                                 { return  T_CHARCONSTANT;   }
\"([\a\b\h\v\f\r -\!\#-\[\]-~]|\\(n|r|t|v|f|a|b|\\|\'|\"))*\"  { return  T_STRINGCONSTANT; }

[a-zA-Z\_][a-zA-Z\_0-9]*   {
                             yylval.sval = new string(yytext); 
                             return  T_ID; 
                           }
[\t\r\v\f\n ]+             //{ return  T_WHITESPACE;}


\'{char_lit}([{char_lit}]+)\'  { return T_ERROR_1; }
\'\'                           { return T_ERROR_2; }
.                              { return T_ERROR_3; }

%%

int yyerror(const char *s) 
{
  cerr << lineno << ": " << s << " at char " << tokenpos << endl;
  return 1;
}

