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

#define is_letter(C) (('a' <= (C) && (C) <= 'z') || ('A' <= (C) && (C) <= 'Z') || (C) == '_')

#define is_digit(C) ('0' <= (C) && (C) <= '9')

#define is_whitespace(C) ((C) == ' ' || (C) == '\t' || (C) == '\r')

static void read_char(Scanner* s)
{
    if (s->read_pos >= strlen(s->input))
        s->c = 0;
    else
        s->c = s->input[s->read_pos];

    s->pos = s->read_pos;
    s->read_pos++;
}

static char peek(Scanner* s)
{
    if (s->read_pos >= strlen(s->input)) return 0;

    return s->input[s->read_pos];
}

static void skip_whitespace(Scanner* s)
{
    while (is_whitespace(s->c)) read_char(s);
}

static ResultToken extract_identifier(Scanner* s)
{
    DC_RES2(ResultToken);

    usize start = s->pos;

    while (is_letter(s->c)) read_char(s);

    usize len = s->pos - start;

    dc_try_fail(token_create(TOK_IDENT, s->input, start, len));

    DToken* t = dc_res_val();

    switch (s->input[start])
    {
        case 'f':
        case 'l':
        case 't':
        case 'i':
        case 'e':
        case 'r':
            t->type = is_keyword(&t->text);
            break;

        default:
            t->type = TOK_IDENT;
            break;
    }

    dc_res_ret_ok(t);
}

static ResultToken extract_number(Scanner* s)
{
    DC_RES2(ResultToken);

    usize start = s->pos;

    while (is_digit(s->c)) read_char(s);

    usize len = s->pos - start;

    dc_try_fail(token_create(TOK_INT, s->input, start, len));

    DToken* t = dc_res_val();

    dc_res_ret_ok(t);
}

static DC_DV_FREE_FN_DECL(custom_token_free)
{
    DC_RES_void();

    switch (_value->type)
    {
        case dc_dvt(voidptr):
            return token_free((DToken*)(dc_dv_as(*_value, voidptr)));
            break;

        default:
            break;
    }

    dc_res_ret();
}

// ***************************************************************************************
// * PUBLIC FUNCTIONS
// ***************************************************************************************

DCResultVoid dang_scanner_init(Scanner* s, const string input)
{
    DC_RES_void();

    s->pos = 0;
    s->read_pos = 0;
    s->input = input;
    dc_try(dc_da_init(&s->tokens, custom_token_free));

    read_char(s);

    dc_res_ret();
}

DCResultVoid dang_scanner_free(Scanner* s)
{
    return dc_da_free(&s->tokens);
}

ResultToken dang_scanner_next_token(Scanner* s)
{
    DC_RES2(ResultToken);

    DToken* token;

    skip_whitespace(s);

    switch (s->c)
    {
        case '=':
            if (peek(s) == '=')
            {
                dc_try_fail(token_create(TOK_EQ, s->input, s->pos, 2));
                token = dc_res_val();
                read_char(s);
            }
            else
            {
                dc_try_fail(token_create(TOK_ASSIGN, s->input, s->pos, 1));
                token = dc_res_val();
            }
            break;

        case ';':
            dc_try_fail(token_create(TOK_SEMICOLON, s->input, s->pos, 1));
            token = dc_res_val();
            break;

        case '(':
            dc_try_fail(token_create(TOK_LPAREN, s->input, s->pos, 1));
            token = dc_res_val();
            break;

        case ')':
            dc_try_fail(token_create(TOK_RPAREN, s->input, s->pos, 1));
            token = dc_res_val();
            break;

        case ',':
            dc_try_fail(token_create(TOK_COMMA, s->input, s->pos, 1));
            token = dc_res_val();
            break;

        case '+':
            dc_try_fail(token_create(TOK_PLUS, s->input, s->pos, 1));
            token = dc_res_val();
            break;

        case '-':
            dc_try_fail(token_create(TOK_MINUS, s->input, s->pos, 1));
            token = dc_res_val();
            break;

        case '!':
            if (peek(s) == '=')
            {
                dc_try_fail(token_create(TOK_NEQ, s->input, s->pos, 2));
                token = dc_res_val();
                read_char(s);
            }
            else
            {
                dc_try_fail(token_create(TOK_BANG, s->input, s->pos, 1));
                token = dc_res_val();
            }
            break;

        case '/':
            dc_try_fail(token_create(TOK_SLASH, s->input, s->pos, 1));
            token = dc_res_val();
            break;

        case '*':
            dc_try_fail(token_create(TOK_ASTERISK, s->input, s->pos, 1));
            token = dc_res_val();
            break;

        case '$':
        {
            if (is_digit(peek(s)))
            {
                read_char(s); // bypass '$'

                usize start = s->pos;
                while (is_digit(peek(s))) read_char(s);
                usize len = s->pos - start + 1;

                dc_try_fail(token_create(TOK_IDENT, s->input, start, len));
                token = dc_res_val();
            }
            else if (peek(s) == '"')
            {
                read_char(s); // bypass '$'
                read_char(s); // bypass '"'

                usize start = s->pos;
                usize len;

                while (true)
                {
                    if (s->c == '"')
                    {
                        len = s->pos - start;
                        read_char(s); // bypass ending '"'
                        break;
                    }
                    else if (s->c == 0 || s->c == '\n')
                    {
                        token_create(TOK_ILLEGAL, s->input, start, s->pos - start);
                        break;
                    }
                    read_char(s);
                }

                dc_try_fail(token_create(TOK_IDENT, s->input, start, len));
                token = dc_res_val();
            }
            else if (peek(s) == '{')
            {
                // ${
                dc_try_fail(token_create(TOK_DOLLAR_LBRACE, s->input, s->pos, 2));
                token = dc_res_val();
                read_char(s);
            }
            else
            {
                dc_try_fail(token_create(TOK_ILLEGAL, s->input, s->pos, 1));
                token = dc_res_val();
            }
            break;
        }

        case '<':
            dc_try_fail(token_create(TOK_LT, s->input, s->pos, 1));
            token = dc_res_val();
            break;

        case '>':
            dc_try_fail(token_create(TOK_GT, s->input, s->pos, 1));
            token = dc_res_val();
            break;

        case '{':
            dc_try_fail(token_create(TOK_LBRACE, s->input, s->pos, 1));
            token = dc_res_val();
            break;

        case '}':
            dc_try_fail(token_create(TOK_RBRACE, s->input, s->pos, 1));
            token = dc_res_val();
            break;

        case '\n':
            dc_try_fail(token_create(TOK_NEWLINE, s->input, s->pos, 1));
            token = dc_res_val();
            break;

        case '\0':
            dc_try_fail(token_create(TOK_EOF, s->input, s->pos, 0));
            token = dc_res_val();
            break;

        default:
        {
            if (is_letter(s->c))
            {
                dc_try_fail(extract_identifier(s));
                token = dc_res_val();
                dc_da_push(&s->tokens, dc_dva(voidptr, token));
                dc_res_ret_ok(token);
            }
            else if (is_digit(s->c))
            {
                dc_try_fail(extract_number(s));
                token = dc_res_val();
                dc_da_push(&s->tokens, dc_dva(voidptr, token));
                dc_res_ret_ok(token);
            }
            else
            {
                dc_try_fail(token_create(TOK_ILLEGAL, s->input, s->pos, 1));
                token = dc_res_val();
            }
        }
        break;
    }


    read_char(s);

    dc_da_push(&s->tokens, dc_dva(voidptr, token));

    if (token->type == TOK_ILLEGAL)
        dc_res_ret_ea(-1, "Scanner error - illegal character at '" DCPRIsv "'", dc_sv_fmt(token->text));

    dc_res_ret_ok(token);
}