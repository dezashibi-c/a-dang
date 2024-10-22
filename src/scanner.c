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
// *  Description: DScanner struct and related functionalities
// ***************************************************************************************

#include "scanner.h"

// ***************************************************************************************
// * PRIVATE FUNCTIONS
// ***************************************************************************************

#define is_letter(C) (('a' <= (C) && (C) <= 'z') || ('A' <= (C) && (C) <= 'Z') || (C) == '_')

#define is_digit(C) ('0' <= (C) && (C) <= '9')

#define is_whitespace(C) ((C) == ' ' || (C) == '\t' || (C) == '\r')

static void read_char(DScanner* s)
{
    if (s->read_pos >= strlen(s->input))
        s->c = 0;
    else
        s->c = s->input[s->read_pos];

    s->pos = s->read_pos;
    s->read_pos++;
}

static char peek(DScanner* s)
{
    if (s->read_pos >= strlen(s->input)) return 0;

    return s->input[s->read_pos];
}

static void skip_whitespace(DScanner* s)
{
    while (is_whitespace(s->c)) read_char(s);
}

static ResTok extract_identifier(DScanner* s)
{
    DC_RES2(ResTok);

    usize start = s->pos;

    while (is_letter(s->c)) read_char(s);

    usize len = s->pos - start;

    dc_try_fail(token_create(TOK_IDENT, s->input, start, len));

    DTok* t = &dc_unwrap();

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

    dc_ret();
}

static ResTok extract_number(DScanner* s)
{
    usize start = s->pos;

    while (is_digit(s->c)) read_char(s);

    usize len = s->pos - start;

    return token_create(TOK_INT, s->input, start, len);
}

// ***************************************************************************************
// * PUBLIC FUNCTIONS
// ***************************************************************************************

DCResVoid dang_scanner_init(DScanner* s, const string input)
{
    DC_RES_void();

    if (!input)
    {
        dc_dbg_log("Cannot initialize scanner with null input");

        dc_ret_e(dc_e_code(NV), "cannot initialize scanner with null input");
    }

    s->pos = 0;
    s->read_pos = 0;
    s->input = input;

    read_char(s);

    dc_ret();
}

ResTok dang_scanner_next_token(DScanner* s)
{
    DC_RES2(ResTok);

    skip_whitespace(s);

    switch (s->c)
    {
        case '=':
            if (peek(s) == '=')
            {
                dc_try_fail(token_create(TOK_EQ, s->input, s->pos, 2));
                read_char(s);
            }
            else
            {
                dc_try_fail(token_create(TOK_ASSIGN, s->input, s->pos, 1));
            }
            break;

        case ';':
            dc_try_fail(token_create(TOK_SEMICOLON, s->input, s->pos, 1));
            break;

        case '(':
            dc_try_fail(token_create(TOK_LPAREN, s->input, s->pos, 1));
            break;

        case ')':
            dc_try_fail(token_create(TOK_RPAREN, s->input, s->pos, 1));
            break;

        case ',':
            dc_try_fail(token_create(TOK_COMMA, s->input, s->pos, 1));
            break;

        case '+':
            dc_try_fail(token_create(TOK_PLUS, s->input, s->pos, 1));
            break;

        case '-':
            dc_try_fail(token_create(TOK_MINUS, s->input, s->pos, 1));
            break;

        case '!':
            if (peek(s) == '=')
            {
                dc_try_fail(token_create(TOK_NEQ, s->input, s->pos, 2));
                read_char(s);
            }
            else
            {
                dc_try_fail(token_create(TOK_BANG, s->input, s->pos, 1));
            }
            break;

        case '/':
            dc_try_fail(token_create(TOK_SLASH, s->input, s->pos, 1));
            break;

        case '*':
            dc_try_fail(token_create(TOK_ASTERISK, s->input, s->pos, 1));
            break;

        case '"':
        case '\'':
        {
            char starter = s->c;
            read_char(s); // bypass " or '

            usize start = s->pos;
            usize len;

            DTokType t = TOK_STRING;

            while (true)
            {
                if (s->c == starter)
                {
                    len = s->pos - start;

                    break;
                }
                else if (s->c == 0)
                {
                    len = 1;
                    start--;
                    t = TOK_ILLEGAL;
                    break;
                }
                read_char(s);
            }

            dc_try_fail(token_create(t, s->input, start, len));
            break;
        }

        case '$':
        {
            if (is_digit(peek(s)))
            {
                read_char(s); // bypass '$'

                usize start = s->pos;
                while (is_digit(peek(s))) read_char(s);
                usize len = s->pos - start + 1;

                dc_try_fail(token_create(TOK_IDENT, s->input, start, len));
            }
            else if (peek(s) == '"')
            {
                read_char(s); // bypass '$'
                read_char(s); // bypass '"'

                usize start = s->pos;
                usize len;

                DTokType t = TOK_IDENT;

                while (true)
                {
                    if (s->c == '"')
                    {
                        len = s->pos - start;

                        break;
                    }
                    else if (s->c == 0 || s->c == '\n')
                    {
                        len = 1;
                        start--;
                        t = TOK_ILLEGAL;
                        break;
                    }
                    read_char(s);
                }

                dc_try_fail(token_create(t, s->input, start, len));
            }
            else if (peek(s) == '{')
            {
                // ${
                dc_try_fail(token_create(TOK_DOLLAR_LBRACE, s->input, s->pos, 2));
                read_char(s);
            }
            else
            {
                dc_try_fail(token_create(TOK_ILLEGAL, s->input, s->pos, 1));
            }
            break;
        }

        case '<':
            dc_try_fail(token_create(TOK_LT, s->input, s->pos, 1));
            break;

        case '>':
            dc_try_fail(token_create(TOK_GT, s->input, s->pos, 1));
            break;

        case '{':
            dc_try_fail(token_create(TOK_LBRACE, s->input, s->pos, 1));
            break;

        case '}':
            dc_try_fail(token_create(TOK_RBRACE, s->input, s->pos, 1));
            break;

        case '[':
            dc_try_fail(token_create(TOK_LBRACKET, s->input, s->pos, 1));
            break;

        case ']':
            dc_try_fail(token_create(TOK_RBRACKET, s->input, s->pos, 1));
            break;

        case '\n':
            dc_try_fail(token_create(TOK_NEWLINE, s->input, s->pos, 1));
            break;

        case '\0':
            dc_try_fail(token_create(TOK_EOF, s->input, s->pos, 0));
            break;

        default:
        {
            if (is_letter(s->c))
            {
                dc_try_fail(extract_identifier(s));
                dc_ret();
            }
            else if (is_digit(s->c))
            {
                dc_try_fail(extract_number(s));
                dc_ret();
            }
            else
            {
                dc_try_fail(token_create(TOK_ILLEGAL, s->input, s->pos, 1));
            }
        }
        break;
    }

    if (dc_unwrap().type == TOK_ILLEGAL)
    {
        dc_dbg_log("DScanner error - illegal character at '" DCPRIsv "'", dc_sv_fmt(dc_unwrap().text));
        dc_ret_ea(-1, "DScanner error - illegal character at '" DCPRIsv "'", dc_sv_fmt(dc_unwrap().text));
    }

    read_char(s);

    dc_ret();
}