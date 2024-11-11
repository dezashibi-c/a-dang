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

#define DN__MAX (dc_dvt(DNodeIndexExpression) + 1)

static ParsePrefixFn parse_prefix_fns[DN__MAX];
static ParseInfixFn parse_infix_fns[DN__MAX];

// ***************************************************************************************
// * PRIVATE FUNCTIONS DECLARATIONS
// ***************************************************************************************

static DCRes parse_expression(DParser* p, Precedence precedence);
static DCRes parse_block_statement(DParser* p);
static DCRes parse_statement(DParser* p);

// ***************************************************************************************
// * PRIVATE FUNCTIONS
// ***************************************************************************************

#define token_is(TOKEN, TYPE) (TOKEN.type == TYPE)
#define token_is_not(TOKEN, TYPE) (TOKEN.type != TYPE)

#define current_token_is(P, TYPE) token_is((P)->current_token, TYPE)
#define current_token_is_not(P, TYPE) token_is_not((P)->current_token, TYPE)

#define peek_token_is(P, TYPE) token_is((P)->peek_token, TYPE)
#define peek_token_is_not(P, TYPE) token_is_not((P)->peek_token, TYPE)

#define peek_prec(P) ((P)->peek_token.type < TOK_TYPE_MAX ? get_precedence((P)->peek_token.type) : PREC_LOWEST)
#define current_prec(P) get_precedence((P)->current_token.type)

#define dang_parser_location_preserve(P) DParserStatementLoc __dang_parser_loc_snapshot = (P)->loc
#define dang_parser_location_revert(P) (P)->loc = __dang_parser_loc_snapshot
#define dang_parser_location_set(P, LOC) (P)->loc = LOC

#define check_quote(CALLEE, PARAMS)                                                                                            \
    do                                                                                                                         \
    {                                                                                                                          \
        if (dc_unwrap2(CALLEE).type == dc_dvt(DNodeIdentifier) &&                                                              \
            (strcmp(dc_dv_as(dc_unwrap2(CALLEE), DNodeIdentifier).value, "quote") == 0 ||                                      \
             strcmp(dc_dv_as(dc_unwrap2(CALLEE), DNodeIdentifier).value, "unquote") == 0))                                     \
        {                                                                                                                      \
            if (dc_unwrap2(PARAMS)->count != 1)                                                                                \
                dc_ea(-1, "'%s' accept only and only one argument, got=" dc_fmt(usize),                                        \
                      dc_dv_as(dc_unwrap2(CALLEE), DNodeIdentifier).value, dc_unwrap2(PARAMS)->count);                         \
        }                                                                                                                      \
    } while (0)

/**
 * expands to checking if the token is end of statement or not
 *
 * @param P is the parser pointer
 * @param TOK must be whether 'current' or 'peek' without single or double quotes
 */
#define token_is_end_of_the_statement(P, TOK)                                                                                  \
    ((((P)->loc == LOC_CALL || (P)->loc == LOC_BLOCK) && TOK##_token_is(P, TOK_RBRACE)) ||                                     \
     (((P)->loc == LOC_ARRAY) && TOK##_token_is(P, TOK_RBRACKET)) ||                                                           \
     (((P)->loc != LOC_CALL && (P)->loc != LOC_ARRAY) &&                                                                       \
      (TOK##_token_is(P, TOK_SEMICOLON) || TOK##_token_is(P, TOK_NEWLINE))))

#define try_moving_to_the_end_of_statement(P)                                                                                  \
    while (!token_is_end_of_the_statement(P, current) && current_token_is_not(P, TOK_EOF))                                     \
    dc_try_fail_temp(DCResVoid, next_token((P)))

#define try_bypassing_all_sc_and_nls_or_fail_with(P, PRE_EXIT_ACTIONS)                                                         \
    while (current_token_is(P, TOK_SEMICOLON) || current_token_is(P, TOK_NEWLINE))                                             \
    {                                                                                                                          \
        DCResVoid res = next_token(p);                                                                                         \
        dc_ret_if_err2(res, { PRE_EXIT_ACTIONS; });                                                                            \
    }

#define try_bypassing_all_nls_or_fail_with(P, PRE_EXIT_ACTIONS)                                                                \
    while (current_token_is(P, TOK_NEWLINE))                                                                                   \
    {                                                                                                                          \
        DCResVoid res = next_token(p);                                                                                         \
        dc_ret_if_err2(res, { PRE_EXIT_ACTIONS; });                                                                            \
    }

#define dang_parser_log_tokens(P)                                                                                              \
    dc_log("cur tok: %s, next tok: %s", tostr_DTokType((P)->current_token.type),                                               \
           (P)->peek_token.type != TOK_TYPE_MAX ? tostr_DTokType((P)->peek_token.type) : "NULL")

#define dang_parser_dbg_log_tokens(P)                                                                                          \
    dc_dbg_log("cur tok: %s, next tok: %s", tostr_DTokType((P)->current_token.type),                                           \
               (P)->peek_token.type != TOK_TYPE_MAX ? tostr_DTokType((P)->peek_token.type) : "NULL")

#define next_token_err_fmt(P, TYPE)                                                                                            \
    "expected next token to be %s, got %s instead.", tostr_DTokType(TYPE), tostr_DTokType(P->peek_token.type)

#define current_token_err_fmt(P, TYPE)                                                                                         \
    "expected current token to be %s, got %s instead.", tostr_DTokType(TYPE), tostr_DTokType(P->current_token.type)

#define unexpected_token_err_fmt(TYPE) "unexpected token '%s'.", tostr_DTokType(TYPE)

#define pool_last_el(P) &dc_da_get2(*(P)->pool, (P)->pool->count - 1)

#define return_pool_last_el(P) dc_res_ok(dc_dv(DCDynValPtr, pool_last_el(P)))

static DCResVoid next_token(DParser* p)
{
    DC_RES_void();

    p->current_token = p->peek_token;

    if (current_token_is(p, TOK_EOF))
    {
        p->peek_token.type = TOK_TYPE_MAX;
        dc_ret();
    }

    ResTok res = dang_scanner_next_token(&p->scanner);
    dc_fail_if_err2(res);

    p->peek_token = dc_unwrap2(res);

    dc_ret();
}

static void add_error(DParser* p, DCError* err)
{
    dc_da_push(p->errors, (err->allocated ? dc_dva(string, err->message) : dc_dv(string, err->message)));
}

static DCResVoid move_if_peek_token_is(DParser* p, DTokType type)
{
    DC_RES_void();

    if (peek_token_is(p, type)) return next_token(p);

    dc_ret_ea(-2, next_token_err_fmt(p, type));
}

static Precedence get_precedence(DTokType type)
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

        case TOK_LBRACKET:
            return PREC_INDEX;

        default:
            break;
    };

    return PREC_LOWEST;
}

static DCRes parse_illegal(DParser* p)
{
    DC_RES();

    dc_ret_ea(-1, "got illegal token of type: %s", tostr_DTokType(p->current_token.type));
}

static DCRes parse_identifier(DParser* p)
{
    DC_RES();
    string data = NULL;
    dc_try_fail_temp(DCResUsize, dc_sprintf(&data, DCPRIsv, dc_sv_fmt(p->current_token.text)));

    dc_try_or_fail_with3(DCResVoid, res, dc_da_push(p->pool, dc_dva(string, data)), { free(data); });

    dc_ret_ok_dv(DNodeIdentifier, dn_identifier(data));
}

static DCRes parse_string_literal(DParser* p)
{
    DC_RES();

    DCDynVal data;

    if (p->current_token.text.len == 0)
        data = dc_dv(string, "");
    else
    {
        string data_str = NULL;
        dc_try_fail_temp(DCResUsize, dc_sprintf(&data_str, DCPRIsv, dc_sv_fmt(p->current_token.text)));
        data = dc_dv(string, data_str);

        dc_try_or_fail_with3(DCResVoid, res, dc_da_push(p->pool, dc_dva(string, data_str)), { free(data_str); });
    }

    dc_ret_ok(data);
}

static DCRes parse_integer_literal(DParser* p)
{
    DC_RES();

    string str = NULL;
    dc_try_fail_temp(DCResUsize, dc_sprintf(&str, DCPRIsv, dc_sv_fmt(p->current_token.text)));

    DCResI64 i64_res = dc_str_to_i64(str);
    dc_ret_if_err2(i64_res, {
        dc_err_dbg_log2(i64_res, "could not parse token text to i64 number");
        free(str);
    });

    free(str);

    dc_ret_ok_dv(i64, dc_unwrap2(i64_res));
}

static DCRes parse_boolean_literal(DParser* p)
{
    DC_RES();
    dc_ret_ok_dv_bool(p->current_token.type == TOK_TRUE);
}

static DCResDa parse_function_params(DParser* p)
{
    DC_RES_da();

    dc_try_fail_temp(DCResVoid, next_token(p));

    dc_try_fail(dc_da_new(NULL));

    DCDynArrPtr arr = dc_unwrap();

    while (current_token_is_not(p, TOK_RPAREN) && current_token_is_not(p, TOK_EOF))
    {
        DCRes ident = parse_identifier(p);
        dc_ret_if_err2(ident, {
            dc_try_fail_temp(DCResVoid, dc_da_free(arr));
            free(arr);
        });

        dc_try_or_fail_with3(DCResVoid, res, dc_da_push(arr, dc_unwrap2(ident)), {
            dc_try_fail_temp(DCResVoid, dc_da_free(arr));
            free(arr);
        });

        dc_try_or_fail_with2(res, next_token(p), {
            dc_try_fail_temp(DCResVoid, dc_da_free(arr));
            free(arr);
        });

        if (current_token_is(p, TOK_COMMA))
            dc_try_or_fail_with2(res, next_token(p), {
                dc_try_fail_temp(DCResVoid, dc_da_free(arr));
                free(arr);
            });
    }

    if (current_token_is_not(p, TOK_RPAREN))
    {
        dc_try_fail_temp(DCResVoid, dc_da_free(arr));
        free(arr);

        dc_ret_ea(-1, "unclosed parenthesis, " current_token_err_fmt(p, TOK_RPAREN));
    }

    dc_try_or_fail_with3(DCResVoid, res, dc_da_push(p->pool, dc_dva(DCDynArrPtr, arr)), {
        dc_try_fail_temp(DCResVoid, dc_da_free(arr));
        free(arr);
    });

    dc_ret();
}

/**
 * Function literal: 'fn' '(' (identifier (,)?)*  ')' '{' statement* '}'
 */
static DCRes parse_function_literal(DParser* p)
{
    DC_RES();

    dc_try_fail_temp(DCResVoid, move_if_peek_token_is(p, TOK_LPAREN));

    /* parsing function parameters  */
    dc_try_or_fail_with3(DCResDa, params_arr_res, parse_function_params(p), {});

    DCDynArrPtr params_arr = dc_unwrap2(params_arr_res);

    /* a function also needs body which can be empty but it's mandatory */

    DCResVoid res = move_if_peek_token_is(p, TOK_LBRACE);
    dc_ret_if_err2(res, {
        dc_err_dbg_log2(res, "function literal needs body");

        dc_try_fail_temp(DCResVoid, dc_da_free(params_arr));
        free(params_arr);
    });

    dang_parser_location_preserve(p);
    DCRes body = parse_block_statement(p);
    dang_parser_location_revert(p);

    dc_ret_if_err2(body, {
        dc_err_dbg_log2(body, "could not parse function literal body");

        dc_try_fail_temp(DCResVoid, dc_da_free(params_arr));
        free(params_arr);
    });

    DCDynArrPtr body_statements = dc_dv_as(dc_unwrap2(body), DNodeBlockStatement).statements;

    /* add body to function literal to finish up parsing */

    dc_ret_ok_dv(DNodeFunctionLiteral, dn_function(params_arr, body_statements));
}

/**
 * Macro: 'macro' '(' (identifier (,)?)*  ')' '{' statement* '}'
 */
static DCRes parse_macro(DParser* p)
{
    DC_RES();

    dc_try_fail_temp(DCResVoid, move_if_peek_token_is(p, TOK_LPAREN));

    /* parsing macro parameters  */
    dc_try_or_fail_with3(DCResDa, params_arr_res, parse_function_params(p), {});

    DCDynArrPtr params_arr = dc_unwrap2(params_arr_res);

    /* a macro also needs body which can be empty but it's mandatory */

    DCResVoid res = move_if_peek_token_is(p, TOK_LBRACE);
    dc_ret_if_err2(res, {
        dc_err_dbg_log2(res, "macro needs body");

        dc_try_fail_temp(DCResVoid, dc_da_free(params_arr));
        free(params_arr);
    });

    dang_parser_location_preserve(p);
    DCRes body = parse_block_statement(p);
    dang_parser_location_revert(p);

    dc_ret_if_err2(body, {
        dc_err_dbg_log2(body, "could not parse macro body");

        dc_try_fail_temp(DCResVoid, dc_da_free(params_arr));
        free(params_arr);
    });

    DCDynArrPtr body_statements = dc_dv_as(dc_unwrap2(body), DNodeBlockStatement).statements;

    /* add body to macro to finish up parsing */

    dc_ret_ok_dv(DNodeMacro, dn_macro(params_arr, body_statements));
}

/**
 * Call Params are expressions separated by whitespace or ','
 * Until it reaches the proper end of statement that is proper for current location
 * Generally they are '\n' and ';' but also EOF, '}' based on the context
 */
static DCResDa parse_expression_list(DParser* p)
{
    DC_RES_da();

    dc_try_fail(dc_da_new(NULL));

    DCDynArrPtr exp_list = dc_unwrap();

    while (!token_is_end_of_the_statement(p, current) && current_token_is_not(p, TOK_EOF))
    {
        DCRes param = parse_expression(p, PREC_LOWEST);

        dc_ret_if_err2(param, {
            dc_try_fail_temp(DCResVoid, dc_da_free(exp_list));
            free(exp_list);
        });

        DCResVoid res = dc_da_push(exp_list, dc_unwrap2(param));
        dc_ret_if_err2(res, {
            dc_try_fail_temp(DCResVoid, dc_da_free(exp_list));
            free(exp_list);
        });

        dang_parser_dbg_log_tokens(p);

        dc_try_or_fail_with2(res, next_token(p), {
            dc_try_fail_temp(DCResVoid, dc_da_free(exp_list));
            free(exp_list);
        });
        if (current_token_is(p, TOK_COMMA))
            dc_try_or_fail_with2(res, next_token(p), {
                dc_try_fail_temp(DCResVoid, dc_da_free(exp_list));
                free(exp_list);
            });
    }

    dc_try_or_fail_with3(DCResVoid, res, dc_da_push(p->pool, dc_dva(DCDynArrPtr, exp_list)), {
        dc_try_fail_temp(DCResVoid, dc_da_free(exp_list));
        free(exp_list);
    });

    dc_ret();
}

/**
 * Expression call is a function call inside '${' and '}'

 *  '${' command (expression ','?)* '}'
 */
static DCRes parse_call_expression(DParser* p)
{
    DC_RES();

    // Bypass opening '${'
    dc_try_fail_temp(DCResVoid, next_token(p));

    dang_parser_location_preserve(p);

    // get the callee
    dang_parser_location_set(p, LOC_CALL);
    DCRes callee = parse_expression(p, PREC_LOWEST);
    dang_parser_location_revert(p);

    dc_ret_if_err2(callee, {});

    // go to next token and bypass if comma is seen
    dc_try_or_fail_with3(DCResVoid, res, next_token(p), {});
    if (current_token_is(p, TOK_COMMA)) dc_try_or_fail_with2(res, next_token(p), {});

    DCDynArrPtr params = NULL;

    if (!token_is_end_of_the_statement(p, current) && current_token_is_not(p, TOK_EOF))
    {
        // Switch to call loc to specify proper ending signal
        dang_parser_location_set(p, LOC_CALL);
        DCResDa params_res = parse_expression_list(p);
        dang_parser_location_revert(p);

        dc_ret_if_err2(res, {});

        check_quote(callee, params_res);
        dc_ret_if_err({});

        params = dc_unwrap2(params_res);
    }

    dc_try_or_fail_with2(res, dc_da_push(p->pool, dc_unwrap2(callee)), {});

    dc_ret_ok_dv(DNodeCallExpression, dn_call(pool_last_el(p), params));
}

/**
 * Hash Literal '{' (expression ':' expression ','?)*  '}'
 */
static DCRes parse_hash_literal(DParser* p)
{
    DC_RES();

    // Bypass opening '{'
    dc_try_fail_temp(DCResVoid, next_token(p));

    /* Bypassing all the meaningless newlines */
    try_bypassing_all_nls_or_fail_with(p, { dc_err_dbg_log2(res, "could move to the next token"); });

    dc_try_or_fail_with3(DCResDa, key_values_res, dc_da_new(NULL), {});
    DCDynArrPtr key_values = dc_unwrap2(key_values_res);

    dang_parser_location_preserve(p);

    DCResVoid res;
    while (current_token_is_not(p, TOK_RBRACE) && current_token_is_not(p, TOK_EOF))
    {
        DCRes key = parse_expression(p, PREC_LOWEST);
        dang_parser_location_revert(p);

        dc_ret_if_err2(key, {
            dc_try_fail_temp(DCResVoid, dc_da_free(key_values));
            free(key_values);
        });

        dc_try_or_fail_with2(res, dc_da_push(key_values, dc_unwrap2(key)), {
            dc_try_fail_temp(DCResVoid, dc_da_free(key_values));
            free(key_values);
        });

        dang_parser_dbg_log_tokens(p);

        dc_try_or_fail_with2(res, move_if_peek_token_is(p, TOK_COLON), {
            dc_try_fail_temp(DCResVoid, dc_da_free(key_values));
            free(key_values);
        });
        dc_try_or_fail_with2(res, next_token(p), {
            dc_try_fail_temp(DCResVoid, dc_da_free(key_values));
            free(key_values);
        });

        DCRes value = parse_expression(p, PREC_LOWEST);
        dang_parser_location_revert(p);

        dc_ret_if_err2(value, {
            dc_try_fail_temp(DCResVoid, dc_da_free(key_values));
            free(key_values);
        });

        dc_try_or_fail_with2(res, dc_da_push(key_values, dc_unwrap2(value)), {
            dc_try_fail_temp(DCResVoid, dc_da_free(key_values));
            free(key_values);
        });

        dang_parser_dbg_log_tokens(p);

        dc_try_or_fail_with2(res, next_token(p), {
            dc_try_fail_temp(DCResVoid, dc_da_free(key_values));
            free(key_values);
        });
        if (current_token_is(p, TOK_COMMA))
            dc_try_or_fail_with2(res, next_token(p), {
                dc_try_fail_temp(DCResVoid, dc_da_free(key_values));
                free(key_values);
            });

        /* Bypassing all the meaningless newlines */
        try_bypassing_all_nls_or_fail_with(p, {
            dc_err_dbg_log2(res, "could move to the next token");

            dc_try_fail_temp(DCResVoid, dc_da_free(key_values));
            free(key_values);
        });
    }

    dc_try_or_fail_with2(res, dc_da_push(p->pool, dc_dva(DCDynArrPtr, key_values)), {
        dc_try_fail_temp(DCResVoid, dc_da_free(key_values));
        free(key_values);
    });

    dc_ret_ok_dv(DNodeHashTableLiteral, dn_hash_table(key_values));
}

/**
 * Array Literal '[' (expression ','?)*  ']'
 */
static DCRes parse_array_literal(DParser* p)
{
    DC_RES();

    // Bypass opening '['
    dc_try_fail_temp(DCResVoid, next_token(p));

    dang_parser_location_preserve(p);

    // Switch to array loc to specify proper ending signal
    dang_parser_location_set(p, LOC_ARRAY);
    dc_try_or_fail_with3(DCResDa, array_res, parse_expression_list(p), {});
    dang_parser_location_revert(p);

    DCDynArrPtr array = dc_unwrap2(array_res);

    dc_ret_ok_dv(DNodeArrayLiteral, dn_array(array));
}

/**
 * Grouped expressions is used to prioritized expressions
 */
static DCRes parse_grouped_expression(DParser* p)
{
    DC_RES();

    dc_try_fail_temp(DCResVoid, next_token(p));

    dang_parser_location_preserve(p);
    dc_try(parse_expression(p, PREC_LOWEST));
    dang_parser_location_revert(p);

    dc_ret_if_err();

    dc_try_or_fail_with3(DCResVoid, res, move_if_peek_token_is(p, TOK_RPAREN),
                         { dc_err_dbg_log2(res, "end of group ')' needed"); });

    dang_parser_dbg_log_tokens(p);

    dc_ret();
}

/**
 * If Expression: In this language if is an expression meaning it can return values
 *                'if' expression '{' statement* '}' ('else' '{' statement* '}')?
 */
static DCRes parse_if_expression(DParser* p)
{
    DC_RES();

    // try to move next to bypass the 'if' token
    dc_try_fail_temp(DCResVoid, next_token(p));

    /* Getting the condition expression */
    dang_parser_location_preserve(p);
    DCRes condition = parse_expression(p, PREC_LOWEST);
    dang_parser_location_revert(p);

    dc_ret_if_err2(condition, { dc_err_dbg_log2(condition, "could not extract condition node for if expression"); });

    /* Expecting to get a '{' in order to start the consequence block */

    DCResVoid res = move_if_peek_token_is(p, TOK_LBRACE);
    dc_ret_if_err2(res, { dc_err_dbg_log2(res, "If expression's consequence: Block Statement expected"); });

    DCRes consequence = parse_block_statement(p);
    dang_parser_location_revert(p);

    dc_ret_if_err2(consequence, { dc_err_dbg_log2(consequence, "could not extract consequence node for if expression"); });

    dang_parser_dbg_log_tokens(p);

    DCDynArrPtr consequence_statements = dc_dv_as(dc_unwrap2(consequence), DNodeBlockStatement).statements;

    DCDynArrPtr alternative_statements = NULL;

    if (peek_token_is(p, TOK_ELSE))
    {
        // bypass previous 'else'
        dc_try_or_fail_with2(res, next_token(p), { dc_err_dbg_log2(res, "could not move to the next token"); });

        /* Expecting to get a '{' in order to start the alternative block */

        res = move_if_peek_token_is(p, TOK_LBRACE);
        dc_ret_if_err2(res, { dc_err_dbg_log2(res, "If expression's alternative: Block Statement expected"); });

        DCRes alternative = parse_block_statement(p);
        dang_parser_location_revert(p);

        dc_ret_if_err2(alternative, { dc_err_dbg_log2(alternative, "could not extract alternative node for if expression"); });

        alternative_statements = dc_dv_as(dc_unwrap2(alternative), DNodeBlockStatement).statements;
    }

    // push condition to the pool
    dc_try_fail_temp(DCResVoid, dc_da_push(p->pool, dc_unwrap2(condition)));

    dc_ret_ok_dv(DNodeIfExpression, (dn_if(pool_last_el(p), consequence_statements, alternative_statements)));
}

static DCRes parse_prefix_expression(DParser* p)
{
    DC_RES();

    DTok tok = p->current_token;

    dc_try_fail_temp(DCResVoid, next_token(p));

    dang_parser_location_preserve(p);
    DCRes right = parse_expression(p, PREC_PREFIX);
    dang_parser_location_revert(p);

    dc_ret_if_err2(right, { dc_err_dbg_log2(right, "could not parse right hand side"); });

    string op = NULL;
    dc_try_fail_temp(DCResUsize, dc_sprintf(&op, DCPRIsv, dc_sv_fmt(tok.text)));

    dc_try_or_fail_with3(DCResVoid, res, dc_da_push(p->pool, dc_dva(string, op)), if (op) free(op));

    dc_try_or_fail_with2(res, dc_da_push(p->pool, dc_unwrap2(right)), if (op) free(op));

    dc_ret_ok_dv(DNodePrefixExpression, dn_prefix(op, pool_last_el(p)));
}

/**
 * Infix Expressions: - + * / == < etc.
 */
static DCRes parse_infix_expression(DParser* p, DCDynValPtr left)
{
    DC_RES();

    string op = NULL;
    dc_try_fail_temp(DCResUsize, dc_sprintf(&op, DCPRIsv, dc_sv_fmt(p->current_token.text)));

    Precedence prec = current_prec(p);

    DCResVoid res = next_token(p);
    dc_ret_if_err2(res, {
        dc_err_dbg_log2(res, "could not move to the next token");

        if (op) free(op);
    });

    dang_parser_location_preserve(p);
    DCRes right = parse_expression(p, prec);
    dang_parser_location_revert(p);

    dc_ret_if_err2(right, {
        dc_err_dbg_log2(right, "could not parse right hand side");

        if (op) free(op);
    });

    dang_parser_dbg_log_tokens(p);

    dc_try_or_fail_with2(res, dc_da_push(p->pool, dc_dva(string, op)), if (op) free(op));

    dc_try_or_fail_with2(res, dc_da_push(p->pool, dc_unwrap2(right)), if (op) free(op));

    dc_ret_ok_dv(DNodeInfixExpression, dn_infix(op, left, pool_last_el(p)));
}

static DCRes parse_index_expression(DParser* p, DCDynValPtr operand)
{
    DC_RES();

    dc_try_fail_temp(DCResVoid, next_token(p));

    dang_parser_location_preserve(p);
    DCRes expression = parse_expression(p, PREC_LOWEST);
    dang_parser_location_revert(p);

    dc_ret_if_err2(expression, { dc_err_dbg_log2(expression, "could not parse expression hand side"); });

    dc_try_or_fail_with3(DCResVoid, res, move_if_peek_token_is(p, TOK_RBRACKET),
                         { dc_err_dbg_log2(res, "index expression must have one expression and closed with ']'"); });

    dc_try_or_fail_with2(res, dc_da_push(p->pool, dc_unwrap2(expression)), {});

    dc_ret_ok_dv(DNodeIndexExpression, dn_index(operand, pool_last_el(p)));
}

/**
 * Let Statement: 'let' identifier expression? StatementTerminator
 */
static DCRes parse_let_statement(DParser* p)
{
    DC_RES();

    /* Parsing identifier for let statement */

    // Check if the next token is an identifier or not
    dc_try_or_fail_with3(DCResVoid, res, move_if_peek_token_is(p, TOK_IDENT), { dc_err_dbg_log2(res, "Identifier needed"); });

    // Try to create a new identifier node
    dc_try_or_fail_with3(DCRes, temp_node, parse_identifier(p), { dc_err_dbg_log2(temp_node, "could not parse name"); });

    DNodeIdentifier ident = dc_dv_as(dc_unwrap2(temp_node), DNodeIdentifier);

    // If the let statement doesn't have initial value just return.
    if (token_is_end_of_the_statement(p, peek) || peek_token_is(p, TOK_EOF))
    {
        dc_try_or_fail_with2(res, next_token(p), { dc_err_dbg_log2(res, "could not move to the next token"); });

        dc_ret_ok_dv(DNodeLetStatement, dn_let(ident.value, NULL)); // return successfully
    }

    /* Parsing initial value */

    // Move to the next token
    dc_try_or_fail_with2(res, next_token(p), { dc_err_dbg_log2(res, "could not move to the next token"); });

    // Try to parse expression as initial value
    dc_try_or_fail_with2(temp_node, parse_expression(p, PREC_LOWEST), { dc_err_dbg_log2(temp_node, "could not parse value"); });

    dang_parser_dbg_log_tokens(p);

    // Based on the current location a proper terminator must be seen
    if (token_is_end_of_the_statement(p, peek) || peek_token_is(p, TOK_EOF))
    {
        dc_try_or_fail_with2(res, next_token(p), { dc_err_dbg_log2(res, "could not move to the next token"); });

        // push the temp_node that is the initial value to the pool
        dc_try_fail_temp(DCResVoid, dc_da_push(p->pool, dc_unwrap2(temp_node)));

        dc_ret_ok_dv(DNodeLetStatement, dn_let(ident.value, pool_last_el(p))); // return successfully
    }

    dc_ret_ea(-1, "end of statement needed, got token of type %s.", tostr_DTokType(p->peek_token.type));
}

/**
 * Return Statement: 'let' expression? StatementTerminator
 */
static DCRes parse_return_statement(DParser* p)
{
    DC_RES();

    // Check if it's a return without a value
    if (token_is_end_of_the_statement(p, peek) || peek_token_is(p, TOK_EOF))
    {
        dc_try_or_fail_with3(DCResVoid, res, next_token(p), { dc_err_dbg_log2(res, "could not move to the next token"); });

        dc_ret_ok_dv(DNodeReturnStatement, dn_return(NULL)); // return successfully
    }

    /* Try to parse return value (expression) */

    dc_try_or_fail_with3(DCResVoid, res, next_token(p), { dc_err_dbg_log2(res, "could not move to the next token"); });

    dc_try_or_fail_with3(DCRes, value, parse_expression(p, PREC_LOWEST), { dc_err_dbg_log2(res, "could not parse value"); });

    dang_parser_dbg_log_tokens(p);

    // Based on the current location a proper terminator must be seen
    if (token_is_end_of_the_statement(p, peek) || peek_token_is(p, TOK_EOF))
    {
        dc_try_or_fail_with2(res, next_token(p), { dc_err_dbg_log2(res, "could not move to the next token"); });

        // push the value that is the initial value to the pool
        dc_try_fail_temp(DCResVoid, dc_da_push(p->pool, dc_unwrap2(value)));

        dc_ret_ok_dv(DNodeReturnStatement, dn_return(pool_last_el(p))); // return successfully
    }

    dc_ret_ea(-1, "end of statement needed, got token of type %s.", tostr_DTokType(p->peek_token.type));
}

static DCRes parse_block_statement(DParser* p)
{
    DC_RES();

    // Try to bypass the '{'
    dc_try_fail_temp(DCResVoid, next_token(p));

    dc_try_or_fail_with3(DCResDa, statements_res, dc_da_new(NULL), {});

    DCDynArrPtr statements = dc_unwrap2(statements_res);

    DCResVoid res;

    while (current_token_is_not(p, TOK_RBRACE) && current_token_is_not(p, TOK_EOF))
    {
        /* Bypassing all the meaningless newlines and semicolons */
        try_bypassing_all_sc_and_nls_or_fail_with(p, { dc_err_dbg_log2(res, "could move to the next token"); });

        // Enter the block
        dang_parser_location_set(p, LOC_BLOCK);

        DCRes stmt = parse_statement(p);
        dc_ret_if_err2(stmt, { dc_err_dbg_log2(stmt, "cannot parse block statement"); });

        dang_parser_dbg_log_tokens(p);

        res = dc_da_push(statements, dc_unwrap2(stmt));
        dc_ret_if_err2(res, {
            dc_err_dbg_log2(res, "could not push the statement to the block");

            dc_try_fail_temp(DCResVoid, dc_da_free(statements));
            free(statements);
        });

        /* Bypassing all the meaningless newlines and semicolons */
        try_bypassing_all_sc_and_nls_or_fail_with(p, {
            dc_err_dbg_log2(res, "could move to the next token");

            dc_try_fail_temp(DCResVoid, dc_da_free(statements));
            free(statements);
        });
    }

    if (current_token_is(p, TOK_EOF))
    {
        dc_try_fail_temp(DCResVoid, dc_da_free(statements));
        free(statements);

        dc_ret_e(-1, "block ended with EOF, expected '}' instead");
    }

    // push the statements to the pool
    dc_try_or_fail_with2(res, dc_da_push(p->pool, dc_dva(DCDynArrPtr, statements)), {
        dc_try_fail_temp(DCResVoid, dc_da_free(statements));
        free(statements);
    });

    dc_ret_ok_dv(DNodeBlockStatement, dn_block(statements));
}

/**
 * Parsing the expression which is going to be used by other expressions or statement
 *
 * Expressions: prefix, infix, if, etc.
 */
static DCRes parse_expression(DParser* p, Precedence precedence)
{
    DC_RES();

    ParsePrefixFn prefix = parse_prefix_fns[p->current_token.type];

    if (!prefix)
    {
        dc_dbg_log(unexpected_token_err_fmt(p->current_token.type));

        dc_ret_ea(1, unexpected_token_err_fmt(p->current_token.type));
    }

    dang_parser_location_preserve(p);
    DCRes left_exp = prefix(p);
    dang_parser_location_revert(p);

    if (dc_is_err2(left_exp)) return left_exp;

    dang_parser_dbg_log_tokens(p);
    dc_dbg_log("precedence %d, peek precedence %d", precedence, peek_prec(p));

    while (!token_is_end_of_the_statement(p, peek) && peek_token_is_not(p, TOK_EOF) && peek_token_is_not(p, TOK_COMMA) &&
           precedence < peek_prec(p))
    {
        ParseInfixFn infix = parse_infix_fns[p->peek_token.type];
        if (!infix) return left_exp;

        dc_try_or_fail_with3(DCResVoid, res, next_token(p), { dc_err_dbg_log2(res, "could not move to the next token"); });

        // push the left to the pool
        dc_try_or_fail_with2(res, dc_da_push(p->pool, dc_unwrap2(left_exp)), {});

        dang_parser_location_preserve(p);
        left_exp = infix(p, pool_last_el(p));
        dang_parser_location_revert(p);
    }

    return left_exp;
}

/**
 * Parsing expression as an statement
 *
 * Basically a function call (command) or other expressions
 */
static DCRes parse_expression_statement(DParser* p)
{
    DC_RES();

    dang_parser_location_preserve(p);

    DCResVoid res;

    DCRes callee = parse_expression(p, PREC_LOWEST);
    dang_parser_location_revert(p);

    dc_ret_if_err2(callee, {});

    // go to next token and bypass if comma is seen
    dc_try_or_fail_with2(res, next_token(p), {});
    if (current_token_is(p, TOK_COMMA)) dc_try_or_fail_with2(res, next_token(p), {});

    // if this is not a call just return the callee itself as a node
    if ((token_is_end_of_the_statement(p, current) && current_token_is_not(p, TOK_SEMICOLON)) || current_token_is(p, TOK_EOF))
        dc_ret_ok(dc_unwrap2(callee));

    DCResDa params_res = parse_expression_list(p);
    dang_parser_location_revert(p);

    dc_ret_if_err2(res, {});

    DCDynArrPtr params = dc_unwrap2(params_res);

    check_quote(callee, params_res);
    dc_ret_if_err({});

    // push the callee to the pool
    dc_try_fail_temp(DCResVoid, dc_da_push(p->pool, dc_unwrap2(callee)));

    dc_ret_ok_dv(DNodeCallExpression, dn_call(pool_last_el(p), params));
}

/**
 * Parses an statement to its proper statement terminator ';' '\n' '}' ')', etc.
 *
 * They suppose to move to the next token after their terminator already
 */
static DCRes parse_statement(DParser* p)
{
    DC_RES();

    dang_parser_location_preserve(p);

    DCRes result;

    switch (p->current_token.type)
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

    if (dc_is_err2(result)) try_moving_to_the_end_of_statement(p);

    dang_parser_location_revert(p);

    return result;
}

/**
 * Program: one or more statement
 *
 * On each loop it tries to parse one statement and save it if it's a success otherwise
 * It already has logged its corresponding error(s)
 *
 * On each iteration the cursor must be at the end of the current statement (sentence terminator)
 */
static ResDNodeProgram parser_parse_program(DParser* p)
{
    DC_RES2(ResDNodeProgram);

    dc_try_or_fail_with3(DCResDa, statements_res, dc_da_new2(30, 3, NULL), {});

    DCDynArrPtr statements = dc_unwrap2(statements_res);

    DCResVoid res = {0};

    while (true)
    {
        /* Bypassing all the meaningless newlines and semicolons */
        while (current_token_is(p, TOK_SEMICOLON) || current_token_is(p, TOK_NEWLINE))
        {
            res = next_token(p);
            if (dc_is_err2(res))
            {
                dc_err_dbg_log2(res, "could not move to the next token");

                add_error(p, &dc_err2(res));

                break;
            }
        }

        /* Break out of the loop if the cursor is at the end */
        if (current_token_is(p, TOK_EOF)) break;

        /* Do actual statement parsing */
        DCRes stmt = parse_statement(p);

        if (dc_is_ok2(stmt))
        {
            // stmt is a DCDynValPtr to the actual dynamic value in the pool
            // or it might be actual value if the value doesn't worth to be saved
            res = dc_da_push(statements, dc_unwrap2(stmt));
            if (dc_is_err2(res))
            {
                dc_err_dbg_log2(res, "could not push the statement to program");

                add_error(p, &dc_err2(res));
            }

            continue;
        }

        add_error(p, &dc_err2(stmt));
    }

    // all errors are saved in the parser's errors field
    if (dang_parser_has_error(p))
    {
        dc_try_fail_temp(DCResVoid, dc_da_free(statements));
        free(statements);

        dc_ret_e(-1, "parser has error");
    }

    // push the program statements in the pool
    dc_da_push(p->pool, dc_dva(DCDynArrPtr, statements));

    dc_ret_ok(dn_program(statements));
}

// ***************************************************************************************
// * PUBLIC FUNCTIONS
// ***************************************************************************************

ResDNodeProgram dang_parse(DParser* p, const string source)
{
    DC_RES2(ResDNodeProgram);

    dc_try_fail_temp(DCResVoid, dang_scanner_init(&p->scanner, source));

    // Starting from body
    p->loc = LOC_BODY;

    p->current_token.type = TOK_TYPE_MAX;
    p->peek_token.type = TOK_TYPE_MAX;

    // Update current and peek tokens
    DCResVoid res;

    res = next_token(p);
    if (dc_is_err2(res))
    {
        dc_err_dbg_log2(res, "could not push the statement to program");

        add_error(p, &dc_err2(res));
    }

    res = next_token(p);
    if (dc_is_err2(res))
    {
        dc_err_dbg_log2(res, "could not push the statement to program");

        add_error(p, &dc_err2(res));
    }

    if (dang_parser_has_error(p)) dc_ret_e(-1, "parser has error");

    return parser_parse_program(p);
}

DCResVoid dang_parser_init(DParser* p, DCDynArrPtr pool, DCDynArrPtr errors)
{
    DC_RES_void();

    if (!p || !pool || !errors)
    {
        dc_dbg_log("DParser or DScanner cannot be NULL");

        dc_ret_e(1, "DParser or DScanner cannot be NULL");
    }

    p->scanner = (DScanner){0};
    if (pool->cap == 0) dc_try_fail(dc_da_init2(pool, 50, 3, NULL));
    if (errors->cap == 0) dc_try_fail(dc_da_init2(errors, 20, 2, NULL));

    p->pool = pool;
    p->errors = errors;

    p->current_token.type = TOK_TYPE_MAX;
    p->peek_token.type = TOK_TYPE_MAX;

    // Starting from body
    p->loc = LOC_BODY;

    // Initialize function pointers
    memset(parse_prefix_fns, 0, sizeof(parse_prefix_fns));
    memset(parse_infix_fns, 0, sizeof(parse_infix_fns));

    parse_prefix_fns[TOK_IDENT] = parse_identifier;
    parse_prefix_fns[TOK_STRING] = parse_string_literal;
    parse_prefix_fns[TOK_INT] = parse_integer_literal;
    parse_prefix_fns[TOK_BANG] = parse_prefix_expression;
    parse_prefix_fns[TOK_MINUS] = parse_prefix_expression;
    parse_prefix_fns[TOK_TRUE] = parse_boolean_literal;
    parse_prefix_fns[TOK_FALSE] = parse_boolean_literal;
    parse_prefix_fns[TOK_LPAREN] = parse_grouped_expression;
    parse_prefix_fns[TOK_LBRACE] = parse_hash_literal;
    parse_prefix_fns[TOK_LBRACKET] = parse_array_literal;
    parse_prefix_fns[TOK_IF] = parse_if_expression;
    parse_prefix_fns[TOK_FUNCTION] = parse_function_literal;
    parse_prefix_fns[TOK_MACRO] = parse_macro;
    parse_prefix_fns[TOK_DOLLAR_LBRACE] = parse_call_expression;

    // Illegal tokens that are supposed to be bypassed already
    parse_prefix_fns[TOK_EOF] = parse_illegal;
    parse_prefix_fns[TOK_ILLEGAL] = parse_illegal;
    parse_prefix_fns[TOK_COMMA] = parse_illegal;
    parse_prefix_fns[TOK_NEWLINE] = parse_illegal;
    parse_prefix_fns[TOK_SEMICOLON] = parse_illegal;
    parse_prefix_fns[TOK_RBRACE] = parse_illegal;
    parse_prefix_fns[TOK_RPAREN] = parse_illegal;
    parse_prefix_fns[TOK_RBRACKET] = parse_illegal;

    parse_infix_fns[TOK_PLUS] = parse_infix_expression;
    parse_infix_fns[TOK_MINUS] = parse_infix_expression;
    parse_infix_fns[TOK_SLASH] = parse_infix_expression;
    parse_infix_fns[TOK_ASTERISK] = parse_infix_expression;
    parse_infix_fns[TOK_EQ] = parse_infix_expression;
    parse_infix_fns[TOK_NEQ] = parse_infix_expression;
    parse_infix_fns[TOK_LT] = parse_infix_expression;
    parse_infix_fns[TOK_GT] = parse_infix_expression;
    parse_infix_fns[TOK_LBRACKET] = parse_index_expression;

    dc_ret();
}

void dang_parser_log_errors(DParser* p)
{
    dc_da_for(error_print_loop, *(p->errors), {
        string error = dc_dv_as(*_it, string);
        dc_log(dc_colorize_fg(LRED, "%s"), error);
    });
}
