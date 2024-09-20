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

#define current_token_is(P, TYPE) ((P)->current_token->type == TYPE)
#define current_token_is_not(P, TYPE) ((P)->current_token->type != TYPE)

#define current_token_is_end_of_stmt(P)                                        \
    (current_token_is(P, TOK_SEMICOLON) || current_token_is(P, TOK_NEWLINE) || \
     current_token_is(P, TOK_EOF))

#define peek_token_is(P, TYPE) ((P)->peek_token->type == TYPE)
#define peek_token_is_not(P, TYPE) ((P)->peek_token->type != TYPE)

static void next_token(Parser* p)
{
    p->current_token = p->peek_token;
    p->peek_token = scanner_next_token(p->scanner);
}

static void add_token_error(Parser* p, DangTokenType type)
{
    string err;
    dc_sprintf(&err, "expected next token to be %s, got %s instead.",
               tostr_DangTokenType(type),
               tostr_DangTokenType(p->peek_token->type));
    dc_da_push(&(p->errors), dc_dv(string, err));
}

static bool move_if_peek_token_is(Parser* p, DangTokenType type)
{
    if (peek_token_is(p, type))
    {
        next_token(p);
        return true;
    }

    add_token_error(p, type);
    return false;
}

static DNode* parse_let_statement(Parser* p)
{
    DNode* stmt = dnode_create(DN_LET_STATEMENT, p->current_token, true);

    if (!move_if_peek_token_is(p, TOK_IDENT)) return NULL;

    DNode* name = dnode_create(DN_IDENTIFIER, p->current_token, false);
    dc_da_push(&stmt->children, dc_dv(voidptr, name));

    while (!current_token_is_end_of_stmt(p)) next_token(p);

    return stmt;
}

static DNode* parse_return_statement(Parser* p)
{
    DNode* stmt = dnode_create(DN_RETURN_STATEMENT, p->current_token, false);

    next_token(p);

    while (!current_token_is_end_of_stmt(p)) next_token(p);

    return stmt;
}

static DNode* parse_statement(Parser* p)
{
    switch (p->current_token->type)
    {
        case TOK_LET:
            return parse_let_statement(p);

        case TOK_RET:
            return parse_return_statement(p);

        default:
            return NULL;
    }
}

// ***************************************************************************************
// * PUBLIC FUNCTIONS
// ***************************************************************************************

void parser_init(Parser* p, Scanner* s)
{
    p->scanner = s;

    p->current_token = NULL;
    p->peek_token = NULL;

    dc_da_init(&(p->errors), NULL);

    next_token(p);
    next_token(p);
}

DNode* parser_parse_program(Parser* p)
{
    (void)p;

    DNode* program = dnode_create(DN_PROGRAM, NULL, true);

    while (p->current_token->type != TOK_EOF)
    {
        DNode* stmt = parse_statement(p);
        if (stmt != NULL) dc_da_push(&program->children, dc_dv(voidptr, stmt));

        next_token(p);
    }

    return program;
}

void parser_log_errors(Parser* p)
{
    dc_da_for(p->errors)
    {
        string error = dc_da_get_as(&(p->errors), _idx, string);
        dc_log("%s", error);
    }
}
