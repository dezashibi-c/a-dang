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

static ResultDNode parse_expression(Parser* p, Precedence precedence);
static ResultDNode parse_block_statement(Parser* p);
static ResultDNode parse_statement(Parser* p);

// ***************************************************************************************
// * PRIVATE FUNCTIONS
// ***************************************************************************************

#define current_token_is(P, TYPE) ((P)->current_token->type == TYPE)
#define current_token_is_not(P, TYPE) ((P)->current_token->type != TYPE)

#define current_token_is_end_of_stmt(P)                                                                                        \
    (current_token_is(P, TOK_SEMICOLON) || current_token_is(P, TOK_NEWLINE) || current_token_is(P, TOK_EOF))

#define current_token_is_nl_or_sc(P) (current_token_is(P, TOK_SEMICOLON) || current_token_is(P, TOK_NEWLINE))

#define peek_token_is(P, TYPE) ((P)->peek_token->type == TYPE)
#define peek_token_is_not(P, TYPE) ((P)->peek_token->type != TYPE)

#define peek_token_is_end_of_stmt(P)                                                                                           \
    (peek_token_is(P, TOK_COMMA) || peek_token_is(P, TOK_SEMICOLON) || peek_token_is(P, TOK_NEWLINE) ||                        \
     peek_token_is(P, TOK_EOF))

#define peek_token_is_end_of_stmt2(P)                                                                                          \
    (peek_token_is(P, TOK_SEMICOLON) || peek_token_is(P, TOK_NEWLINE) || peek_token_is(P, TOK_EOF))

#define peek_prec(P) get_precedence((P)->peek_token->type)
#define current_prec(P) get_precedence((P)->current_token->type)

#define parser_location_preserve(P) ParserStatementLoc __parser_loc_snapshot = (P)->current_loc
#define parser_location_revert(P) (P)->current_loc = __parser_loc_snapshot
#define parser_location_set(P, LOC) (P)->current_loc = LOC

#define next_token_err_fmt(P, TYPE)                                                                                            \
    "expected next token to be %s, got %s instead.", tostr_DTokenType(TYPE), tostr_DTokenType(P->peek_token->type)

#define current_token_err_fmt(P, TYPE)                                                                                         \
    "expected current token to be %s, got %s instead.", tostr_DTokenType(TYPE), tostr_DTokenType(P->current_token->type)

#define unexpected_token_err_fmt(TYPE) "unexpected token '%s' is detected.", tostr_DTokenType(TYPE)

static DCResultVoid next_token(Parser* p)
{
    DC_RES_void();

    p->current_token = p->peek_token;

    ResultToken res = scanner_next_token(p->scanner);
    dc_res_fail_if_err2(res);

    p->peek_token = dc_res_val2(res);

    dc_res_ret();
}

static DCResultVoid next_token_if_not_eof(Parser* p)
{
    DC_RES_void();

    if (current_token_is_not(p, TOK_EOF)) return next_token(p);

    dc_res_ret();
}

static void add_error(Parser* p, DCError* err)
{
    dc_da_push(&p->errors, (err->allocated ? dc_dva(string, err->message) : dc_dv(string, err->message)));
}

static bool current_token_is_end_of_the_statement(Parser* p)
{
    bool is_cur_tok_terminator = false;
    for (DTokenType* dtt = p->terminators[p->current_loc]; *dtt != TOK_TYPE_MAX; ++dtt)
    {
        if (current_token_is(p, *dtt))
        {
            is_cur_tok_terminator = true;
            break;
        }
    }

    return is_cur_tok_terminator;
}

static bool peek_token_is_end_of_the_statement(Parser* p)
{
    bool is_peek_tok_terminator = false;
    for (DTokenType* dtt = p->terminators[p->current_loc]; *dtt != TOK_TYPE_MAX; ++dtt)
    {
        dc_dbg_log("checking: %s", tostr_DTokenType(*dtt));

        if (peek_token_is(p, *dtt))
        {
            is_peek_tok_terminator = true;
            break;
        }
    }

    return is_peek_tok_terminator;
}

static DCResultVoid move_to_the_end_of_statement(Parser* p)
{
    DC_RES_void();

    for (DTokenType* dtt = p->terminators[p->current_loc]; *dtt != TOK_TYPE_MAX; ++dtt)
    {
        if (current_token_is(p, *dtt)) dc_res_ret();
    }

    while (true)
    {
        bool is_next_token_a_terminator = false;
        for (DTokenType* dtt = p->terminators[p->current_loc]; *dtt != TOK_TYPE_MAX; ++dtt)
        {
            if (peek_token_is(p, *dtt))
            {
                is_next_token_a_terminator = true;
                break;
            }
        }

        if (is_next_token_a_terminator)
        {
            dc_try_fail(next_token_if_not_eof(p));

            continue;
        }

        break;
    }

    dc_res_ret();
}

static DCResultVoid move_if_peek_token_is(Parser* p, DTokenType type)
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

        default:
            break;
    };

    return PREC_LOWEST;
}

static ResultDNode parse_identifier(Parser* p)
{
    return dn_new(DN_IDENTIFIER, p->current_token, false);
}

static ResultDNode parse_integer_literal(Parser* p)
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

static ResultDNode parse_boolean_literal(Parser* p)
{
    DC_TRY_DEF2(ResultDNode, dn_new(DN_BOOLEAN_LITERAL, p->current_token, true));

    DCResultVoid res = dn_val_push(dc_res_val(), u8, (p->current_token->type == TOK_TRUE));
    dc_res_ret_if_err2(res, {
        dc_res_err_dbg_log2(res, "could not push the value to expression");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
    });

    dc_res_ret();
}

static DCResultVoid parse_function_params(Parser* p, DNode* parent_node)
{
    DC_RES_void();

    if (peek_token_is(p, TOK_RPAREN))
    {
        dc_try_fail(next_token(p));
        dc_res_ret();
    }

    dc_try_fail(next_token(p));

    while (true)
    {
        ResultDNode ident = parse_identifier(p);
        dc_res_ret_if_err2(ident, {});

        DCResultVoid res = dn_child_push(parent_node, dc_res_val2(ident));
        dc_res_ret_if_err2(res, dc_try_fail(dn_free(dc_res_val2(ident))));

        dc_try_fail(next_token(p));
        if (current_token_is(p, TOK_COMMA))
            dc_try_fail(next_token(p));
        else if (current_token_is(p, TOK_RPAREN) || current_token_is(p, TOK_EOF))
            break;
    }

    if (current_token_is_not(p, TOK_RPAREN)) dc_res_ret_ea(-1, current_token_err_fmt(p, TOK_RPAREN));

    dc_res_ret();
}

/**
 * Function literal: 'fn' '(' (identifier (,)?)*  ')' '{' statement* '}'
 */
static ResultDNode parse_function_literal(Parser* p)
{
    DC_RES2(ResultDNode);

    DToken* tok = p->current_token;

    dc_try_fail_temp(DCResultVoid, move_if_peek_token_is(p, TOK_LPAREN));

    dc_try_fail(dn_new(DN_FUNCTION_LITERAL, tok, true));

    DCResultVoid res = parse_function_params(p, dc_res_val());
    dc_res_ret_if_err2(res, {
        dc_res_err_dbg_log2(res, "could not parse function literal params");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
    });

    res = move_if_peek_token_is(p, TOK_LBRACE);
    dc_res_ret_if_err2(res, {
        dc_res_err_dbg_log2(res, "function literal needs body");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
    });

    parser_location_preserve(p);
    ResultDNode body = parse_block_statement(p);
    parser_location_revert(p);

    dc_res_ret_if_err2(body, {
        dc_res_err_dbg_log2(body, "could not parse function literal body");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
    });

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
 */
static DCResultVoid parse_call_params(Parser* p, DNode* parent_node)
{
    DC_RES_void();

    if (peek_token_is(p, TOK_RPAREN)) dc_res_ret();

    dc_try_fail(next_token(p));

    while (!peek_token_is_end_of_the_statement(p))
    {
        ResultDNode param = parse_expression(p, PREC_LOWEST);

        dc_res_ret_if_err2(param, {});

        DCResultVoid res = dn_child_push(parent_node, dc_res_val2(param));
        dc_res_ret_if_err2(res, dc_try_fail(dn_free(dc_res_val2(param))));

        dc_dbg_log("curr tt=%s, next tt=%s, nt=%s", tostr_DTokenType(p->current_token->type),
                   tostr_DTokenType(p->peek_token->type), tostr_DNType(dc_res_val2(param)->type));

        dc_try_fail(next_token_if_not_eof(p));
        if (current_token_is(p, TOK_COMMA)) dc_try_fail(next_token(p));
    }

    dc_res_ret();
}

/**
 * Grouped expressions can be an infix grouped to be prioritized or a call expression
 */
static ResultDNode parse_grouped_expression(Parser* p)
{
    DC_RES2(ResultDNode);

    dc_try_fail_temp(DCResultVoid, next_token(p));

    parser_location_preserve(p);
    dc_try(parse_expression(p, PREC_LOWEST));
    parser_location_revert(p);

    dc_res_ret_if_err();

    // Check if it's a grouped expression
    if (dc_res_val()->type != DN_CALL_EXPRESSION && dc_res_val()->type != DN_IDENTIFIER)
    {
        dc_try_fail_temp(DCResultVoid, move_if_peek_token_is(p, TOK_RPAREN));
        dc_res_ret();
    }

    /* It's not a grouped expression so it's a call */

    ResultDNode call_node = dn_new(DN_CALL_EXPRESSION, NULL, true);
    dc_res_ret_if_err2(call_node, dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val())));

    // Pushing the retrieved node in the main result to the call node as the first child
    DCResultVoid res = dn_child_push(dc_res_val2(call_node), dc_res_val());
    dc_res_ret_if_err2(res, {
        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val2(call_node)));
        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
    });

    // Otherwise it's a function call
    // It continues until it gets EOF or RPAREN
    parser_location_set(p, LOC_GROUP);
    res = parse_call_params(p, dc_res_val2(call_node));
    parser_location_revert(p);

    dc_res_ret_if_err2(res, dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val2(call_node))));

    // In case it hasn't got RPAREN it's wrong (unclosed paren group)
    if (current_token_is_not(p, TOK_RPAREN))
    {
        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val2(call_node)));
        dc_res_ret_ea(-1, current_token_err_fmt(p, TOK_RPAREN));
    }

    return call_node;
}

/**
 * If Expression: In this language if is an expression meaning it can return values
 *                'if' expression '{' statement* '}' ('else' '{' statement* '}')?
 */
static ResultDNode parse_if_expression(Parser* p)
{
    DC_RES2(ResultDNode);

    // hold ('if') and try to move next
    DToken* tok = p->current_token;
    dc_try_fail_temp(DCResultVoid, next_token(p));

    dc_try_fail(dn_new(DN_IF_EXPRESSION, tok, true));

    /* Getting the condition expression */
    parser_location_preserve(p);
    ResultDNode condition = parse_expression(p, PREC_LOWEST);
    parser_location_revert(p);

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
    parser_location_revert(p);

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

    if (peek_token_is(p, TOK_ELSE))
    {
        res = next_token(p);
        dc_res_ret_if_err2(res, {
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
        parser_location_revert(p);

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

static ResultDNode parse_prefix_expression(Parser* p)
{
    DC_RES2(ResultDNode);

    DToken* tok = p->current_token;

    dc_try_fail_temp(DCResultVoid, next_token(p));

    parser_location_preserve(p);
    ResultDNode right = parse_expression(p, PREC_PREFIX);
    parser_location_revert(p);

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
static ResultDNode parse_infix_expression(Parser* p, DNode* left)
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

    parser_location_preserve(p);
    ResultDNode right = parse_expression(p, prec);
    parser_location_revert(p);

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

    dc_res_ret();
}

/**
 * Let Statement: 'let' identifier expression? StatementTerminator
 */
static ResultDNode parse_let_statement(Parser* p)
{
    DC_RES2(ResultDNode);

    DToken* tok = p->current_token; // capturing current token 'let'

    /* Parsing identifier for let statement */

    // Check if the next token is an identifier or not
    dc_res_try_or_fail_with3(DCResultVoid, res, move_if_peek_token_is(p, TOK_IDENT), {
        dc_res_err_dbg_log2(res, "Identifier needed");

        dc_try_fail_temp(DCResultVoid, move_to_the_end_of_statement(p));
    });

    // Try to create a new identifier node
    dc_res_try_or_fail_with3(ResultDNode, temp_node, dn_new(DN_IDENTIFIER, p->current_token, false), {
        dc_res_err_dbg_log2(temp_node, "could not parse name");

        dc_try_fail_temp(DCResultVoid, move_to_the_end_of_statement(p));
    });

    // Try to create a new let statement node
    // If successful it will be saved in the main result variable
    dc_res_try_or_fail_with(dn_new(DN_LET_STATEMENT, tok, true), {
        dc_res_err_dbg_log("could not create let statement node");
        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val2(temp_node)));

        dc_try_fail_temp(DCResultVoid, move_to_the_end_of_statement(p));
    });

    // Try to push the identifier to the let statement as the first child
    dc_res_try_or_fail_with2(res, dn_child_push(dc_res_val(), dc_res_val2(temp_node)), {
        dc_res_err_dbg_log2(res, "could not push the name to statement");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val2(temp_node)));

        dc_try_fail_temp(DCResultVoid, move_to_the_end_of_statement(p));
    });

    // If the let statement doesn't have initial value just return.
    if (peek_token_is_end_of_the_statement(p))
    {
        dc_res_try_or_fail_with2(res, next_token(p), {
            dc_res_err_dbg_log2(res, "could not move to the next token");

            dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        });

        dc_res_ret(); // return successfully
    }

    /* Parsing initial value */

    // Move to the next token
    dc_res_try_or_fail_with2(res, next_token(p), {
        dc_res_err_dbg_log2(res, "could not move to the next token");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));

        dc_try_fail_temp(DCResultVoid, move_to_the_end_of_statement(p));
    });

    // Try to parse expression as initial value
    dc_res_try_or_fail_with2(temp_node, parse_expression(p, PREC_LOWEST), {
        dc_res_err_dbg_log2(temp_node, "could not parse value");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));

        dc_try_fail_temp(DCResultVoid, move_to_the_end_of_statement(p));
    });

    // Try to push the initial value expression
    dc_res_try_or_fail_with2(res, dn_child_push(dc_res_val(), dc_res_val2(temp_node)), {
        dc_res_err_dbg_log2(res, "could not push the value to statement");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val2(temp_node)));

        dc_try_fail_temp(DCResultVoid, move_to_the_end_of_statement(p));
    });

    // Based on the current location a proper terminator must be seen
    if (peek_token_is_end_of_the_statement(p))
    {
        dc_res_try_or_fail_with2(res, next_token(p), {
            dc_res_err_dbg_log2(res, "could not move to the next token");

            dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        });

        dc_res_ret(); // return successfully
    }

    dc_res_ret_e(-1, "end of statement needed");
}

/**
 * Return Statement: 'let' expression? StatementTerminator
 */
static ResultDNode parse_return_statement(Parser* p)
{
    DC_TRY_DEF2(ResultDNode, dn_new(DN_RETURN_STATEMENT, p->current_token, true));

    // Check if it's a return without a value
    if (peek_token_is_end_of_the_statement(p))
    {
        dc_res_try_or_fail_with3(DCResultVoid, res, next_token(p), {
            dc_res_err_dbg_log2(res, "could not move to the next token");

            dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        });

        dc_res_ret(); // return successfully
    }

    /* Try to parse return value (expression) */

    dc_res_try_or_fail_with3(DCResultVoid, res, next_token(p), {
        dc_res_err_dbg_log2(res, "could not move to the next token");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));

        dc_try_fail_temp(DCResultVoid, move_to_the_end_of_statement(p));
    });

    dc_res_try_or_fail_with3(ResultDNode, value, parse_expression(p, PREC_LOWEST), {
        dc_res_err_dbg_log2(res, "could not parse value");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));

        dc_try_fail_temp(DCResultVoid, move_to_the_end_of_statement(p));
    });

    dc_dbg_log("token=%s, next_tok=%s", tostr_DTokenType(p->current_token->type), tostr_DTokenType(p->peek_token->type));

    dc_res_try_or_fail_with2(res, dn_child_push(dc_res_val(), dc_res_val2(value)), {
        dc_res_err_dbg_log2(res, "could not push the value to statement");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val2(value)));

        dc_try_fail_temp(DCResultVoid, move_to_the_end_of_statement(p));
    });

    // Based on the current location a proper terminator must be seen
    if (peek_token_is_end_of_the_statement(p))
    {
        dc_res_try_or_fail_with2(res, next_token(p), {
            dc_res_err_dbg_log2(res, "could not move to the next token");

            dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        });

        dc_res_ret(); // return successfully
    }

    dc_res_ret_e(-1, "end of statement needed");
}

static ResultDNode parse_block_statement(Parser* p)
{
    DC_RES2(ResultDNode);

    DToken* tok = p->current_token;

    // Try to bypass the '{'
    dc_try_fail_temp(DCResultVoid, next_token(p));

    dc_try_fail(dn_new(DN_BLOCK_STATEMENT, tok, true));

    DCResultVoid res;

    // Enter the block
    parser_location_set(p, LOC_BLOCK);

    while (current_token_is_not(p, TOK_RBRACE))
    {
        if (current_token_is(p, TOK_EOF)) dc_res_ret_e(-1, "block ended with EOF, expected '}' instead");

        ResultDNode stmt = parse_statement(p);
        dc_res_ret_if_err2(stmt, {
            dc_res_err_dbg_log2(stmt, "cannot parse block statement");

            dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        });

        res = dn_child_push(dc_res_val(), dc_res_val2(stmt));
        dc_res_ret_if_err2(res, {
            dc_res_err_dbg_log2(res, "could not push the statement to the block");

            dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
            dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val2(stmt)));
        });

        res = next_token_if_not_eof(p);
        dc_res_ret_if_err2(res, {
            dc_res_err_dbg_log2(res, "could not move to the next token");

            dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        });
    }

    dc_res_ret();
}

/**
 * Parsing the expression which is going to be used by other expressions or statement
 *
 * Expressions: prefix, infix, if, etc.
 */
static ResultDNode parse_expression(Parser* p, Precedence precedence)
{
    DC_RES2(ResultDNode);

    ParsePrefixFn prefix = p->parse_prefix_fns[p->current_token->type];

    if (!prefix)
    {
        dc_dbg_log(unexpected_token_err_fmt(p->current_token->type));

        dc_res_ret_ea(1, unexpected_token_err_fmt(p->current_token->type));
    }

    parser_location_preserve(p);
    ResultDNode left_exp = prefix(p);
    parser_location_revert(p);

    while (!peek_token_is_end_of_the_statement(p) && precedence < peek_prec(p))
    {
        ParseInfixFn infix = p->parse_infix_fns[p->peek_token->type];
        if (!infix) return left_exp;

        dc_res_try_or_fail_with3(DCResultVoid, res, next_token(p), {
            dc_res_err_dbg_log2(res, "could not move to the next token");

            dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val2(left_exp)));
        });

        parser_location_preserve(p);
        left_exp = infix(p, dc_res_val2(left_exp));
        parser_location_revert(p);
    }

    return left_exp;
}

/**
 * Parsing expression as an statement
 *
 * Expressions: prefix, infix, if, etc.
 */
static ResultDNode parse_expression_statement(Parser* p)
{
    DC_TRY_DEF2(ResultDNode, dn_new(DN_EXPRESSION_STATEMENT, p->current_token, true));

    /* Parsing the expression for expression statement */

    dc_res_try_or_fail_with3(ResultDNode, expression, parse_expression(p, PREC_LOWEST), {
        dc_res_err_dbg_log2(expression, "could not parse expression");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));

        dc_try_fail_temp(DCResultVoid, move_to_the_end_of_statement(p));
    });

    dc_res_try_or_fail_with3(DCResultVoid, res, dn_child_push(dc_res_val(), dc_res_val2(expression)), {
        dc_res_err_dbg_log2(res, "could not push the name to statement");

        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val2(expression)));

        dc_try_fail_temp(DCResultVoid, move_to_the_end_of_statement(p));
    });

    // Based on the current location a proper terminator must be seen
    if (peek_token_is_end_of_the_statement(p))
    {
        dc_res_try_or_fail_with2(res, next_token(p), {
            dc_res_err_dbg_log2(res, "could not move to the next token");

            dc_try_fail_temp(DCResultVoid, dn_free(dc_res_val()));
        });

        dc_res_ret(); // return successfully
    }

    dc_res_ret_e(-1, "end of statement needed");
}

/**
 * Parses an statement to its proper statement terminator ';' '\n' '}' ')', etc.
 *
 * They suppose to move to the next token after their terminator already
 */
static ResultDNode parse_statement(Parser* p)
{
    parser_location_preserve(p);

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

    parser_location_revert(p);
    return result;
}

// ***************************************************************************************
// * PUBLIC FUNCTIONS
// ***************************************************************************************

DCResultVoid parser_init(Parser* p, Scanner* s)
{
    DC_RES_void();

    if (!p || !s)
    {
        dc_dbg_log("Parser or Scanner cannot be NULL");

        dc_res_ret_e(1, "Parser or Scanner cannot be NULL");
    }

    p->scanner = s;

    p->current_token = NULL;
    p->peek_token = NULL;

    dc_try(dc_da_init(&p->errors, NULL));

    // Initialize parsing location
    memset(p->terminators, 0, sizeof(p->terminators));

    // Starting from body
    p->current_loc = LOC_BODY;

    p->terminators[LOC_BODY][0] = TOK_SEMICOLON;
    p->terminators[LOC_BODY][1] = TOK_NEWLINE;
    p->terminators[LOC_BODY][2] = TOK_EOF;
    p->terminators[LOC_BODY][3] = TOK_TYPE_MAX;

    p->terminators[LOC_BLOCK][0] = TOK_SEMICOLON;
    p->terminators[LOC_BLOCK][1] = TOK_NEWLINE;
    p->terminators[LOC_BLOCK][2] = TOK_RBRACE;
    p->terminators[LOC_BLOCK][3] = TOK_EOF;
    p->terminators[LOC_BLOCK][4] = TOK_TYPE_MAX;

    p->terminators[LOC_GROUP][0] = TOK_SEMICOLON;
    p->terminators[LOC_GROUP][1] = TOK_NEWLINE;
    p->terminators[LOC_GROUP][2] = TOK_RPAREN;
    p->terminators[LOC_GROUP][3] = TOK_EOF;
    p->terminators[LOC_GROUP][4] = TOK_TYPE_MAX;

    // Initialize function pointers
    memset(p->parse_prefix_fns, 0, sizeof(p->parse_prefix_fns));
    memset(p->parse_infix_fns, 0, sizeof(p->parse_infix_fns));

    p->parse_prefix_fns[TOK_IDENT] = parse_identifier;
    p->parse_prefix_fns[TOK_INT] = parse_integer_literal;
    p->parse_prefix_fns[TOK_BANG] = parse_prefix_expression;
    p->parse_prefix_fns[TOK_MINUS] = parse_prefix_expression;
    p->parse_prefix_fns[TOK_TRUE] = parse_boolean_literal;
    p->parse_prefix_fns[TOK_FALSE] = parse_boolean_literal;
    p->parse_prefix_fns[TOK_LPAREN] = parse_grouped_expression;
    p->parse_prefix_fns[TOK_IF] = parse_if_expression;
    p->parse_prefix_fns[TOK_FUNCTION] = parse_function_literal;

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

DCResultVoid parser_free(Parser* p)
{
    DC_RES_void();

    dc_try(dc_da_free(&p->errors));

    dc_try(scanner_free(p->scanner));

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
ResultDNode parser_parse_program(Parser* p)
{
    DC_TRY_DEF2(ResultDNode, dn_new(DN_PROGRAM, NULL, true));

    DCResultVoid res;

    while (true)
    {
        /* Bypassing all the meaningless newlines and semicolons */
        while (current_token_is_end_of_the_statement(p))
        {
            res = next_token_if_not_eof(p);
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
        }
    }

    if (p->errors.count != 0) dc_res_ret_e(-1, "parser has error");

    dc_res_ret();
}

void parser_log_errors(Parser* p)
{
    dc_da_for(p->errors)
    {
        string error = dc_da_get_as(p->errors, _idx, string);
        dc_log("%s", error);
    }
}
