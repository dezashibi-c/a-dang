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

typedef struct DParser DParser;

typedef ResNode (*ParsePrefixFn)(DParser*);
typedef ResNode (*ParseInfixFn)(DParser*, DNode*);

typedef enum
{
    PREC_LOWEST,
    PREC_EQUALS,
    PREC_CMP,
    PREC_SUM,
    PREC_PROD,
    PREC_PREFIX,
    PREC_CALL,
    PREC_INDEX,
} Precedence;

typedef enum
{
    LOC_BODY,
    LOC_BLOCK,
    LOC_CALL,
    LOC_ARRAY,
} DParserStatementLoc;

typedef struct DParser
{
    DScanner* scanner;

    DTok current_token;
    DTok peek_token;

    DCDynArr errors;

    DParserStatementLoc loc;

    ParsePrefixFn parse_prefix_fns[DN__MAX];
    ParseInfixFn parse_infix_fns[DN__MAX];
} DParser;

#define dang_parser_has_error(P) ((P)->errors.count != 0)

DCResVoid dang_parser_init(DParser* p, DScanner* s);
DCResVoid dang_parser_free(DParser* p);
ResNode dang_parser_parse_program(DParser* p);
void dang_parser_log_errors(DParser* p);

#endif // DANG_PARSER_H
