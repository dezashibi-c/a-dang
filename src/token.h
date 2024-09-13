// ***************************************************************************************
//    Project: Dang Compiler -> https://github.com/dezashibi-c/dang
//    File: token.h
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
// *  Description: Token `enum` and token related functionalities
// ***************************************************************************************

#ifndef DANG_TOKEN_H
#define DANG_TOKEN_H

#include "dcommon/dcommon.h"

typedef enum
{
    TOK_ILLEGAL,
    TOK_EOF,

    TOK_IDENT,
    TOK_INT,

    TOK_ASSIGN,
    TOK_PLUS,
    TOK_MINUS,
    TOK_BANG,
    TOK_ASTERISK,
    TOK_SLASH,

    TOK_LT,
    TOK_GT,
    TOK_EQ,
    TOK_NEQ,

    TOK_COMMA,
    TOK_SEMICOLON,
    TOK_NEWLINE,

    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACE,
    TOK_RBRACE,

    TOK_FUNCTION,
    TOK_LET,
    TOK_TRUE,
    TOK_FALSE,
    TOK_IF,
    TOK_ELSE,
    TOK_RET

} DangTokenType;

typedef struct
{
    DangTokenType type;
    DCStringView text;
} Token;

string tostr_DangTokenType(DangTokenType enum_item);
DangTokenType is_keyword(DCStringView* text);

Token* token_make(DangTokenType type, string str, usize start, usize len);
void token_free(Token* t);

#endif // DANG_TOKEN_H
