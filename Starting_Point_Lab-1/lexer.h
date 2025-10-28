//*****************************************************************************
// purpose: header file for Lab 1 assignment, defines tokens
// version: Fall 2024
//  author: Ed Swan / Joe Crumpton
//
// Speak to the TA or Instructor if you think that you need to modify this file.
//
//*****************************************************************************
#ifndef LEXER_H
#define LEXER_H

// include standard IO streams
#include <stdio.h>

// List of token codes

// Data Types
#define DATE             10  // date - year/month/day

// Punctuation
#define SEPARATOR        20  // commas in a CSV

// Generic data values
//    can be used in several different data fields
#define YES              30
#define NO               31
#define UNKNOWN_VALUE    32
#define MISSING          33

// Data values used in specific fields
#define LABORATORY       40
#define PROBABLE         41

#define MALE             50
#define FEMALE           51
#define OTHER            52

#define AGE_0X           60
#define AGE_1X           61
#define AGE_2X           62
#define AGE_4X           64
#define AGE_5X           65
#define AGE_6X           66
#define AGE_7X           67
#define AGE_8X           68


#define HISPANIC         70   // ethnicity
// the following "race" categories imply non-hispanic ethnicity
#define NATIVE_AMERICAN  71   
#define ASIAN            72
#define BLACK            73
#define PACIFIC_ISLANDER 74
#define WHITE            75
#define MULTIPLE_OTHER   76

#define EOF_TOKEN        -1  // end of file
#define UNKNOWN_TOKEN    99  // unknown lexeme

#endif

