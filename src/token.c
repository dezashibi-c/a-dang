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

string tostr_DangTokenType(DangTokenType dtt)
{
    switch (dtt)
    {
        dc_str_case(TOK_ILLEGAL);
        dc_str_case(TOK_EOF);
        dc_str_case(TOK_IDENT);
        dc_str_case(TOK_INT);
        dc_str_case(TOK_ASSIGN);
        dc_str_case(TOK_PLUS);
        dc_str_case(TOK_MINUS);
        dc_str_case(TOK_BANG);
        dc_str_case(TOK_ASTERISK);
        dc_str_case(TOK_SLASH);
        dc_str_case(TOK_LT);
        dc_str_case(TOK_GT);
        dc_str_case(TOK_EQ);
        dc_str_case(TOK_NEQ);
        dc_str_case(TOK_COMMA);
        dc_str_case(TOK_SEMICOLON);
        dc_str_case(TOK_NEWLINE);
        dc_str_case(TOK_LPAREN);
        dc_str_case(TOK_RPAREN);
        dc_str_case(TOK_LBRACE);
        dc_str_case(TOK_RBRACE);
        dc_str_case(TOK_FUNCTION);
        dc_str_case(TOK_LET);
        dc_str_case(TOK_TRUE);
        dc_str_case(TOK_FALSE);
        dc_str_case(TOK_IF);
        dc_str_case(TOK_ELSE);
        dc_str_case(TOK_RET);
    };

    return NULL;
}

DangTokenType is_keyword(DCStringView* text)
{
    if (dc_sv_str_eq((*text), "fn"))
        return TOK_FUNCTION;

    else if (dc_sv_str_eq((*text), "let"))
        return TOK_LET;

    else if (dc_sv_str_eq((*text), "true"))
        return TOK_TRUE;

    else if (dc_sv_str_eq((*text), "false"))
        return TOK_FALSE;

    else if (dc_sv_str_eq((*text), "if"))
        return TOK_IF;

    else if (dc_sv_str_eq((*text), "else"))
        return TOK_ELSE;

    else if (dc_sv_str_eq((*text), "return"))
        return TOK_RET;

    else
        return TOK_IDENT;
}

Token* token_make(DangTokenType type, string str, usize start, usize len)
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
