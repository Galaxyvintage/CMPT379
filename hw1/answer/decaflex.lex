%{
/*
 * Author      : Zheyang Li
 * Date Created: 05-26-2016
 * Date Edited : 06-06-2016
 * Deadline    : 06-06-2016
 * Description : lex analyzer for Decaf language
 */

#include <iostream>
#include <cstdlib>

#define T_FUNC         1
#define T_PACKAGE      2
#define T_VAR          3
#define T_INTTYPE      4
#define T_STRINGTYPE   5
#define T_BOOLTYPE     6
#define T_VOID         7
#define T_NULL         8
#define T_BREAK        9
#define T_CONTINUE     10
#define T_EXTERN       11
#define T_TRUE         12
#define T_FALSE        13
#define T_IF           14
#define T_ELSE         15
#define T_FOR          16
#define T_WHILE        17
#define T_RETURN       18
#define T_LCB          19
#define T_RCB          20
#define T_LPAREN       21
#define T_RPAREN       22
#define T_LSB          23
#define T_RSB          24
#define T_COMMA        25
#define T_SEMICOLON    26
#define T_EQ           27
#define T_LEQ          28
#define T_GEQ          29
#define T_NEQ          30
#define T_LEFTSHIFT    31
#define T_RIGHTSHIFT   32
#define T_AND          33
#define T_OR           34
#define T_PLUS         35
#define T_MINUS        36
#define T_MULT         37
#define T_DIV          38
#define T_NOT          39
#define T_ASSIGN       40
#define T_LT           41
#define T_RT           42
#define T_MOD          43
#define T_DOT          44
#define T_INTCONSTANT    45
#define T_CHARCONSTANT   46
#define T_STRINGCONSTANT 47
#define T_ID           48
#define T_WHITESPACE   49
#define T_COMMENT      50

#define T_ERROR_1      51
#define T_ERROR_2      52
#define T_ERROR_3      53

using namespace std;
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

[a-zA-Z\_][a-zA-Z\_0-9]*   { return  T_ID;        }
[\t\r\v\f\n ]+             { return  T_WHITESPACE;}


\'{char_lit}([{char_lit}]+)\'  { return T_ERROR_1; }
\'\'                           { return T_ERROR_2; }
.                              { return T_ERROR_3; }

%%

int main () {
  int token;
  string lexeme;
  int current_line = 1;

  while ((token = yylex()))
  {
    //cout<<"token is :"<<token<<endl;
    if (token > 0)
    {
      lexeme.assign(yytext);
      switch(token)
      {
        case 1: cout << "T_FUNC "            << lexeme << endl; break;
        case 2: cout << "T_PACKAGE "         << lexeme << endl; break;
        case 3: cout << "T_VAR "             << lexeme << endl; break;
        case 4: cout << "T_INTTYPE "         << lexeme << endl; break;
        case 5: cout << "T_STRINGTYPE "      << lexeme << endl; break;
        case 6: cout << "T_BOOLTYPE "        << lexeme << endl; break;
        case 7: cout << "T_VOID "            << lexeme << endl; break;
        case 8: cout << "T_NULL "            << lexeme << endl; break;
        case 9: cout << "T_BREAK "           << lexeme << endl; break;
        case 10: cout << "T_CONTINUE "       << lexeme << endl; break;
        case 11: cout << "T_EXTERN "         << lexeme << endl; break;
        case 12: cout << "T_TRUE "           << lexeme << endl; break;
        case 13: cout << "T_FALSE "          << lexeme << endl; break;
        case 14: cout << "T_IF "             << lexeme << endl; break;
        case 15: cout << "T_ELSE "           << lexeme << endl; break;
        case 16: cout << "T_FOR "            << lexeme << endl; break;
        case 17: cout << "T_WHILE "          << lexeme << endl; break;
        case 18: cout << "T_RETURN "         << lexeme << endl; break;
        case 19: cout << "T_LCB "            << lexeme << endl; break;
        case 20: cout << "T_RCB "            << lexeme << endl; break;
        case 21: cout << "T_LPAREN "         << lexeme << endl; break;
        case 22: cout << "T_RPAREN "         << lexeme << endl; break;
        case 23: cout << "T_LSB "            << lexeme << endl; break;
        case 24: cout << "T_RSB "            << lexeme << endl; break;
        case 25: cout << "T_COMMA "          << lexeme << endl; break;
        case 26: cout << "T_SEMICOLON "      << lexeme << endl; break;
        case 27: cout << "T_EQ "             << lexeme << endl; break;
        case 28: cout << "T_LEQ "            << lexeme << endl; break;
        case 29: cout << "T_REQ "            << lexeme << endl; break;
        case 30: cout << "T_NEQ "            << lexeme << endl; break;
        case 31: cout << "T_LEFTSHIFT "      << lexeme << endl; break;
        case 32: cout << "T_RIGHTSHIFT "     << lexeme << endl; break;
        case 33: cout << "T_AND "            << lexeme << endl; break;
        case 34: cout << "T_OR "             << lexeme << endl; break;
        case 35: cout << "T_PLUS "           << lexeme << endl; break;
        case 36: cout << "T_MINUS "          << lexeme << endl; break;
        case 37: cout << "T_MULT "           << lexeme << endl; break;
        case 38: cout << "T_DIV "            << lexeme << endl; break;
        case 39: cout << "T_NOT "            << lexeme << endl; break;
        case 40: cout << "T_ASSIGN "         << lexeme << endl; break;
        case 41: cout << "T_LT "             << lexeme << endl; break;
        case 42: cout << "T_RT "             << lexeme << endl; break;
        case 43: cout << "T_MOD "            << lexeme << endl; break;
        case 44: cout << "T_DOT "            << lexeme << endl; break;
        case 45: cout << "T_INTCONSTANT "    << lexeme << endl; break;
        case 46: cout << "T_CHARCONSTANT "   << lexeme << endl; break;
        case 47: cout << "T_STRINGCONSTANT " << lexeme << endl; break;
        case 48: cout << "T_ID "             << lexeme << endl; break;
        case 49:
        {
          cout << "T_WHITESPACE ";

          for(int x = 0; x < lexeme.size(); ++x)
          {
            if(lexeme[x] == '\n')
            {
              cout<<"\\n";
              current_line++;
	    }
	  }

          cout << endl;
          break;
        }
        case 50:
        {
	        cout << "T_COMMENT ";

          for(int x = 0; x < lexeme.size(); ++x)
          {
            if(lexeme[x] != '\n')
            {
              cout<<lexeme[x];
            }
          }

          cout << "\\n" << endl;
          current_line++;
          break;
        }
        case 51:
        {
          cerr << "Error: illegal character at line "   << current_line << "," << yyleng;
          exit(EXIT_FAILURE);
        }
        case 52:
        {
          cerr << "Error: empty unexpected character at line "<< current_line << "," << yyleng;
          exit(EXIT_FAILURE);
        }
        case 53:
        {
          cerr << "Error: unexpected unexpected character at line "<< current_line << "," << yyleng;
          exit(EXIT_FAILURE);
        }
        default:
        {
          exit(EXIT_FAILURE);
        }
      }
    }
    else
    {
      if (token < 0)
      {
        exit(EXIT_FAILURE);
      }
    }
  }
  exit(EXIT_SUCCESS);
}
