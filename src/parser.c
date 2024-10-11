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

#include "common.h"

// ***************************************************************************************
// * PRIVATE FUNCTIONS DECLARATIONS
// ***************************************************************************************

static ResultDNode parse_expression(DParser* p, Precedence precedence);
static ResultDNode parse_block_statement(DParser* p);
static ResultDNode parse_statement(DParser* p);

// ***************************************************************************************
// * PRIVATE FUNCTIONS
// ***************************************************************************************

#define token_is(TOKEN, TYPE) ((TOKEN) && TOKEN->type == TYPE)
#define token_is_not(TOKEN, TYPE) ((TOKEN) && TOKEN->type != TYPE)

#define current_token_is(P, TYPE) token_is((P)->current_token, TYPE)
#define current_token_is_not(P, TYPE) token_is_not((P)->current_token, TYPE)

#define peek_token_is(P, TYPE) token_is((P)->peek_token, TYPE)
#define peek_token_is_not(P, TYPE) token_is_not((P)->peek_token, TYPE)

#define peek_prec(P) ((P)->peek_token ? get_precedence((P)->peek_token->type) : PREC_LOWEST)
#define current_prec(P) get_precedence((P)->current_token->type)

#define dang_parser_location_preserve(P) DParserStatementLoc __dang_parser_loc_snapshot = (P)->loc
#define dang_parser_location_revert(P) (P)->loc = __dang_parser_loc_snapshot
#define dang_parser_location_set(P, LOC) (P)->loc = LOC

/**
 * expands to checking if the token is end of statement or not
 *
 * @param P is the parser pointer
 * @param TOK must be whether 'current' or 'peek' without single or double quotes
 */
#define token_is_end_of_the_statement(P, TOK)                                                                                  \
    (((P)->loc != LOC_BODY && TOK##_token_is(P, TOK_RBRACE)) ||                                                                \
     ((P)->loc != LOC_CALL && (TOK##_token_is(P, TOK_SEMICOLON) || TOK##_token_is(P, TOK_NEWLINE))))

#define try_moving_to_the_end_of_statement(P)                                                                                  \
    while (!token_is_end_of_the_statement(P, current) && current_token_is_not(P, TOK_EOF))                                     \
    dc_try_fail_temp(DCResultVoid, next_token((P)))

#define try_bypassing_all_sc_and_nls_or_fail_with(P, PRE_EXIT_ACTIONS)                                                         \
    while (current_token_is(P, TOK_SEMICOLON) || current_token_is(P, TOK_NEWLINE))                                             \
    {                                                                                                                          \
        DCResultVoid res = next_token(p);                                                                                      \
        dc_res_ret_if_err2(res, { PRE_EXIT_ACTIONS; });                                                                        \
    }

#define dang_parser_log_tokens(P)                                                                                              \
    dc_log("cur tok: %s, next tok: %s", tostr_DTokenType((P)->current_token->type),                                            \
           (P)->peek_token ? tostr_DTokenType((P)->peek_token->type) : "NULL")

#define dang_parser_dbg_log_tokens(P)                                                                                          \
    dc_dbg_log("cur tok: %s, next tok: %s", tostr_DTokenType((P)->current_token->type),                                        \
               (P)->peek_token ? tostr_DTokenType((P)->peek_token->type) : "NULL")

#define next_token_err_fmt(P, TYPE)                                                                                            \
    "expected next token to be %s, got %s instead.", tostr_DTokenType(TYPE), tostr_DTokenType(P->peek_token->type)

#define current_token_err_fmt(P, TYPE)                                                                                         \
    "expected current token to be %s, got %s instead.", tostr_DTokenType(TYPE), tostr_DTokenType(P->current_token->type)

#define unexpected_token_err_fmt(TYPE) "unexpected token '%s'.", tostr_DTokenType(TYPE)

static DCResultVoid next_token(DParser* p)
{
    DC_RES_void();

    p->current_token = p->peek_token;

    if (current_token_is(p, TOK_EOF))
    {
        p->peek_token = NULL;
        dc_res_ret();
    }

    ResultToken res = dang_scanner_next_token(p->scanner);
    dc_res_fail_if_err2(res);

    p->peek_token = dc_res_val2(res);

    dc_res_ret();
}

static void add_error(DParser* p, DCError* err)
{
    dc_da_push(&p->errors, (err->allocated ? dc_dva(string, err->message) : dc_dv(string, err->message)));
}

static DCResultVoid move_if_peek_token_is(DParser* p, DTokenType type)
{
    DC_RES_void();

    if (peek_token_is(p, type)) return next_token(p);

    dc_res_ret_ea(-2, next_token_err_fmt(p, type));
}

static Precedence get_precedence(DTokenType type)
{
    switch (type)
    {
        case TOK_EQ:
        case TOK_NEQ:
            return PREC_EQUALS;

        case TOK_LT:
        case TOK_GT:
            return PREC_CMP;

        case TOK_PLUS:
        case TOK_MINUS:
            return PREC_SUM;

        case TOK_SLASH:
        case TOK_ASTERISK:
            return PREC_PROD;

        case TOK_DOLLAR_LBRACE:
            return PREC_CALL;

        default:
            break;
    };

    return PREC_LOWEST;
}

static ResultDNode parse_illegal(DParser* p)
{
    DC_RES2(ResultDNode);

    dc_res_ret_ea(-1, "got illegal token of type: %s", tostr_DTokenType(p->current_token->type));
}

static ResultDNode parse_identifier(DParser* p)
{
    return dn_new(DN_IDENTIFIER, p->current_token, false);
}

static ResultDNode parse_string_literal(DParser* p)
{
    return dn_new(DN_STRING_LITERAL, p->current_token, false);
}

static ResultDNode parse_integer_literal(DParser* p)
{
    DC_RES2(ResultDNode);

    DCResultString str_res = dc_sv_as_cstr(&p->current_token->text);
    dc_res_ret_if_err2(str_res, { dc_res_err_dbg_log2(str_res, "Cannot retrieve string value of the current token"); });

    DCResultI64 i64_res = dc_str_to_i64(dc_res_val2(str_res));
    dc_res_ret_if_err2(i64_res, { dc_res_err_dbg_log2(i64_res, "could not parse token text to i64 number"); });

    dc_try_fail(dn_new(DN_INTEGER_LITERAL, p->current_token, true));

    DCResultVoid res = dn_val_push(dc_res_val(), i64, dc_res_val2(i64_res));
    dc_res_ret_if_err2(res, {
        dc_res_err_dbg_log2(res, "could not push the value to expression");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
    });

    dc_res_ret();
}

static ResultDNode parse_boolean_literal(DParser* p)
{
    DC_TRY_DEF2(ResultDNode, dn_new(DN_BOOLEAN_LITERAL, p->current_token, true));

    DCResultVoid res = dn_val_push(dc_res_val(), u8, (p->current_token->type == TOK_TRUE));
    dc_res_ret_if_err2(res, {
        dc_res_err_dbg_log2(res, "could not push the value to expression");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
    });

    dc_res_ret();
}

static DCResultVoid parse_function_params(DParser* p, DNode* parent_node)
{
    DC_RES_void();

    dc_try_fail(next_token(p));

    while (current_token_is_not(p, TOK_RPAREN) && current_token_is_not(p, TOK_EOF))
    {
        ResultDNode ident = parse_identifier(p);
        dc_res_ret_if_err2(ident, {});

        DCResultVoid res = dn_child_push(parent_node, dc_res_val2(ident));
        dc_res_ret_if_err2(res, dc_try_fail(dn_free(dc_res_val2(ident))));

        dc_try_fail(next_token(p));

        if (current_token_is(p, TOK_COMMA)) dc_try_fail(next_token(p));
    }

    if (current_token_is_not(p, TOK_RPAREN)) dc_res_ret_ea(-1, "unclosed parenthesis, " current_token_err_fmt(p, TOK_RPAREN));

    dc_res_ret();
}

/**
 * Function literal: 'fn' '(' (identifier (,)?)*  ')' '{' statement* '}'
 */
static ResultDNode parse_function_literal(DParser* p)
{
    DC_RES2(ResultDNode);

    // hold current token 'fn'
    DToken* tok = p->current_token;

    dc_try_fail_temp(DCResultVoid, move_if_peek_token_is(p, TOK_LPAREN));

    dc_try_fail(dn_new(DN_FUNCTION_LITERAL, tok, true));

    /* parsing function parameters  */

    DCResultVoid res = parse_function_params(p, dc_res_val());
    dc_res_ret_if_err2(res, {
        dc_res_err_dbg_log2(res, "could not parse function literal params");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
    });

    /* a function also needs body which can be empty but it's mandatory */

    res = move_if_peek_token_is(p, TOK_LBRACE);
    dc_res_ret_if_err2(res, {
        dc_res_err_dbg_log2(res, "function literal needs body");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
    });

    dang_parser_location_preserve(p);
    ResultDNode body = parse_block_statement(p);
    dang_parser_location_revert(p);

    dc_res_ret_if_err2(body, {
        dc_res_err_dbg_log2(body, "could not parse function literal body");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
    });

    /* add body to function literal to finish up parsing */

    res = dn_child_push(dc_res_val(), dc_res_val2(body));
    dc_res_ret_if_err2(res, {
        dc_res_err_dbg_log2(res, "could not push the body to the function literal");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val2(body)));
    });

    dc_res_ret();
}

/**
 * Call Params are expressions separated by whitespace or ','
 * Until it reaches the proper end of statement that is proper for current location
 * Generally they are '\n' and ';' but also EOF, '}' based on the context
 */
static DCResultVoid parse_call(DParser* p, DNode* parent_node)
{
    DC_RES_void();

    while (!token_is_end_of_the_statement(p, current) && current_token_is_not(p, TOK_EOF))
    {
        ResultDNode param = parse_expression(p, PREC_LOWEST);

        dc_res_ret_if_err2(param, {});

        DCResultVoid res = dn_child_push(parent_node, dc_res_val2(param));
        dc_res_ret_if_err2(res, dc_try_fail(dn_free(dc_res_val2(param))));

        dang_parser_dbg_log_tokens(p);

        dc_try_fail(next_token(p));
        if (current_token_is(p, TOK_COMMA)) dc_try_fail(next_token(p));
    }

    dc_res_ret();
}

/**
 * Expression call is a function call inside '${' and '}'
 *
 * '${' command (param ','?)* '}'
 */
static ResultDNode parse_call_expression(DParser* p)
{
    DC_RES2(ResultDNode);

    // Bypass '${'
    dc_try_fail_temp(DCResultVoid, next_token(p));

    DCResultVoid res;

    dc_try_or_fail_with(dn_new(DN_CALL_EXPRESSION, NULL, true), {});

    dang_parser_location_preserve(p);

    // Switch to call loc to specify proper ending signal
    dang_parser_location_set(p, LOC_CALL);
    res = parse_call(p, dc_res_val());

    dang_parser_location_revert(p);

    dc_res_ret_if_err2(res, { dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val())); });

    dc_res_ret();
}

/**
 * Grouped expressions is used to prioritized expressions
 */
static ResultDNode parse_grouped_expression(DParser* p)
{
    DC_RES2(ResultDNode);

    dc_try_fail_temp(DCResultVoid, next_token(p));

    dang_parser_location_preserve(p);
    dc_try(parse_expression(p, PREC_LOWEST));
    dang_parser_location_revert(p);

    dc_res_ret_if_err();

    dc_try_or_fail_with3(DCResultVoid, res, move_if_peek_token_is(p, TOK_RPAREN), {
        dc_res_err_dbg_log2(res, "end of group ')' needed");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
    });

    dang_parser_dbg_log_tokens(p);

    dc_res_ret();
}

/**
 * If Expression: In this language if is an expression meaning it can return values
 *                'if' expression '{' statement* '}' ('else' '{' statement* '}')?
 */
static ResultDNode parse_if_expression(DParser* p)
{
    DC_RES2(ResultDNode);

    // hold ('if') and try to move next
    DToken* tok = p->current_token;
    dc_try_fail_temp(DCResultVoid, next_token(p));

    dc_try_fail(dn_new(DN_IF_EXPRESSION, tok, true));

    /* Getting the condition expression */
    dang_parser_location_preserve(p);
    ResultDNode condition = parse_expression(p, PREC_LOWEST);
    dang_parser_location_revert(p);

    dc_res_ret_if_err2(condition, {
        dc_res_err_dbg_log2(condition, "could not extract condition node for if expression");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
    });

    DCResultVoid res = dn_child_push(dc_res_val(), dc_res_val2(condition));
    dc_res_ret_if_err2(res, {
        dc_res_err_dbg_log2(res, "could not push the condition to the if expression");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val2(condition)));
    });

    /* Expecting to get a '{' in order to start the consequence block */

    res = move_if_peek_token_is(p, TOK_LBRACE);
    dc_res_ret_if_err2(res, {
        dc_res_err_dbg_log2(res, "If expression's consequence: Block Statement expected");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
    });

    ResultDNode consequence = parse_block_statement(p);
    dang_parser_location_revert(p);

    dc_res_ret_if_err2(consequence, {
        dc_res_err_dbg_log2(consequence, "could not extract consequence node for if expression");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
    });

    res = dn_child_push(dc_res_val(), dc_res_val2(consequence));
    dc_res_ret_if_err2(res, {
        dc_res_err_dbg_log2(res, "could not push the consequence to the if expression");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val2(consequence)));
    });

    dang_parser_dbg_log_tokens(p);

    if (peek_token_is(p, TOK_ELSE))
    {
        // bypass previous 'else'
        dc_try_or_fail_with2(res, next_token(p), {
            dc_res_err_dbg_log2(res, "could not move to the next token");

            dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        });

        /* Expecting to get a '{' in order to start the alternative block */

        res = move_if_peek_token_is(p, TOK_LBRACE);
        dc_res_ret_if_err2(res, {
            dc_res_err_dbg_log2(res, "If expression's alternative: Block Statement expected");

            dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        });

        ResultDNode alternative = parse_block_statement(p);
        dang_parser_location_revert(p);

        dc_res_ret_if_err2(alternative, {
            dc_res_err_dbg_log2(alternative, "could not extract alternative node for if expression");

            dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        });

        res = dn_child_push(dc_res_val(), dc_res_val2(alternative));
        dc_res_ret_if_err2(res, {
            dc_res_err_dbg_log2(res, "could not push the alternative to the if expression");

            dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
            dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val2(alternative)));
        });
    }

    dc_res_ret();
}

static ResultDNode parse_prefix_expression(DParser* p)
{
    DC_RES2(ResultDNode);

    DToken* tok = p->current_token;

    dc_try_fail_temp(DCResultVoid, next_token(p));

    dang_parser_location_preserve(p);
    ResultDNode right = parse_expression(p, PREC_PREFIX);
    dang_parser_location_revert(p);

    dc_res_ret_if_err2(right, { dc_res_err_dbg_log2(right, "could not parse right hand side"); });

    dc_try_fail(dn_new(DN_PREFIX_EXPRESSION, tok, true));

    DCResultVoid res = dn_child_push(dc_res_val(), dc_res_val2(right));
    dc_res_ret_if_err2(res, {
        dc_res_err_dbg_log2(res, "could not push the value to expression");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val2(right)));
    });

    dc_res_ret();
}

/**
 * Infix Expressions: - + * / == < etc.
 */
static ResultDNode parse_infix_expression(DParser* p, DNode* left)
{
    DC_TRY_DEF2(ResultDNode, dn_new(DN_INFIX_EXPRESSION, p->current_token, true));

    DCResultVoid res = dn_child_push(dc_res_val(), left);
    dc_res_ret_if_err2(res, {
        dc_res_err_dbg_log2(res, "could not push the value to expression");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
    });

    Precedence prec = current_prec(p);

    res = next_token(p);
    dc_res_ret_if_err2(res, {
        dc_res_err_dbg_log2(res, "could not move to the next token");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
    });

    dang_parser_location_preserve(p);
    ResultDNode right = parse_expression(p, prec);
    dang_parser_location_revert(p);

    dc_res_ret_if_err2(right, {
        dc_res_err_dbg_log2(right, "could not parse right hand side");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
    });

    res = dn_child_push(dc_res_val(), dc_res_val2(right));
    dc_res_ret_if_err2(res, {
        dc_res_err_dbg_log2(res, "could not push the value to expression");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val2(right)));
    });

    dang_parser_dbg_log_tokens(p);

    dc_res_ret();
}

/**
 * Let Statement: 'let' identifier expression? StatementTerminator
 */
static ResultDNode parse_let_statement(DParser* p)
{
    DC_RES2(ResultDNode);

    DToken* tok = p->current_token; // capturing current token 'let'

    /* Parsing identifier for let statement */

    // Check if the next token is an identifier or not
    dc_try_or_fail_with3(DCResultVoid, res, move_if_peek_token_is(p, TOK_IDENT),
                         { dc_res_err_dbg_log2(res, "Identifier needed"); });

    // Try to create a new identifier node
    dc_try_or_fail_with3(ResultDNode, temp_node, dn_new(DN_IDENTIFIER, p->current_token, false),
                         { dc_res_err_dbg_log2(temp_node, "could not parse name"); });

    // Try to create a new let statement node
    // If successful it will be saved in the main result variable
    dc_try_or_fail_with(dn_new(DN_LET_STATEMENT, tok, true), {
        dc_res_err_dbg_log("could not create let statement node");
        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val2(temp_node)));
    });

    // Try to push the identifier to the let statement as the first child
    dc_try_or_fail_with2(res, dn_child_push(dc_res_val(), dc_res_val2(temp_node)), {
        dc_res_err_dbg_log2(res, "could not push the name to statement");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val2(temp_node)));
    });

    // If the let statement doesn't have initial value just return.
    if (token_is_end_of_the_statement(p, peek) || peek_token_is(p, TOK_EOF))
    {
        dc_try_or_fail_with2(res, next_token(p), {
            dc_res_err_dbg_log2(res, "could not move to the next token");

            dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        });

        dc_res_ret(); // return successfully
    }

    /* Parsing initial value */

    // Move to the next token
    dc_try_or_fail_with2(res, next_token(p), {
        dc_res_err_dbg_log2(res, "could not move to the next token");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
    });

    // Try to parse expression as initial value
    dc_try_or_fail_with2(temp_node, parse_expression(p, PREC_LOWEST), {
        dc_res_err_dbg_log2(temp_node, "could not parse value");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
    });

    // Try to push the initial value expression
    dc_try_or_fail_with2(res, dn_child_push(dc_res_val(), dc_res_val2(temp_node)), {
        dc_res_err_dbg_log2(res, "could not push the value to statement");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val2(temp_node)));
    });

    dang_parser_dbg_log_tokens(p);

    // Based on the current location a proper terminator must be seen
    if (token_is_end_of_the_statement(p, peek) || peek_token_is(p, TOK_EOF))
    {
        dc_try_or_fail_with2(res, next_token(p), {
            dc_res_err_dbg_log2(res, "could not move to the next token");

            dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        });

        dc_res_ret(); // return successfully
    }

    dc_res_ret_ea(-1, "end of statement needed, got token of type %s.", tostr_DTokenType(p->peek_token->type));
}

/**
 * Return Statement: 'let' expression? StatementTerminator
 */
static ResultDNode parse_return_statement(DParser* p)
{
    DC_TRY_DEF2(ResultDNode, dn_new(DN_RETURN_STATEMENT, p->current_token, true));

    // Check if it's a return without a value
    if (token_is_end_of_the_statement(p, peek) || peek_token_is(p, TOK_EOF))
    {
        dc_try_or_fail_with3(DCResultVoid, res, next_token(p), {
            dc_res_err_dbg_log2(res, "could not move to the next token");

            dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        });

        dc_res_ret(); // return successfully
    }

    /* Try to parse return value (expression) */

    dc_try_or_fail_with3(DCResultVoid, res, next_token(p), {
        dc_res_err_dbg_log2(res, "could not move to the next token");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
    });

    dc_try_or_fail_with3(ResultDNode, value, parse_expression(p, PREC_LOWEST), {
        dc_res_err_dbg_log2(res, "could not parse value");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
    });

    dang_parser_dbg_log_tokens(p);

    dc_try_or_fail_with2(res, dn_child_push(dc_res_val(), dc_res_val2(value)), {
        dc_res_err_dbg_log2(res, "could not push the value to statement");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val2(value)));
    });

    // Based on the current location a proper terminator must be seen
    if (token_is_end_of_the_statement(p, peek) || peek_token_is(p, TOK_EOF))
    {
        dc_try_or_fail_with2(res, next_token(p), {
            dc_res_err_dbg_log2(res, "could not move to the next token");

            dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        });

        dc_res_ret(); // return successfully
    }

    dc_res_ret_ea(-1, "end of statement needed, got token of type %s.", tostr_DTokenType(p->peek_token->type));
}

static ResultDNode parse_block_statement(DParser* p)
{
    DC_RES2(ResultDNode);

    DToken* tok = p->current_token;

    // Try to bypass the '{'
    dc_try_fail_temp(DCResultVoid, next_token(p));

    dc_try_fail(dn_new(DN_BLOCK_STATEMENT, tok, true));

    DCResultVoid res;

    while (current_token_is_not(p, TOK_RBRACE) && current_token_is_not(p, TOK_EOF))
    {
        /* Bypassing all the meaningless newlines and semicolons */
        try_bypassing_all_sc_and_nls_or_fail_with(p, {
            dc_res_err_dbg_log2(res, "could move to the next token");

            dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        });

        // Enter the block
        dang_parser_location_set(p, LOC_BLOCK);

        ResultDNode stmt = parse_statement(p);
        dc_res_ret_if_err2(stmt, {
            dc_res_err_dbg_log2(stmt, "cannot parse block statement");

            dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        });

        dang_parser_dbg_log_tokens(p);

        res = dn_child_push(dc_res_val(), dc_res_val2(stmt));
        dc_res_ret_if_err2(res, {
            dc_res_err_dbg_log2(res, "could not push the statement to the block");

            dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
            dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val2(stmt)));
        });

        /* Bypassing all the meaningless newlines and semicolons */
        try_bypassing_all_sc_and_nls_or_fail_with(p, {
            dc_res_err_dbg_log2(res, "could move to the next token");

            dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        });
    }

    if (current_token_is(p, TOK_EOF)) dc_res_ret_e(-1, "block ended with EOF, expected '}' instead");

    dc_res_ret();
}

/**
 * Parsing the expression which is going to be used by other expressions or statement
 *
 * Expressions: prefix, infix, if, etc.
 */
static ResultDNode parse_expression(DParser* p, Precedence precedence)
{
    DC_RES2(ResultDNode);

    ParsePrefixFn prefix = p->parse_prefix_fns[p->current_token->type];

    if (!prefix)
    {
        dc_dbg_log(unexpected_token_err_fmt(p->current_token->type));

        dc_res_ret_ea(1, unexpected_token_err_fmt(p->current_token->type));
    }

    dang_parser_location_preserve(p);
    ResultDNode left_exp = prefix(p);
    dang_parser_location_revert(p);

    if (dc_res_is_err2(left_exp)) return left_exp;

    dang_parser_dbg_log_tokens(p);
    dc_dbg_log("precedence %d, peek precedence %d", precedence, peek_prec(p));

    while (!token_is_end_of_the_statement(p, peek) && peek_token_is_not(p, TOK_EOF) && peek_token_is_not(p, TOK_COMMA) &&
           precedence < peek_prec(p))
    {
        ParseInfixFn infix = p->parse_infix_fns[p->peek_token->type];
        if (!infix) return left_exp;

        dc_try_or_fail_with3(DCResultVoid, res, next_token(p), {
            dc_res_err_dbg_log2(res, "could not move to the next token");

            dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val2(left_exp)));
        });

        dang_parser_location_preserve(p);
        left_exp = infix(p, dc_res_val2(left_exp));
        dang_parser_location_revert(p);
    }

    return left_exp;
}

/**
 * Parsing expression as an statement
 *
 * Basically a function call (command) or other expressions
 */
static ResultDNode parse_expression_statement(DParser* p)
{
    DC_RES2(ResultDNode);

    DCResultVoid res;
    ResultDNode call = dn_new(DN_CALL_EXPRESSION, NULL, true);
    dc_res_ret_if_err2(call, {});

    dang_parser_location_preserve(p);
    res = parse_call(p, dc_res_val2(call));
    dang_parser_location_revert(p);

    dc_res_ret_if_err2(res, { dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val2(call))); });

    dc_try_or_fail_with(dn_new(DN_EXPRESSION_STATEMENT, NULL, true),
                        { dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val2(call))); });

    if (dn_child_count(dc_res_val2(call)) == 1 && dn_child(dc_res_val2(call), 0)->type != DN_CALL_EXPRESSION &&
        current_token_is_not(p, TOK_SEMICOLON))
    {
        // return the first child of call node
        DNode* first_child = dn_child(dc_res_val2(call), 0);

        // mark the first child as NULL so it won't get freed
        dc_dv_set(dc_res_val2(call)->children.elements[0], voidptr, NULL);

        // free the call node
        dc_try_or_fail_with2(res, dn_free(dc_res_val2(call)), {
            dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
            dc_try_fail_temp(DCResultVoid, dn_free(first_child));
        });

        // push the first child to the expression statement
        dc_try_or_fail_with2(res, dn_child_push(dc_res_val(), first_child), {
            dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
            dc_try_fail_temp(DCResultVoid, dn_free(first_child));
        });

        dc_res_ret();
    }

    dc_try_or_fail_with2(res, dn_child_push(dc_res_val(), dc_res_val2(call)), {
        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val2(call)));
    });

    dc_res_ret();
}

/**
 * Parses an statement to its proper statement terminator ';' '\n' '}' ')', etc.
 *
 * They suppose to move to the next token after their terminator already
 */
static ResultDNode parse_statement(DParser* p)
{
    DC_RES2(ResultDNode);

    dang_parser_location_preserve(p);

    ResultDNode result;

    switch (p->current_token->type)
    {
        case TOK_LET:
            result = parse_let_statement(p);
            break;

        case TOK_RET:
            result = parse_return_statement(p);
            break;

        default:
            result = parse_expression_statement(p);
            break;
    }

    if (dc_res_is_err2(result)) try_moving_to_the_end_of_statement(p);

    dang_parser_location_revert(p);

    return result;
}

// ***************************************************************************************
// * PUBLIC FUNCTIONS
// ***************************************************************************************

DCResultVoid dang_parser_init(DParser* p, DScanner* s)
{
    DC_RES_void();

    if (!p || !s)
    {
        dc_dbg_log("DParser or DScanner cannot be NULL");

        dc_res_ret_e(1, "DParser or DScanner cannot be NULL");
    }

    p->scanner = s;

    p->current_token = NULL;
    p->peek_token = NULL;

    dc_try(dc_da_init(&p->errors, NULL));

    // Starting from body
    p->loc = LOC_BODY;

    // Initialize function pointers
    memset(p->parse_prefix_fns, 0, sizeof(p->parse_prefix_fns));
    memset(p->parse_infix_fns, 0, sizeof(p->parse_infix_fns));

    p->parse_prefix_fns[TOK_IDENT] = parse_identifier;
    p->parse_prefix_fns[TOK_STRING] = parse_string_literal;
    p->parse_prefix_fns[TOK_INT] = parse_integer_literal;
    p->parse_prefix_fns[TOK_BANG] = parse_prefix_expression;
    p->parse_prefix_fns[TOK_MINUS] = parse_prefix_expression;
    p->parse_prefix_fns[TOK_TRUE] = parse_boolean_literal;
    p->parse_prefix_fns[TOK_FALSE] = parse_boolean_literal;
    p->parse_prefix_fns[TOK_LPAREN] = parse_grouped_expression;
    p->parse_prefix_fns[TOK_IF] = parse_if_expression;
    p->parse_prefix_fns[TOK_FUNCTION] = parse_function_literal;
    p->parse_prefix_fns[TOK_DOLLAR_LBRACE] = parse_call_expression;

    // Illegal tokens that are supposed to be bypassed already
    p->parse_prefix_fns[TOK_EOF] = parse_illegal;
    p->parse_prefix_fns[TOK_ILLEGAL] = parse_illegal;
    p->parse_prefix_fns[TOK_COMMA] = parse_illegal;
    p->parse_prefix_fns[TOK_NEWLINE] = parse_illegal;
    p->parse_prefix_fns[TOK_SEMICOLON] = parse_illegal;
    p->parse_prefix_fns[TOK_RBRACE] = parse_illegal;
    p->parse_prefix_fns[TOK_RPAREN] = parse_illegal;

    p->parse_infix_fns[TOK_PLUS] = parse_infix_expression;
    p->parse_infix_fns[TOK_MINUS] = parse_infix_expression;
    p->parse_infix_fns[TOK_SLASH] = parse_infix_expression;
    p->parse_infix_fns[TOK_ASTERISK] = parse_infix_expression;
    p->parse_infix_fns[TOK_EQ] = parse_infix_expression;
    p->parse_infix_fns[TOK_NEQ] = parse_infix_expression;
    p->parse_infix_fns[TOK_LT] = parse_infix_expression;
    p->parse_infix_fns[TOK_GT] = parse_infix_expression;

    // Update current and peek tokens
    dc_try_fail(next_token(p));
    dc_try_fail(next_token(p));

    dc_res_ret();
}

DCResultVoid dang_parser_free(DParser* p)
{
    DC_RES_void();

    dc_try(dc_da_free(&p->errors));

    dc_try(dang_scanner_free(p->scanner));

    dc_res_ret();
}

/**
 * Program: one or more statement
 *
 * On each loop it tries to parse one statement and save it if it's a success otherwise
 * It already has logged its corresponding error(s)
 *
 * On each iteration the cursor must be at the end of the current statement (sentence terminator)
 */
ResultDNode dang_parser_parse_program(DParser* p)
{
    DC_TRY_DEF2(ResultDNode, dn_new(DN_PROGRAM, NULL, true));

    DCResultVoid res = {0};

    while (true)
    {
        /* Bypassing all the meaningless newlines and semicolons */
        while (current_token_is(p, TOK_SEMICOLON) || current_token_is(p, TOK_NEWLINE))
        {
            res = next_token(p);
            if (dc_res_is_err2(res))
            {
                dc_res_err_dbg_log2(res, "could not move to the next token");

                add_error(p, &dc_res_err2(res));

                break;
            }
        }

        /* Break out of the loop if the cursor is at the end */
        if (current_token_is(p, TOK_EOF)) break;

        /* Do actual statement parsing */
        ResultDNode stmt = parse_statement(p);

        if (dc_res_is_ok2(stmt))
        {
            res = dn_child_push(dc_res_val(), dc_res_val2(stmt));
            if (dc_res_is_err2(res))
            {
                dc_res_err_dbg_log2(res, "could not push the statement to program");

                add_error(p, &dc_res_err2(res));

                dn_free(dc_res_val2(stmt));
            }

            continue;
        }

        add_error(p, &dc_res_err2(stmt));
    }

    dc_res_ret();
}

void dang_parser_log_errors(DParser* p)
{
    dc_da_for(p->errors)
    {
        string error = dc_da_get_as(p->errors, _idx, string);
        dc_log(dc_colorize_fg(LRED, "%s"), error);
    }
}
