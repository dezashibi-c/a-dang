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

typedef struct
{
    Scanner* scanner;

    Token* current_token;
    Token* peek_token;
} Parser;

void parser_init(Parser* p, Scanner* s);
DNode* parser_parse_program(Parser* p);

#endif // DANG_PARSER_H
