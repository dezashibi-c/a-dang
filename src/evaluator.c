// ***************************************************************************************
//    Project: Dang Compiler -> https://github.com/dezashibi-c/dang
//    File: evaluator.c
//    Date: 2024-10-08
//    Author: Navid Dezashibi
//    Contact: navid@dezashibi.com
//    Website: https://dezashibi.com | https://github.com/dezashibi
//    License:
//     Please refer to the LICENSE file, repository or website for more
//     information about the licensing of this work. If you have any questions
//     or concerns, please feel free to contact me at the email address provided
//     above.
// ***************************************************************************************
// *  Description: Evaluator source file
// ***************************************************************************************

#include "evaluator.h"

static DCResult eval_statements(DNode* dn)
{
    DC_RES();

    dc_da_for(dn->children) dc_try_fail(dang_eval(dn_child(dn, _idx)));

    dc_res_ret();
}

static DCResult eval_bang_operator(DCDynVal* right)
{
    DC_RES();

    dc_try_or_fail_with3(DCResultBool, right_as_bool, dc_dv_as_bool(right), {});

    dc_res_ret_ok_dv(u8, !dc_res_val2(right_as_bool));
}

static DCResult eval_minus_prefix_operator(DCDynVal* right)
{
    DC_RES();

    if (dc_dv_is_not(*right, i64))
        dc_res_ret_ea(-1, "'-' operator does not support right value of type: %s", dc_tostr_dvt(right));

    dc_res_ret_ok_dv(i64, -dc_dv_as(*right, i64));
}

static DCResult eval_prefix_expression(DNode* dn, DCDynVal* right)
{
    DC_RES();

    if (dc_sv_str_eq(dn_text(dn), "!"))
        return eval_bang_operator(right);

    else if (dc_sv_str_eq(dn_text(dn), "-"))
        return eval_minus_prefix_operator(right);

    else
        dc_res_ret_ea(-1, "unimplemented infix operator '" DCPRIsv "'", dc_sv_fmt(dn_text(dn)));
}

static DCResult eval_integer_infix_expression(DNode* dn, DCDynVal* left, DCDynVal* right)
{
    DC_RES();

    i64 lval = dc_dv_as(*left, i64);
    i64 rval = dc_dv_as(*right, i64);

    if (dc_sv_str_eq(dn_text(dn), "+"))
        dc_res_ret_ok_dv(i64, lval + rval);

    else if (dc_sv_str_eq(dn_text(dn), "-"))
        dc_res_ret_ok_dv(i64, lval - rval);

    else if (dc_sv_str_eq(dn_text(dn), "*"))
        dc_res_ret_ok_dv(i64, lval * rval);

    else if (dc_sv_str_eq(dn_text(dn), "/"))
        dc_res_ret_ok_dv(i64, lval / rval);

    else
        dc_res_ret_ea(-1, "unimplemented prefix operator '" DCPRIsv "'", dc_sv_fmt(dn_text(dn)));
}

static DCResult eval_infix_expression(DNode* dn, DCDynVal* left, DCDynVal* right)
{
    DC_RES();

    if (dc_dv_is(*left, i64) && dc_dv_is(*right, i64))
        return eval_integer_infix_expression(dn, left, right);

    else
        dc_res_ret_ea(-1, "unimplemented infix for '" DCPRIsv "' operator between '%s' and '%s'", dc_sv_fmt(dn_text(dn)),
                      dc_tostr_dvt(left), dc_tostr_dvt(right));
}

DCResult dang_eval(DNode* dn)
{
    DC_RES();

    if (!dn)
        dc_res_ret_e(-1, "got NULL node");

    else if (!dn_type_is_valid(dn->type))
        dc_res_ret_ea(-1, "got invalid node type: %s", tostr_DNType(dn->type));

    switch (dn->type)
    {
        case DN_PROGRAM:
            return eval_statements(dn);

        case DN_EXPRESSION_STATEMENT:
            return dang_eval(dn_child(dn, 0));

        case DN_PREFIX_EXPRESSION:
        {
            dc_try_or_fail_with3(DCResult, right, dang_eval(dn_child(dn, 0)), {});

            return eval_prefix_expression(dn, &dc_res_val2(right));
        }

        case DN_INFIX_EXPRESSION:
        {
            dc_try_or_fail_with3(DCResult, left, dang_eval(dn_child(dn, 0)), {});
            dc_try_or_fail_with3(DCResult, right, dang_eval(dn_child(dn, 1)), {});

            return eval_infix_expression(dn, &dc_res_val2(left), &dc_res_val2(right));
        }

        case DN_BOOLEAN_LITERAL:
            dc_res_ret_ok_dv(u8, dn_child_as(dn, 0, u8));

        case DN_INTEGER_LITERAL:
            dc_res_ret_ok_dv(i64, dn_child_as(dn, 0, i64));

        default:
            break;
    };

    dc_res_ret_ea(-1, "Unimplemented or unsupported node type: %s", tostr_DNType(dn->type));
}
