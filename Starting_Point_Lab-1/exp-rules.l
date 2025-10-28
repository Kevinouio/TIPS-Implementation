 /*********************************************************************
  Name: Kevin Ho             NetID: kth258
  Course: CSE 4714              
  Lab: 01 - Lexical Analyzer 
  Purpose of File: So the main goal of this file is to understand how to 
  generate a lexical analyzer.


 This is the file that you should modify for Lab 1.
 **********************************************************************/

 /*=====================================================================
  Lab 1 - exp-rules.l  (Starter Skeleton)

  Edit this file to complete the Lab 1 assignment.
  Only Sections B (Definitions) and C (Rules) need changes.
 =====================================================================*/

 /*------------------------------
  Section A: Options & Includes
  (Leave this alone)
 ------------------------------*/
%option noyywrap      
%option nounput       
%option noinput       

%{
  #include <stdio.h>
  #include "lexer.h"

  int line_number = 1;   /* Driver uses this */
%}

 /*------------------------------
  Section B: Definitions (Macros)
  Add more macros if helpful.
 ------------------------------*/
DIG    [0-9]
YEAR   {DIG}{4}
MONTH  {DIG}{2}
DAY    {DIG}{2}
WSP    [ \t]*

 /*------------------------------
  Section C: Rules
  Each regex → return TOKEN.
  Examples are given; fill in the rest.
 ------------------------------*/
%%
[ \t\r]+        { /* skip spaces" ", tabs"\t", OR carriage returns "\r"... which appear one or more times "+" */ }

 /* TODO: Increment line_number whenever there is a newline character */
\n+                 { line_number += yyleng;}

 /* TODO: Add rules for quoted ethnicity/race phrases */
\"[Ww]hite,{WSP}[Nn]on-[Hh]ispanic\"                     { return WHITE; }
\"[Bb]lack,{WSP}[Nn]on-[Hh]ispanic\"                     { return BLACK; }
\"[Aa]sian,{WSP}[Nn]on-[Hh]ispanic\"                     { return ASIAN; }
\"[Aa]merican{WSP}+[Ii]ndian[ _]\/[ _][Aa]laska{WSP}+[Nn]ative,{WSP}[Nn]on-[Hh]ispanic\" { return NATIVE_AMERICAN; }
\"[Nn]ative{WSP}+[Hh]awaiian{WSP}\/{WSP}[Oo]ther{WSP}+[Pp]acific{WSP}+[Ii]slander,{WSP}[Nn]on-[Hh]ispanic{WSP}\" { return PACIFIC_ISLANDER; }
\"[Mm]ultiple\/[Oo]ther,{WSP}[Nn]on-[Hh]ispanic\"   { return MULTIPLE_OTHER; }
\"[Mm]ultiple\" {return MULTIPLE_OTHER; }
\"[Hh]ispanic\/[Ll]atino\"                                { return HISPANIC; }


 /* TODO: Add rules for Age buckets (0-9 Years ... 80+ Years) */
0{WSP}-{WSP}9[ ]+[Yy]ears          { return AGE_0X; }
10{WSP}-{WSP}19[ ]+[Yy]ears         { return AGE_1X; }
20{WSP}-{WSP}39[ ]+[Yy]ears         { return AGE_2X; }

40{WSP}-{WSP}49[ ]+[Yy]ears         { return AGE_4X; }
50{WSP}-{WSP}59[ ]+[Yy]ears         { return AGE_5X; }
60{WSP}-{WSP}69[ ]+[Yy]ears         { return AGE_6X; }
70{WSP}-{WSP}79[ ]+[Yy]ears         { return AGE_7X; }
80\+[ ]*[Yy]ears                    { return AGE_8X; }


 /* TODO: Add rules for Case Status (Laboratory-confirmed, Probable) */
[Ll]aboratory-confirmed[ ][Cc]ase       {return LABORATORY; }
[Pp]robable[ ][Cc]ase                   {return PROBABLE; }



 /* Example: simple words (case-insensitive first letter) */
[Yy]es              { return YES; }
[Nn]o               { return NO; }

 /* TODO: Add rule for Date */
{YEAR}\/{MONTH}\/{DAY}             { return DATE; }

 /* TODO: Add rule for "Unknown" → UNKNOWN_VALUE */
[Uu]nknown          {return UNKNOWN_VALUE; }


 /* TODO: Add rule for "Missing" → MISSING */
[Mm]issing          {return MISSING; }



 /* TODO: Add rules for Gender (Male, Female, Other) */
[Ff]emale           {return FEMALE; }
[Mm]ale             {return MALE; }
[Oo]ther            {return OTHER; }

 /* Example: punctuation */
","                 { return SEPARATOR; }






 /* End-of-file */
<<EOF>>         { return EOF_TOKEN; }

 /* Anything else → unknown */
.               { return UNKNOWN_TOKEN; }

%%



