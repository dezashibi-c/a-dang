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

// ***************************************************************************************
// * PRIVATE FUNCTIONS
// ***************************************************************************************

static DObjResult eval_block_statements(DNode* dn, DEnv* de)
{
    DC_RES2(DObjResult);

    dc_res_ok(dobj_null());

    dc_da_for(dn->children)
    {
        DNode* stmt = dn_child(dn, _idx);
        dc_try_fail(dang_eval(stmt, de));

        if (dobj_is_return(dc_res_val())) break;
    }

    dc_res_ret();
}

static DObjResult eval_program_statements(DNode* dn, DEnv* de)
{
    DC_TRY_DEF2(DObjResult, eval_block_statements(dn, de));

    dc_res_val().is_returned = false;

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
        dc_res_ret_ea(-1, "'-' operator does not support right value of type '%s'", dc_tostr_dvt(&right->dv));

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
        dc_res_ret_ea(-1, "unimplemented infix operator '" DCPRIsv "' for '%s' and '%s'", dc_sv_fmt(dn_text(dn)),
                      dc_tostr_dvt(&left->dv), dc_tostr_dvt(&right->dv));
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
        dc_res_ret_ea(-1, "unimplemented infix operator '" DCPRIsv "' for '%s' and '%s'", dc_sv_fmt(dn_text(dn)),
                      dc_tostr_dvt(&left->dv), dc_tostr_dvt(&right->dv));
}

static DObjResult eval_string_infix_expression(DNode* dn, DObject* left, DObject* right)
{
    DC_RES2(DObjResult);

    string lval = dobj_as_string(*left);
    string rval = dobj_as_string(*right);

    if (dc_sv_str_eq(dn_text(dn), "+"))
    {
        string result;
        dc_sprintf(&result, "%s%s", lval, rval);
        DObject str_obj = dobj_string(result);

        dc_dv_mark_alloc(str_obj.dv);

        dc_res_ret_ok(str_obj);
    }

    if (dc_sv_str_eq(dn_text(dn), "=="))
        dc_res_ret_ok(dobj_bool(strcmp(lval, rval) == 0));

    else
        dc_res_ret_ea(-1, "unimplemented infix operator '" DCPRIsv "' for '%s' and '%s'", dc_sv_fmt(dn_text(dn)),
                      dc_tostr_dvt(&left->dv), dc_tostr_dvt(&right->dv));
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

    else if (dobj_is_string(*right) && dobj_is_string(*left))
        return eval_string_infix_expression(dn, left, right);

    else if (dobj_is_string(*left) && !dc_sv_str_eq(dn_text(dn), "=="))
    {
        DObject right2 = dobj_string(dc_res_val2(dc_tostr_dv(&right->dv)));
        return eval_string_infix_expression(dn, left, &right2);
    }

    else if (dobj_is_string(*right) && !dc_sv_str_eq(dn_text(dn), "=="))
    {
        DObject left2 = dobj_string(dc_res_val2(dc_tostr_dv(&left->dv)));
        return eval_string_infix_expression(dn, &left2, right);
    }

    else
        dc_res_ret_ea(-1, "unimplemented infix operator '" DCPRIsv "' for '%s' and '%s'", dc_sv_fmt(dn_text(dn)),
                      dc_tostr_dvt(&left->dv), dc_tostr_dvt(&right->dv));
}

static DObjResult eval_if_expression(DNode* dn, DEnv* de)
{
    DC_RES2(DObjResult);

    DNode* condition = dn_child(dn, 0);
    DNode* consequence = dn_child(dn, 1);
    DNode* alternative = (dn_child_count(dn) > 2) ? dn_child(dn, 2) : NULL;

    dc_try_or_fail_with3(DObjResult, condition_evaluated, dang_eval(condition, de), {});

    dc_try_or_fail_with3(DCResultBool, condition_as_bool, dc_dv_as_bool(&dc_res_val2(condition_evaluated).dv), {});

    if (dc_res_val2(condition_as_bool))
        return dang_eval(consequence, de);

    else if (alternative)
        return dang_eval(alternative, de);

    else
        dc_res_ret_ok(dobj_null());
}

static DObjResult eval_let_statement(DNode* dn, DEnv* de)
{
    DC_RES2(DObjResult);

    DNode* var_name = dn_child(dn, 0);
    dn_string_init(var_name);

    DObjPResult check = dang_env_get(de, var_name->text);
    if (dc_res_is_ok2(check))
    {
        dc_dbg_log("symbol '%s' is already defined", var_name->text);

        dc_res_ret_ea(-1, "symbol '%s' is already defined", var_name->text);
    }

    DNode* value_node = (dn_child_count(dn) > 1) ? dn_child(dn, 1) : NULL;

    DObject* value = NULL;
    DObjPResult val_obj_res;

    if (value_node)
    {
        dc_try_or_fail_with3(DObjResult, v_res, dang_eval(value_node, de), {});

        dc_try_or_fail_with2(val_obj_res, dang_obj_new_from(&dc_res_val2(v_res)), {});
        value = dc_res_val2(val_obj_res);

        // It doesn't matter if it was a returned value from some other function, etc.
        // It must be reset as it is now have been saved in the environment.
        value->is_returned = false;
    }

    dc_try_fail_temp(DObjPResult, dang_env_set(de, var_name->text, value));

    dc_res_ret_ok(dobj_null());
}

static DCResultVoid eval_children_nodes(DNode* call_node, DObject* parent_result, usize child_start_index, DEnv* de)
{
    DC_RES_void();

    if (dn_child_count(call_node) <= 1) dc_res_ret();

    // first child is the function identifier or expression
    // the rest is function argument
    for (usize i = child_start_index; i < dn_child_count(call_node); ++i)
    {
        dc_try_or_fail_with3(DObjResult, evaluated, dang_eval(dn_child(call_node, i), de), {});

        dc_try_or_fail_with3(DObjPResult, arg_res, dang_obj_new_from(&dc_res_val2(evaluated)), {});

        dc_try_or_fail_with3(DCResultVoid, res, dc_da_push(&parent_result->children, dc_dva(voidptr, dc_res_val2(arg_res))), {
            dc_dbg_log("failed to push object to parent object");
            dc_try_fail(dang_obj_free(dc_res_val2(arg_res)));
        });
    }

    dc_res_ret();
}

static DEnvResult extend_function_env(DObject* call_obj, DNode* fn_node)
{
    DC_TRY_DEF2(DEnvResult, dang_env_new_enclosed(call_obj->env));

    if (call_obj->children.count != dn_child_count(fn_node) - 1)
        dc_res_ret_ea(-1, "function needs at least %" PRIuMAX " arguments, got=%" PRIuMAX, dn_child_count(fn_node) - 1,
                      call_obj->children.count);

    // extending the environment by defining arguments
    // with given evaluated objects assigning to them
    dc_da_for(fn_node->children)
    {
        // we shouldn't get the last child as it is the function body
        if (_idx >= (dn_child_count(fn_node) - 1)) break;

        DNode* arg_name = dn_child(fn_node, _idx);
        dn_string_init(arg_name);

        DObject* value = NULL;

        // if the number of passed arguments are not sufficient the arguments will be defined as NULL
        if (_idx < call_obj->children.count) value = dobj_child(call_obj, _idx);

        dc_try_fail_temp(DObjPResult, dang_env_set(dc_res_val(), arg_name->text, value));
    }

    dc_res_ret();
}

/**
 * fn_obj's node is a pointer to the "fn" declaration node that has been saved in the env
 * fn_obj's node's children are arguments except the last one that is the body
 * call_obj holds all the evaluated object arguments in its children field
 * call_obj holds current env as well
 */
static DObjResult apply_function(DObject* call_obj, DNode* fn_node)
{
    DC_RES2(DObjResult);

    dc_try_or_fail_with3(DEnvResult, fn_env_res, extend_function_env(call_obj, fn_node), {});

    DNode* body = dn_child(fn_node, dn_child_count(fn_node) - 1);

    dc_try(dang_eval(body, dc_res_val2(fn_env_res)));

    // if we've returned of a function that's ok
    // but we don't need to pass it on to the upper level
    dc_res_val().is_returned = false;

    dc_res_ret();
}

// ***************************************************************************************
// * BUILTIN FUNCTIONS
// ***************************************************************************************

static DECL_DBUILTIN_FUNCTION(len)
{
    DC_RES2(DObjResult);

    if (call_obj->children.count != 1)
        dc_res_ret_ea(-1, "invalid number of argument passed to 'len', expected=1, got=%" PRIuMAX, call_obj->children.count);

    DObject* arg = dobj_child(call_obj, 0);

    if (arg->type != DOBJ_STRING) dc_res_ret_ea(-1, "cannot calculate length of '%s'", tostr_DObjType(arg->type));

    i64 len = 0;

    if (dobj_as_string(*arg)) len = (i64)strlen(dobj_as_string(*arg));

    dc_res_ret_ok(dobj_int(len));
}

DBuiltinFunction len_ptr = len;

static DObjResult find_builtin(string name)
{
    DC_RES2(DObjResult);

    if (strcmp(name, "len") == 0)
        dc_res_ret_ok(dobj_bfn(len_ptr));


    else
    {
        dc_dbg_log("key '%s' not found in the environment", name);

        dc_res_ret_ea(6, "'%s' is not defined", name);
    }
}

// ***************************************************************************************
// * PRIVATE HELPER FUNCTIONS
// ***************************************************************************************

static DC_HT_HASH_FN_DECL(string_hash)
{
    DC_RES_u32();

    string str = (string)_key;
    u32 hash = 5381;
    i32 c;
    while ((c = *str++))
    {
        hash = ((hash << 5) + hash) + c;
    }

    dc_res_ret_ok(hash);
}

static DC_HT_KEY_CMP_FN_DECL(string_key_cmp)
{
    DC_RES_bool();

    dc_res_ret_ok(strcmp((string)_key1, (string)_key2) == 0);
}

static DC_DV_FREE_FN_DECL(env_store_free)
{
    DC_RES_void();

    if (_value && _value->type == dc_dvt(voidptr))
    {
        DCHashEntry* he = (DCHashEntry*)dc_dv_as(*_value, voidptr);
        DObject* dobj = (DObject*)dc_dv_as(he->value, voidptr);

        dc_try_fail(dang_obj_free(dobj));

        free(dobj);
    }

    dc_res_ret();
}

// ***************************************************************************************
// * PUBLIC FUNCTIONS
// ***************************************************************************************

DObjResult dang_eval(DNode* dn, DEnv* de)
{
    DC_RES2(DObjResult);

    if (!dn)
        dc_res_ret_e(-1, "got NULL node");

    else if (!dn_type_is_valid(dn->type))
        dc_res_ret_ea(-1, "got invalid node type: %s", tostr_DNType(dn->type));

    switch (dn->type)
    {
        case DN_PROGRAM:
            return eval_program_statements(dn, de);

        case DN_EXPRESSION_STATEMENT:
            return dang_eval(dn_child(dn, 0), de);

        case DN_PREFIX_EXPRESSION:
        {
            dc_try_or_fail_with3(DObjResult, right, dang_eval(dn_child(dn, 0), de), {});

            return eval_prefix_expression(dn, &dc_res_val2(right));
        }

        case DN_INFIX_EXPRESSION:
        {
            dc_try_or_fail_with3(DObjResult, left, dang_eval(dn_child(dn, 0), de), {});
            dc_try_or_fail_with3(DObjResult, right, dang_eval(dn_child(dn, 1), de), {});

            return eval_infix_expression(dn, &dc_res_val2(left), &dc_res_val2(right));
        }

        case DN_BOOLEAN_LITERAL:
            dc_res_ret_ok(dobj_bool(dn_child_as(dn, 0, u8)));

        case DN_INTEGER_LITERAL:
            dc_res_ret_ok(dobj_int(dn_child_as(dn, 0, i64)));

        case DN_STRING_LITERAL:
            dn_string_init(dn);
            dc_res_ret_ok(dobj_string(dn->text));

        case DN_IDENTIFIER:
        {
            dn_string_init(dn);

            DObjPResult symbol = dang_env_get(de, dn->text);

            if (dc_res_is_ok2(symbol)) dc_res_ret_ok(*dc_res_val2(symbol));

            return find_builtin(dn->text);
        }

        case DN_BLOCK_STATEMENT:
            return eval_block_statements(dn, de);

        case DN_IF_EXPRESSION:
            return eval_if_expression(dn, de);

        case DN_RETURN_STATEMENT:
        {
            DNode* ret_val = (dn_child_count(dn) > 0) ? dn_child(dn, 0) : NULL;

            if (!ret_val) dc_res_ret_ok(dobj_return_null());

            dc_try_or_fail_with3(DObjResult, value, dang_eval(ret_val, de), {});

            dobj_mark_as_return(dc_res_val2(value));

            dc_res_ret_ok(dc_res_val2(value));
        }

        case DN_LET_STATEMENT:
            return eval_let_statement(dn, de);

        case DN_FUNCTION_LITERAL:
            dc_res_ret_ok(dobj_fn(dn, de));

        case DN_ARRAY_LITERAL:
        {
            DObject call_obj;
            dang_obj_init(&call_obj, DOBJ_ARRAY, dc_dv(voidptr, NULL), de, false, true);

            dc_try_fail_temp(DCResultVoid, eval_children_nodes(dn, &call_obj, 0, de));

            dc_res_ret_ok(call_obj);
        }

        case DN_CALL_EXPRESSION:
        {
            // function node is the first child
            DNode* fn_node = dn_child(dn, 0);

            // evaluating it must return a function object
            // that is in fact the function literal node saved in the environment before
            // it also holds pointer to its environment
            dc_try_or_fail_with3(DObjResult, fn_res, dang_eval(fn_node, de), {});

            DObject fn_obj = dc_res_val2(fn_res);

            if (fn_obj.type != DOBJ_FUNCTION && fn_obj.type != DOBJ_BUILTIN)
            {
                dc_dbg_log("not a function got: '%s'", tostr_DObjType(fn_obj.type));

                dc_res_ret_ea(-1, "not a function got: '%s'", tostr_DObjType(fn_obj.type));
            }

            // this is a temporary object to hold the evaluated children and the env
            DObject call_obj;
            dang_obj_init(&call_obj, DOBJ_NULL, dc_dv(voidptr, NULL), fn_obj.env, false, true);

            // eval arguments (first element is function symbol the rest is arguments)
            // so we start evaluating children at index 1
            dc_try_fail_temp(DCResultVoid, eval_children_nodes(dn, &call_obj, 1, de));


            if (fn_obj.type == DOBJ_BUILTIN)
            {
                DBuiltinFunction fn = dc_dv_as(fn_obj.dv, voidptr);
                return fn(&call_obj);
            }

            return apply_function(&call_obj, dobj_get_node(fn_obj));
        }

        default:
            break;
    };

    dc_res_ret_ea(-1, "Unimplemented or unsupported node type: %s", tostr_DNType(dn->type));
}

DCResultVoid dang_obj_init(DObject* dobj, DObjType dobjt, DCDynVal dv, DEnv* de, bool is_returned, bool has_children)
{
    DC_RES_void();

    dobj->type = dobjt;
    dobj->dv = dv;
    dobj->is_returned = is_returned;

    dobj->env = de;

    dobj->children = (DCDynArr){0};

    if (has_children) dc_try_fail(dc_da_init2(&dobj->children, 10, 3, dobj_child_free));

    dc_res_ret();
}


DObjPResult dang_obj_new(DObjType dobjt, DCDynVal dv, DEnv* de, bool is_returned, bool has_children)
{
    DC_RES2(DObjPResult);

    DObject* dobj = (DObject*)malloc(sizeof(DObject));
    if (!dobj)
    {
        dc_dbg_log("Memory allocation failed");

        dc_res_ret_e(2, "Memory allocation failed");
    }

    dc_try_fail_temp(DCResultVoid, dang_obj_init(dobj, dobjt, dv, de, is_returned, has_children));

    dc_res_ret_ok(dobj);
}

DObjPResult dang_obj_new_from(DObject* dobj)
{
    DC_RES2(DObjPResult);

    if (!dobj)
    {
        dc_dbg_log("Cannot initialize dang object from uninitialized objects");

        dc_res_ret_e(2, "cannot initialize dang object from uninitialized objects");
    }

    bool has_children = dobj->children.count > 0;

    dc_try_fail(dang_obj_new(dobj->type, dobj->dv, dobj->env, dobj->is_returned, has_children));

    if (has_children) dc_da_for(dobj->children) dc_da_push(&dc_res_val()->children, dc_dv(voidptr, dobj_child(dobj, _idx)));

    dc_res_ret();
}

DCResultVoid dang_obj_free(DObject* dobj)
{
    DC_TRY_DEF2(DCResultVoid, dc_dv_free(&dobj->dv, NULL));

    if (dobj->children.cap != 0) dc_try(dc_da_free(&dobj->children));

    // free the end only if 1. it is not NULL and 2. it is an enclosed env
    if (dobj->env && dobj->env->outer)
    {
        dc_try(dang_env_free(dobj->env));
        dobj->env = NULL;
    }

    dc_res_ret();
}

DC_DV_FREE_FN_DECL(dobj_child_free)
{
    DC_RES_void();

    if (dc_dv_is(*_value, voidptr) && dc_dv_as(*_value, voidptr) != NULL)
        dc_try(dang_obj_free((DObject*)dc_dv_as(*_value, voidptr)));

    dc_res_ret();
}

DEnvResult dang_env_new()
{
    DC_RES2(DEnvResult);

    DEnv* de = (DEnv*)malloc(sizeof(DEnv));
    if (de == NULL)
    {
        dc_dbg_log("Memory allocation failed");

        dc_res_ret_e(2, "Memory allocation failed");
    }

    dc_try_or_fail_with3(DCResultVoid, res, dc_ht_init(&de->store, 17, string_hash, string_key_cmp, env_store_free), {
        dc_dbg_log("cannot initialize dang environment hash table");

        free(de);
    });

    de->outer = NULL;

    dc_res_ret_ok(de);
}

DEnvResult dang_env_new_enclosed(DEnv* outer)
{
    DC_TRY_DEF2(DEnvResult, dang_env_new());

    dc_res_val()->outer = outer;

    dc_res_ret();
}

DCResultVoid dang_env_free(DEnv* de)
{
    DC_RES_void();

    dc_try_fail(dc_ht_free(&de->store));

    free(de);

    dc_res_ret();
}


DObjPResult dang_env_get(DEnv* de, string name)
{
    DC_RES2(DObjPResult);

    DCDynVal* found = NULL;

    dc_try_fail_temp(DCResultUsize, dc_ht_find_by_key(&de->store, name, &found));

    if (found) dc_res_ret_ok((DObject*)dc_dv_as(*found, voidptr));

    if (de->outer) return dang_env_get(de->outer, name);

    dc_dbg_log("key '%s' not found in the environment", name);

    dc_res_ret_ea(6, "'%s' is not defined", name);
}

DObjPResult dang_env_set(DEnv* de, string name, DObject* dobj)
{
    DC_RES2(DObjPResult);

    if (!dobj)
    {
        dc_try_or_fail_with3(DObjPResult, dobj_res, dang_obj_new(DOBJ_NULL, dc_dv(voidptr, NULL), NULL, false, false), {});

        dobj = dc_res_val2(dobj_res);
    }

    dc_try_fail_temp(DCResultVoid, dc_ht_set(&de->store, name, dc_dva(voidptr, dobj)));

    dc_res_ret_ok(dobj);
}

string tostr_DObjType(DObjType dobjt)
{
    switch (dobjt)
    {
        dc_str_case(DOBJ_INTEGER);
        dc_str_case(DOBJ_BOOLEAN);
        dc_str_case(DOBJ_STRING);
        dc_str_case(DOBJ_FUNCTION);
        dc_str_case(DOBJ_BUILTIN);
        dc_str_case(DOBJ_ARRAY);
        dc_str_case(DOBJ_NULL);

        default:
            return "(unknown evaluated object type)";
    }
}
