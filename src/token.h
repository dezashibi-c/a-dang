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

dc_def_enum(TokenType,

            TOK_ILLEGAL, TOK_EOF,

            TOK_IDENT, TOK_INT,

            TOK_ASSIGN, TOK_PLUS,

            TOK_COMMA, TOK_SEMICOLON, TOK_NEWLINE,

            TOK_LPAREN, TOK_RPAREN, TOK_LBRACE, TOK_RBRACE,

            TOK_FUNCTION, TOK_LET

);

typedef struct
{
    TokenType type;
    string text;
} Token;

static string tostr_TokenType(TokenType enum_item)
{
    {
        switch (enum_item)
        {
            tostr_enum_scase(TOK_ILLEGAL);
            tostr_enum_scase(TOK_EOF);
            tostr_enum_scase(TOK_IDENT);
            tostr_enum_scase(TOK_INT);
            tostr_enum_scase(TOK_ASSIGN);
            tostr_enum_scase(TOK_PLUS);
            tostr_enum_scase(TOK_COMMA);
            tostr_enum_scase(TOK_SEMICOLON);
            tostr_enum_scase(TOK_NEWLINE);
            tostr_enum_scase(TOK_LPAREN);
            tostr_enum_scase(TOK_RPAREN);
            tostr_enum_scase(TOK_LBRACE);
            tostr_enum_scase(TOK_RBRACE);
            tostr_enum_scase(TOK_FUNCTION);
            tostr_enum_scase(TOK_LET);
        };

        return NULL;
    }
}

Token* token_make(TokenType type);
Token* token_make_from_char(TokenType type, byte c);
Token* token_make_from_string(TokenType type, string str);

#endif // DANG_TOKEN_H
