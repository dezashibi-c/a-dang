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

static DObjResult eval_statements(DNode* dn)
{
    DC_RES2(DObjResult);

    dc_da_for(dn->children)
    {
        DNode* stmt = dn_child(dn, _idx);
        dc_try_fail(dang_eval(stmt));

        if (dobj_is_return(dc_res_val())) dc_res_ret();
    }

    dc_res_ret();
}

static DObjResult eval_bang_operator(DObject* right)
{
    DC_RES2(DObjResult);

    dc_res_ret_ok(dobj_bool(!dobj_as_bool(*right)));
}

static DObjResult eval_minus_prefix_operator(DObject* right)
{
    DC_RES2(DObjResult);

    if (!dobj_is_int(*right))
        dc_res_ret_ea(-1, "'-' operator does not support right value of type: %s", dc_tostr_dvt(&right->dv));

    dc_res_ret_ok(dobj_int(-dobj_as_int(*right)));
}

static DObjResult eval_prefix_expression(DNode* dn, DObject* right)
{
    DC_RES2(DObjResult);

    if (dc_sv_str_eq(dn_text(dn), "!"))
        return eval_bang_operator(right);

    else if (dc_sv_str_eq(dn_text(dn), "-"))
        return eval_minus_prefix_operator(right);

    else
        dc_res_ret_ea(-1, "unimplemented infix operator '" DCPRIsv "'", dc_sv_fmt(dn_text(dn)));
}

static DObjResult eval_integer_infix_expression(DNode* dn, DObject* left, DObject* right)
{
    DC_RES2(DObjResult);

    i64 lval = dobj_as_int(*left);
    i64 rval = dobj_as_int(*right);

    if (dc_sv_str_eq(dn_text(dn), "+"))
        dc_res_ret_ok(dobj_int(lval + rval));

    else if (dc_sv_str_eq(dn_text(dn), "-"))
        dc_res_ret_ok(dobj_int(lval - rval));

    else if (dc_sv_str_eq(dn_text(dn), "*"))
        dc_res_ret_ok(dobj_int(lval * rval));

    else if (dc_sv_str_eq(dn_text(dn), "/"))
        dc_res_ret_ok(dobj_int(lval / rval));

    else if (dc_sv_str_eq(dn_text(dn), "<"))
        dc_res_ret_ok(dobj_bool(lval < rval));

    else if (dc_sv_str_eq(dn_text(dn), ">"))
        dc_res_ret_ok(dobj_bool(lval > rval));

    else if (dc_sv_str_eq(dn_text(dn), "=="))
        dc_res_ret_ok(dobj_bool(lval == rval));

    else if (dc_sv_str_eq(dn_text(dn), "!="))
        dc_res_ret_ok(dobj_bool(lval != rval));

    else
        dc_res_ret_ea(-1, "unimplemented prefix operator '" DCPRIsv "'", dc_sv_fmt(dn_text(dn)));
}

static DObjResult eval_boolean_infix_expression(DNode* dn, DObject* left, DObject* right)
{
    DC_RES2(DObjResult);

    bool lval = dobj_as_bool(*left);
    bool rval = dobj_as_bool(*right);

    if (dc_sv_str_eq(dn_text(dn), "=="))
        dc_res_ret_ok(dobj_bool(lval == rval));

    else if (dc_sv_str_eq(dn_text(dn), "!="))
        dc_res_ret_ok(dobj_bool(lval != rval));

    else
        dc_res_ret_ea(-1, "unimplemented prefix operator '" DCPRIsv "'", dc_sv_fmt(dn_text(dn)));
}

static DObjResult eval_infix_expression(DNode* dn, DObject* left, DObject* right)
{
    DC_RES2(DObjResult);

    if (dobj_is_int(*left) && dobj_is_int(*right))
        return eval_integer_infix_expression(dn, left, right);

    else if (dobj_is_bool(*left) && dobj_is_bool(*right))
        return eval_boolean_infix_expression(dn, left, right);

    else if (dobj_is_bool(*left))
    {
        DObject right2 = dobj_bool(dc_res_val2(dc_dv_as_bool(&right->dv)));
        return eval_boolean_infix_expression(dn, left, &right2);
    }

    else if (dobj_is_bool(*right))
    {
        DObject left2 = dobj_bool(dc_res_val2(dc_dv_as_bool(&left->dv)));
        return eval_boolean_infix_expression(dn, &left2, right);
    }

    else
        dc_res_ret_ea(-1, "unimplemented infix for '" DCPRIsv "' operator between '%s' and '%s'", dc_sv_fmt(dn_text(dn)),
                      dc_tostr_dvt(&left->dv), dc_tostr_dvt(&right->dv));
}

static DObjResult eval_if_expression(DNode* dn)
{
    DC_RES2(DObjResult);

    DNode* condition = dn_child(dn, 0);
    DNode* consequence = dn_child(dn, 1);
    DNode* alternative = (dn_child_count(dn) > 2) ? dn_child(dn, 2) : NULL;

    dc_try_or_fail_with3(DObjResult, condition_evaluated, dang_eval(condition), {});

    dc_try_or_fail_with3(DCResultBool, condition_as_bool, dc_dv_as_bool(&dc_res_val2(condition_evaluated).dv), {});

    if (dc_res_val2(condition_as_bool))
        return dang_eval(consequence);

    else if (alternative)
        return dang_eval(alternative);

    else
        dc_res_ret_ok(dobj_null());
}

DObjResult dang_eval(DNode* dn)
{
    DC_RES2(DObjResult);

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
            dc_try_or_fail_with3(DObjResult, right, dang_eval(dn_child(dn, 0)), {});

            return eval_prefix_expression(dn, &dc_res_val2(right));
        }

        case DN_INFIX_EXPRESSION:
        {
            dc_try_or_fail_with3(DObjResult, left, dang_eval(dn_child(dn, 0)), {});
            dc_try_or_fail_with3(DObjResult, right, dang_eval(dn_child(dn, 1)), {});

            return eval_infix_expression(dn, &dc_res_val2(left), &dc_res_val2(right));
        }

        case DN_BOOLEAN_LITERAL:
            dc_res_ret_ok(dobj_bool(dn_child_as(dn, 0, u8)));

        case DN_INTEGER_LITERAL:
            dc_res_ret_ok(dobj_int(dn_child_as(dn, 0, i64)));

        case DN_BLOCK_STATEMENT:
            return eval_statements(dn);

        case DN_IF_EXPRESSION:
            return eval_if_expression(dn);

        case DN_RETURN_STATEMENT:
        {
            DNode* ret_val = (dn_child_count(dn) > 0) ? dn_child(dn, 0) : NULL;

            if (!ret_val) dc_res_ret_ok(dobj_return_null());

            dc_try_or_fail_with3(DObjResult, value, dang_eval(ret_val), {});

            dc_res_ret_ok(dobj_return(dc_res_val2(value)));
        }

        // todo:: this is temporary must be fixed to actual call evaluation
        case DN_CALL_EXPRESSION:
        {
            DNode* ret_val = (dn_child_count(dn) > 0) ? dn_child(dn, 0) : NULL;

            if (!ret_val) dc_res_ret_ok(dobj_null());

            dc_try_or_fail_with3(DObjResult, value, dang_eval(ret_val), {});

            dc_res_ret_ok(dc_res_val2(value));
        }

        default:
            break;
    };

    dc_res_ret_ea(-1, "Unimplemented or unsupported node type: %s", tostr_DNType(dn->type));
}

string tostr_DObjType(DObjType dobjt)
{
    switch (dobjt)
    {
        dc_str_case(DOBJ_INTEGER);
        dc_str_case(DOBJ_BOOLEAN);
        dc_str_case(DOBJ_NULL);

        default:
            return "(unknown evaluated object type)";
    }
}