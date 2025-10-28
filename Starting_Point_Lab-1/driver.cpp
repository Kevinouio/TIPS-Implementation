//*****************************************************************************
// purpose: driver file for CSE 4714 Lab 1
// version: Fall 2024
//  author: Ed Swan / Joe Crumpton
//
// Speak to the TA or Instructor if you think that you need to modify this file.
//
//*****************************************************************************
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h> // Comment
#include "lexer.h"

extern "C"
{
    // Instantiate global variables
    extern FILE *yyin;         // input stream
    extern FILE *yyout;        // output stream
    extern int   yyleng;       // length of current lexeme
    extern char *yytext;       // text of current lexeme
    extern int   yylex();      // the generated lexical analyzer
    extern int   line_number;  // the current line number
}

// Do the analysis
int main( int argc, char* argv[] )
{
  int token;   // hold each token code
  char* token_str;  // point to the logical name of the token

  // Set the input stream
  if (argc > 1) {
    printf("INFO: Using the %s file for input\n", argv[1]);
    yyin = fopen(argv[1], "r"); 
  }
  else {
    printf("INFO: Using the sample.csv file for input\n");
    yyin = fopen("sample.csv", "r");
  }
  
  if (!yyin) {
    printf("ERROR: input file not found\n");
    return (-1);
  }

  // Set the output stream
  yyout = stdout;

  // Do the lexical parsing
  token = yylex();
  while( true ) 
  {
    if ( token == EOF_TOKEN ) {
      printf("Found end of file...\n");
      break;
    }

    // Determine the logical name of the lexeme
    switch( token )
    {
      case DATE:             token_str = (char*)"DATE"; break;
      case SEPARATOR:        token_str = (char*)"SEPARATOR"; break;
      case YES:              token_str = (char*)"YES"; break;
      case NO:               token_str = (char*)"NO"; break;
      case UNKNOWN_VALUE:    token_str = (char*)"UNKNOWN_VALUE"; break;
      case MISSING:          token_str = (char*)"MISSING"; break;

      case LABORATORY:       token_str = (char*)"LABORATORY"; break;    
      case PROBABLE:         token_str = (char*)"PROBABLE"; break;    

      case MALE:             token_str = (char*)"MALE"; break;    
      case FEMALE:           token_str = (char*)"FEMALE"; break;    
      case OTHER:            token_str = (char*)"OTHER"; break;    

      case AGE_0X:           token_str = (char*)"AGE_0X"; break;    
      case AGE_1X:           token_str = (char*)"AGE_1X"; break;    
      case AGE_2X:           token_str = (char*)"AGE_2X"; break;   
      case AGE_4X:           token_str = (char*)"AGE_4X"; break;    
      case AGE_5X:           token_str = (char*)"AGE_5X"; break;    
      case AGE_6X:           token_str = (char*)"AGE_6X"; break;    
      case AGE_7X:           token_str = (char*)"AGE_7X"; break;    
      case AGE_8X:           token_str = (char*)"AGE_8X"; break;    

      case HISPANIC:         token_str = (char*)"HISPANIC"; break;
      case NATIVE_AMERICAN:  token_str = (char*)"NATIVE_AMERICAN"; break;    
      case ASIAN:            token_str = (char*)"ASIAN"; break; 
      case BLACK:            token_str = (char*)"BLACK"; break; 
      case PACIFIC_ISLANDER: token_str = (char*)"PACIFIC_ISLANDER"; break; 
      case WHITE:            token_str = (char*)"WHITE"; break; 
      case MULTIPLE_OTHER:   token_str = (char*)"MULTIPLE_OTHER"; break; 
      case EOF_TOKEN:        token_str = (char*)"EOF_TOKEN"; break;
      case UNKNOWN_TOKEN:    token_str = (char*)"UNKNOWN_TOKEN"; break;

      default:              token_str = (char*)"=== unmapped token name ===";
    }

    // What did we find?
    printf("line: %d  lexeme: |%s|  length: %d  token: %s\n", 
        line_number, yytext, yyleng, token_str);
    
    // Is it an error?
    if( token == UNKNOWN_TOKEN ) {
      printf("ERROR: unknown token\n");
      return(-2);
    }

    // Get the next token
    token = yylex();
  }

  return(0);
}
