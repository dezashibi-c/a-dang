// ***************************************************************************************
//    Project: Dang Compiler -> https://github.com/dezashibi-c/dang
//    File: scanner.h
//    Date: 2024-09-10
//    Author: Navid Dezashibi
//    Contact: navid@dezashibi.com
//    Website: https://dezashibi.com | https://github.com/dezashibi
//    License:
//     Please refer to the LICENSE file, repository or website for more
//     information about the licensing of this work. If you have any questions
//     or concerns, please feel free to contact me at the email address provided
//     above.
// ***************************************************************************************
// *  Description: DScanner struct and related functionalities
// ***************************************************************************************

#ifndef DANG_SCANNER_H
#define DANG_SCANNER_H

#include "types.h"

#include "token.h"

typedef struct
{
    string input;
    usize pos;
    usize read_pos;
    char c;
} DScanner;

DCResVoid dang_scanner_init(DScanner* s, const string input);
ResTok dang_scanner_next_token(DScanner* s);

#endif // DANG_SCANNER_H
