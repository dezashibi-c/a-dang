// ***************************************************************************************
//    Project: Dang Compiler -> https://github.com/dezashibi-c/dang
//    File: parser.h
//    Date: 2024-09-16
//    Author: Navid Dezashibi
//    Contact: navid@dezashibi.com
//    Website: https://dezashibi.com | https://github.com/dezashibi
//    License:
//     Please refer to the LICENSE file, repository or website for more
//     information about the licensing of this work. If you have any questions
//     or concerns, please feel free to contact me at the email address provided
//     above.
// ***************************************************************************************
// *  Description: parsing structs and related functionalities
// ***************************************************************************************

#ifndef DANG_PARSER_H
#define DANG_PARSER_H

#include "ast.h"
#include "scanner.h"

typedef struct Parser Parser;

typedef ResultDNode (*ParsePrefixFn)(Parser*);
typedef ResultDNode (*ParseInfixFn)(Parser*, DNode*);

typedef enum
{
    PREC_LOWEST,
    PREC_EQUALS,
    PREC_CMP,
    PREC_SUM,
    PREC_PROD,
    PREC_PREFIX,
    PREC_CALL
} Precedence;

typedef enum
{
    LOC_BODY,
    LOC_BLOCK,
    LOC_CALL,

    LOC_MAX,
} ParserStatementLoc;

typedef struct Parser
{
    Scanner* scanner;

    DToken* current_token;
    DToken* peek_token;

    DCDynArr errors;

    DTokenType terminators[LOC_MAX][5];
    ParserStatementLoc loc;

    ParsePrefixFn parse_prefix_fns[DN__MAX];
    ParseInfixFn parse_infix_fns[DN__MAX];
} Parser;

DCResultVoid parser_init(Parser* p, Scanner* s);
DCResultVoid parser_free(Parser* p);
ResultDNode parser_parse_program(Parser* p);
void parser_log_errors(Parser* p);

#endif // DANG_PARSER_H
