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

string tostr_DTokType(DTokType dtt)
{
    switch (dtt)
    {
        dc_str_case(TOK_ILLEGAL);
        dc_str_case(TOK_EOF);
        dc_str_case(TOK_IDENT);
        dc_str_case(TOK_INT);
        dc_str_case(TOK_STRING);
        dc_str_case(TOK_ASSIGN);
        dc_str_case(TOK_PLUS);
        dc_str_case(TOK_MINUS);
        dc_str_case(TOK_BANG);
        dc_str_case(TOK_ASTERISK);
        dc_str_case(TOK_SLASH);
        dc_str_case(TOK_DOLLAR);
        dc_str_case(TOK_DOLLAR_LBRACE);
        dc_str_case(TOK_COLON);
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
        dc_str_case(TOK_LBRACKET);
        dc_str_case(TOK_RBRACKET);
        dc_str_case(TOK_FUNCTION);
        dc_str_case(TOK_LET);
        dc_str_case(TOK_TRUE);
        dc_str_case(TOK_FALSE);
        dc_str_case(TOK_IF);
        dc_str_case(TOK_ELSE);
        dc_str_case(TOK_RET);

        default:
            break;
    };

    return NULL;
}

DTokType is_keyword(DCStringView* text)
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

ResTok token_create(DTokType type, string str, usize start, usize len)
{
    DC_RES2(ResTok);

    if (type != TOK_EOF && (!str || start + len > strlen(str)))
    {
        dc_dbg_log("Only TOK_EOF can be created with NULL string");

        dc_ret_e(1, "Only TOK_EOF can be created with NULL string");
    }

    dc_ret_ok(((DTok){.text = dc_sv(str, start, len), .type = type}));
}
