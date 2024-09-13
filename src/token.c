// ***************************************************************************************
//    Project: Dang Compiler -> https://github.com/dezashibi-c/dang
//    File: token.c
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
// *  Description: token struct and related functionalities
// ***************************************************************************************

#include "token.h"

string tostr_TokenType(TokenType enum_item)
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
            tostr_enum_scase(TOK_MINUS);
            tostr_enum_scase(TOK_BANG);
            tostr_enum_scase(TOK_ASTERISK);
            tostr_enum_scase(TOK_SLASH);
            tostr_enum_scase(TOK_LT);
            tostr_enum_scase(TOK_GT);
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

TokenType is_keyword(DCStringView* text)
{
    if (dc_sv_str_eq((*text), "fn"))
        return TOK_FUNCTION;
    else if (dc_sv_str_eq((*text), "let"))
        return TOK_LET;
    else
        return TOK_IDENT;
}

Token* token_make(TokenType type, string str, usize start, usize len)
{
    if (type != TOK_EOF && (!str || start >= strlen(str)))
    {
        return NULL;
    }

    // Allocate memory for the token
    Token* token = (Token*)malloc(sizeof(Token));
    if (!token) return NULL; // Handle memory allocation failure

    token->text = dc_sv_create(str, start, len);
    token->type = type;

    return token;
}

void token_free(Token* t)
{
    dc_sv_free(&t->text);
}
