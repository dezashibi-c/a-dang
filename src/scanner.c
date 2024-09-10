// ***************************************************************************************
//    Project: Dang Compiler -> https://github.com/dezashibi-c/dang
//    File: scanner.c
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
// *  Description: Scanner struct and related functionalities
// ***************************************************************************************

#include "scanner.h"

// ***************************************************************************************
// * PRIVATE FUNCTIONS
// ***************************************************************************************

static void read_char(Scanner* s)
{
    if (s->read_pos >= strlen(s->input))
        s->c = 0;
    else
        s->c = s->input[s->read_pos];

    s->pos = s->read_pos;
    s->read_pos++;
}

// ***************************************************************************************
// * PUBLIC FUNCTIONS
// ***************************************************************************************

void scanner_init(Scanner* s, const string input)
{
    s->pos = 0;
    s->read_pos = 0;
    s->input = input;
    dc_dynarr_init(&s->tokens);

    read_char(s);
}

void scanner_free(Scanner* s)
{
    dc_dynarr_free(&s->tokens);
}

Token* scanner_next_token(Scanner* s)
{
    Token* token;

    switch (s->c)
    {
        case '=':
            token = token_make_from_char(TOK_ASSIGN, s->c);
            break;

        case ';':
            token = token_make_from_char(TOK_SEMICOLON, s->c);
            break;

        case '(':
            token = token_make_from_char(TOK_LPAREN, s->c);
            break;

        case ')':
            token = token_make_from_char(TOK_RPAREN, s->c);
            break;

        case ',':
            token = token_make_from_char(TOK_COMMA, s->c);
            break;

        case '+':
            token = token_make_from_char(TOK_PLUS, s->c);
            break;

        case '{':
            token = token_make_from_char(TOK_LBRACE, s->c);
            break;

        case '}':
            token = token_make_from_char(TOK_RBRACE, s->c);
            break;

        case '\n':
            token = token_make_from_char(TOK_NEWLINE, s->c);
            break;

        case '\0':
            token = token_make_from_char(TOK_EOF, s->c);
            break;

        default:
            token = token_make_from_char(TOK_ILLEGAL, s->c);
            break;
    }

    read_char(s);

    dc_dynarr_add(&s->tokens, dc_dynval_lit(voidptr, (void*)token));
    return token;
}