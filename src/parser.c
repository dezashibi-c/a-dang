// ***************************************************************************************
//    Project: Dang Compiler -> https://github.com/dezashibi-c/dang
//    File: parser.c
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

#include "parser.h"

// ***************************************************************************************
// * PRIVATE FUNCTIONS
// ***************************************************************************************

static void next_token(Parser* p)
{
    p->current_token = p->peek_token;
    p->peek_token = scanner_next_token(p->scanner);
}

// ***************************************************************************************
// * PUBLIC FUNCTIONS
// ***************************************************************************************

void parser_init(Parser* p, Scanner* s)
{
    p->scanner = s;

    p->current_token = NULL;
    p->peek_token = NULL;

    next_token(p);
    next_token(p);
}

DNodeProgram parser_parse_program(Parser* p)
{

    return (DNodeProgram){.type = DN__UNKNOWN};
}
