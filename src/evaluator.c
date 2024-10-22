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

DObj dobj_null = {0};

DObj dobj_null_return = {0};

DObj dobj_true = {0};

DObj dobj_false = {0};

static ResObj perform_evaluation_process(DNode* dn, DEnv* de);

// ***************************************************************************************
// * PRIVATE FUNCTIONS
// ***************************************************************************************

static ResObj get_boolean_object(bool expression)
{
    DC_RES2(ResObj);

    if (expression) dc_ret_ok(&dobj_true);

    dc_ret_ok(&dobj_false);
}

static ResObj eval_block_statements(DNode* dn, DEnv* de)
{
    DC_RES2(ResObj);

    if (dn_child_count(dn) == 0) dc_ret_ok(&dobj_null);

    dc_da_for(dn->children, {
        DNode* stmt = dn_child(dn, _idx);
        dc_try_fail(perform_evaluation_process(stmt, de));

        if (dobj_is_return(dc_unwrap())) DC_BREAK;
    });

    dc_ret();
}

static ResObj eval_program_statements(DNode* dn, DEnv* de)
{
    DC_RES2(ResObj);

    if (dn_child_count(dn) == 0) dc_ret_ok(&dobj_null);

    dc_da_for(dn->children, {
        DNode* stmt = dn_child(dn, _idx);
        dc_try_fail(perform_evaluation_process(stmt, de));

        if (stmt->type == DN_RETURN_STATEMENT) DC_BREAK;
    });

    dc_ret();
}

static ResObj eval_bang_operator(DObj* right)
{
    return get_boolean_object(!dobj_as_bool(*right));
}

static ResObj eval_minus_prefix_operator(DObj* right)
{
    DC_RES2(ResObj);

    if (!dobj_is_int(*right)) dc_ret_ea(-1, "'-' operator does not support right value of type '%s'", dc_tostr_dvt(&right->dv));

    dc_try_fail(dang_obj_new(DOBJ_INTEGER, dc_dv(i64, -dobj_as_int(*right)), NULL, false, false));

    dc_ret();
}

static ResObj eval_prefix_expression(DNode* dn, DObj* right)
{
    DC_RES2(ResObj);

    string node_text = dn_data_as(dn, string);

    if (strcmp(node_text, "!") == 0)
        return eval_bang_operator(right);

    else if (strcmp(node_text, "-") == 0)
        return eval_minus_prefix_operator(right);

    else
        dc_ret_ea(-1, "unimplemented infix operator '%s'", node_text);
}

static ResObj eval_integer_infix_expression(DNode* dn, DObj* left, DObj* right)
{
    DC_RES2(ResObj);

    i64 lval = dobj_as_int(*left);
    i64 rval = dobj_as_int(*right);

    string node_text = dn_data_as(dn, string);

    DCDynVal value = {0};

    if (strcmp(node_text, "+") == 0)
        value = dc_dv(i64, (lval + rval));

    else if (strcmp(node_text, "-") == 0)
        value = dc_dv(i64, (lval - rval));

    else if (strcmp(node_text, "*") == 0)
        value = dc_dv(i64, (lval * rval));

    else if (strcmp(node_text, "/") == 0)
        value = dc_dv(i64, (lval / rval));

    else if (strcmp(node_text, "<") == 0)
        return get_boolean_object(lval < rval);

    else if (strcmp(node_text, ">") == 0)
        return get_boolean_object(lval > rval);

    else if (strcmp(node_text, "==") == 0)
        return get_boolean_object(lval == rval);

    else if (strcmp(node_text, "!=") == 0)
        return get_boolean_object(lval != rval);

    else
        dc_ret_ea(-1, "unimplemented infix operator '%s' for '%s' and '%s'", node_text, dc_tostr_dvt(&left->dv),
                  dc_tostr_dvt(&right->dv));

    dc_try_fail(dang_obj_new(DOBJ_INTEGER, value, NULL, false, false));

    dc_ret();
}

static ResObj eval_boolean_infix_expression(DNode* dn, DObj* left, DObj* right)
{
    DC_RES2(ResObj);

    bool lval = dobj_as_bool(*left);
    bool rval = dobj_as_bool(*right);

    string node_text = dn_data_as(dn, string);

    if (strcmp(node_text, "==") == 0)
        return get_boolean_object(lval == rval);

    else if (strcmp(node_text, "!=") == 0)
        return get_boolean_object(lval != rval);

    dc_ret_ea(-1, "unimplemented infix operator '%s' for '%s' and '%s'", node_text, dc_tostr_dvt(&left->dv),
              dc_tostr_dvt(&right->dv));
}

static ResObj eval_string_infix_expression(DNode* dn, DObj* left, DObj* right)
{
    DC_RES2(ResObj);

    string lval = dobj_as_string(*left);
    string rval = dobj_as_string(*right);

    string node_text = dn_data_as(dn, string);

    if (strcmp(node_text, "+") == 0)
    {
        string result;
        dc_sprintf(&result, "%s%s", lval, rval);

        dc_try_fail(dang_obj_new(DOBJ_STRING, dc_dva(string, result), NULL, false, false));

        dc_ret();
    }

    else if (strcmp(node_text, "==") == 0)
        return get_boolean_object(strcmp(lval, rval) == 0);

    dc_ret_ea(-1, "unimplemented infix operator '%s' for '%s' and '%s'", node_text, dc_tostr_dvt(&left->dv),
              dc_tostr_dvt(&right->dv));
}

static ResObj eval_infix_expression(DNode* dn, DObj* left, DObj* right)
{
    DC_RES2(ResObj);

    string node_text = dn_data_as(dn, string);

    if (dobj_is_int(*left) && dobj_is_int(*right))
        return eval_integer_infix_expression(dn, left, right);

    else if (dobj_is_bool(*left) && dobj_is_bool(*right))
        return eval_boolean_infix_expression(dn, left, right);

    else if (dobj_is_bool(*left))
    {
        DObj right2 = dobj_bool(dc_unwrap2(dc_dv_as_bool(&right->dv)));
        return eval_boolean_infix_expression(dn, left, &right2);
    }

    else if (dobj_is_bool(*right))
    {
        DObj left2 = dobj_bool(dc_unwrap2(dc_dv_as_bool(&left->dv)));
        return eval_boolean_infix_expression(dn, &left2, right);
    }

    else if (dobj_is_string(*right) && dobj_is_string(*left))
        return eval_string_infix_expression(dn, left, right);

    else if (dobj_is_string(*left) && strcmp(node_text, "==") != 0)
    {
        DObj right2 = dobj_string(dc_unwrap2(dc_tostr_dv(&right->dv)));
        return eval_string_infix_expression(dn, left, &right2);
    }

    else if (dobj_is_string(*right) && strcmp(node_text, "==") != 0)
    {
        DObj left2 = dobj_string(dc_unwrap2(dc_tostr_dv(&left->dv)));
        return eval_string_infix_expression(dn, &left2, right);
    }

    dc_ret_ea(-1, "unimplemented infix operator '%s' for '%s' and '%s'", node_text, dc_tostr_dvt(&left->dv),
              dc_tostr_dvt(&right->dv));
}

static ResObj eval_index_expression(DObj* left, DObj* index)
{
    DC_RES2(ResObj);

    usize idx = (usize)(dobj_as_int(*index));

    if (idx > left->children.count - 1)
    {
        dc_try_fail_temp(DCResVoid, dang_obj_free(left));
        dc_try_fail_temp(DCResVoid, dang_obj_free(index));

        dc_ret_ok(&dobj_null);
    }

    dc_ret_ok(dobj_child(left, idx));
}

static ResObj eval_if_expression(DNode* dn, DEnv* de)
{
    DC_RES2(ResObj);

    DNode* condition = dn_child(dn, 0);
    DNode* consequence = dn_child(dn, 1);
    DNode* alternative = (dn_child_count(dn) > 2) ? dn_child(dn, 2) : NULL;

    dc_try_or_fail_with3(ResObj, condition_evaluated, perform_evaluation_process(condition, de), {});

    dc_try_or_fail_with3(DCResBool, condition_as_bool, dc_dv_as_bool(&dc_unwrap2(condition_evaluated)->dv), {});

    if (dc_unwrap2(condition_as_bool))
        return perform_evaluation_process(consequence, de);

    else if (alternative)
        return perform_evaluation_process(alternative, de);

    dc_ret_ok(&dobj_null);
}

static ResObj eval_let_statement(DNode* dn, DEnv* de)
{
    DC_RES2(ResObj);

    string var_name = dn_child_data_as(dn, 0, string);

    DObj* value = NULL;
    if (dn_child_count(dn) > 1)
    {
        DNode* value_node = dn_child(dn, 1);

        dc_try_or_fail_with3(ResObj, v_res, perform_evaluation_process(value_node, de), {});

        dc_try_or_fail_with3(ResObj, val_obj_res, dang_obj_copy(dc_unwrap2(v_res)), {});
        value = dc_unwrap2(val_obj_res);

        // It doesn't matter if it was a returned value from some other function, etc.
        // It must be reset as it is now have been saved in the environment.
        value->is_returned = false;
    }

    dc_try_fail_temp(ResObj, dang_env_set(de, var_name, value, false));

    dc_ret_ok(&dobj_null);
}

static DCResVoid eval_children_nodes(DNode* call_node, DObj* parent_result, usize child_start_index, DEnv* de)
{
    DC_RES_void();

    // first child is the function identifier or expression
    // the rest is function argument
    for (usize i = child_start_index; i < dn_child_count(call_node); ++i)
    {
        dc_try_or_fail_with3(ResObj, arg_res, perform_evaluation_process(dn_child(call_node, i), de), {});

        dc_try_or_fail_with3(DCResVoid, res, dc_da_push(&parent_result->children, dc_dva(DObjPtr, dc_unwrap2(arg_res))), {
            dc_dbg_log("failed to push object to parent object");
            dc_try_fail(dang_obj_free(dc_unwrap2(arg_res)));
        });
    }

    dc_ret();
}

static ResEnv extend_function_env(DObj* call_obj, DNode* fn_node)
{
    DC_TRY_DEF2(ResEnv, dang_env_new_enclosed(call_obj->env));

    if (call_obj->children.count != dn_child_count(fn_node) - 1)
        dc_ret_ea(-1, "function needs at least " dc_fmt(usize) " arguments, got=" dc_fmt(usize), dn_child_count(fn_node) - 1,
                  call_obj->children.count);

    // extending the environment by defining arguments
    // with given evaluated objects assigning to them
    dc_da_for(fn_node->children, {
        // we shouldn't get the last child as it is the function body
        if (_idx >= (dn_child_count(fn_node) - 1)) DC_BREAK;

        string arg_name = dn_child_data_as(fn_node, _idx, string);

        DObj* value = dobj_child(call_obj, _idx);

        dc_try_fail_temp(ResObj, dang_env_set(dc_unwrap(), arg_name, value, false));
    });

    dc_ret();
}

/**
 * fn_obj's node is a pointer to the "fn" declaration node that has been saved in the env
 * fn_obj's node's children are arguments except the last one that is the body
 * call_obj holds all the evaluated object arguments in its children field
 * call_obj holds current env as well
 */
static ResObj apply_function(DObj* call_obj, DNode* fn_node)
{
    DC_RES2(ResObj);

    dc_try_or_fail_with3(ResEnv, fn_env_res, extend_function_env(call_obj, fn_node),
                         dc_try_fail_temp(DCResVoid, dang_obj_free(call_obj)));

    DNode* body = dn_child(fn_node, dn_child_count(fn_node) - 1);

    dc_try(perform_evaluation_process(body, dc_unwrap2(fn_env_res)));

    // if we've returned of a function that's ok
    // but we don't need to pass it on to the upper level
    if (dc_is_ok()) dc_unwrap()->is_returned = false;

    dc_ret();
}

// ***************************************************************************************
// * BUILTIN FUNCTIONS
// ***************************************************************************************

static DECL_DBUILTIN_FUNCTION(len)
{
    DC_RES2(ResObj);

    if (call_obj->children.count != 1)
        dc_ret_ea(-1, "invalid number of argument passed to 'len', expected=1, got=%" PRIuMAX, call_obj->children.count);

    DObj* arg = dobj_child(call_obj, 0);

    if (arg->type != DOBJ_STRING && arg->type != DOBJ_ARRAY)
        dc_ret_ea(-1, "cannot calculate length of '%s'", tostr_DObjType(arg->type));

    i64 len = 0;

    if (arg->type == DOBJ_STRING)
    {
        if (dobj_as_string(*arg)) len = (i64)strlen(dobj_as_string(*arg));
    }
    else if (arg->type == DOBJ_ARRAY)
    {
        len = (i64)arg->children.count;
    }

    dc_try_fail(dang_obj_new(DOBJ_INTEGER, dc_dv(i64, len), NULL, false, false));

    dc_ret();
}

static DECL_DBUILTIN_FUNCTION(first)
{
    DC_RES2(ResObj);

    if (call_obj->children.count != 1)
        dc_ret_ea(-1, "invalid number of argument passed to 'first', expected=1, got=%" PRIuMAX, call_obj->children.count);

    DObj* arg = dobj_child(call_obj, 0);

    if (arg->type != DOBJ_ARRAY) dc_ret_ea(-1, "cannot retrieve first element of '%s'", tostr_DObjType(arg->type));

    if (arg->children.count == 0) dc_ret_ok(&dobj_null);

    dc_ret_ok(dobj_child(arg, 0));
}

static DECL_DBUILTIN_FUNCTION(last)
{
    DC_RES2(ResObj);

    if (call_obj->children.count != 1)
        dc_ret_ea(-1, "invalid number of argument passed to 'last', expected=1, got=%" PRIuMAX, call_obj->children.count);

    DObj* arg = dobj_child(call_obj, 0);

    if (arg->type != DOBJ_ARRAY) dc_ret_ea(-1, "cannot retrieve last element of '%s'", tostr_DObjType(arg->type));

    if (arg->children.count == 0) dc_ret_ok(&dobj_null);

    dc_ret_ok(dobj_child(arg, arg->children.count - 1));
}

static DECL_DBUILTIN_FUNCTION(rest)
{
    DC_RES2(ResObj);

    if (call_obj->children.count != 1)
        dc_ret_ea(-1, "invalid number of argument passed to 'rest', expected=1, got=%" PRIuMAX, call_obj->children.count);

    DObj* arg = dobj_child(call_obj, 0);

    if (arg->type != DOBJ_ARRAY) dc_ret_ea(-1, "cannot retrieve rest elements of '%s'", tostr_DObjType(arg->type));

    if (arg->children.count == 0) dc_ret_ok(&dobj_null);

    dc_try_fail(dang_obj_new(DOBJ_ARRAY, dc_dv_nullptr(), NULL, false, true));

    dc_da_for(arg->children, {
        // not the first child but the rest
        if (_idx == 0) continue;

        dc_try_or_fail_with3(ResObj, el_res, dang_obj_copy(dc_dv_as(*_it, DObjPtr)), {
            // try to free the generated array object
            dc_try_fail_temp(DCResVoid, dang_obj_free(dc_unwrap()));
        });

        dc_try_or_fail_with3(DCResVoid, temp_res, dc_da_push(&dc_unwrap()->children, dc_dva(DObjPtr, dc_unwrap2(el_res))), {
            // try to free the generated array object
            dc_try_fail_temp(DCResVoid, dang_obj_free(dc_unwrap()));
        });
    });

    dc_ret();
}

static DECL_DBUILTIN_FUNCTION(push)
{
    DC_RES2(ResObj);

    if (call_obj->children.count != 2)
        dc_ret_ea(-1, "invalid number of argument passed to 'push', expected=2, got=%" PRIuMAX, call_obj->children.count);

    DObj* arr = dobj_child(call_obj, 0);
    DObj* el = dobj_child(call_obj, 1);

    if (arr->type != DOBJ_ARRAY) dc_ret_ea(-1, "cannot push element to '%s'", tostr_DObjType(arr->type));

    dc_try_fail_temp(DCResVoid, dc_da_push(&arr->children, dc_dva(DObjPtr, el)));

    dc_ret_ok(arr);
}

static ResObj find_builtin(string name)
{
    DC_RES2(ResObj);

    DBuiltinFunction fn;

    if (strcmp(name, "len") == 0)
        fn = len;

    else if (strcmp(name, "first") == 0)
        fn = first;

    else if (strcmp(name, "last") == 0)
        fn = last;

    else if (strcmp(name, "rest") == 0)
        fn = rest;

    else if (strcmp(name, "push") == 0)
        fn = push;

    else
    {
        dc_dbg_log("key '%s' not found in the environment", name);

        dc_ret_ea(6, "'%s' is not defined", name);
    }

    dc_try_fail(dang_obj_new(DOBJ_BUILTIN, dc_dv(DBuiltinFunction, fn), NULL, false, false));

    dc_ret();
}

// ***************************************************************************************
// * PRIVATE HELPER FUNCTIONS
// ***************************************************************************************

static DC_HT_HASH_FN_DECL(string_hash)
{
    DC_RES_u32();

    if (_key->type != dc_dvt(string)) dc_ret_e(dc_e_code(TYPE), dc_e_msg(TYPE));

    string str = dc_dv_as(*_key, string);
    u32 hash = 5381;
    i32 c;
    while ((c = *str++))
    {
        hash = ((hash << 5) + hash) + c;
    }

    dc_ret_ok(hash);
}

static DC_HT_KEY_CMP_FN_DECL(string_key_cmp)
{
    return dc_dv_eq(_key1, _key2);
}

static DC_HT_KEY_VALUE_FREE_FN_DECL(env_store_free)
{
    DC_RES_void();

    if (dc_dv_is_allocated(_key_value->value))
    {
        DObj* dobj = dc_dv_as(_key_value->value, DObjPtr);

        dc_try_fail(dang_obj_free(dobj));

        free(dobj);
    }

    dc_ret();
}

// ***************************************************************************************
// * MAIN EVALUATION PROCESS
// ***************************************************************************************

static ResObj perform_evaluation_process(DNode* dn, DEnv* de)
{
    DC_RES2(ResObj);

    if (!dn)
        dc_ret_e(-1, "got NULL node");

    else if (!dn_type_is_valid(dn->type))
        dc_ret_ea(-1, "got invalid node type: %s", tostr_DNType(dn->type));

    switch (dn->type)
    {
        case DN_PROGRAM:
            return eval_program_statements(dn, de);

        case DN_EXPRESSION_STATEMENT:
            return perform_evaluation_process(dn_child(dn, 0), de);

        case DN_PREFIX_EXPRESSION:
        {
            dc_try_or_fail_with3(ResObj, right, perform_evaluation_process(dn_child(dn, 0), de), {});

            return eval_prefix_expression(dn, dc_unwrap2(right));
        }

        case DN_INFIX_EXPRESSION:
        {
            dc_try_or_fail_with3(ResObj, left, perform_evaluation_process(dn_child(dn, 0), de), {});
            dc_try_or_fail_with3(ResObj, right, perform_evaluation_process(dn_child(dn, 1), de),
                                 dc_try_fail_temp(DCResVoid, dang_obj_free(dc_unwrap2(left))));

            return eval_infix_expression(dn, dc_unwrap2(left), dc_unwrap2(right));
        }

        case DN_BOOLEAN_LITERAL:
            return get_boolean_object(!!dn_data_as(dn, u8));

        case DN_INTEGER_LITERAL:
            dc_try_fail(dang_obj_new(DOBJ_INTEGER, dn_data(dn), NULL, false, false));
            dc_ret();

        case DN_STRING_LITERAL:
            dc_try_fail(dang_obj_new(DOBJ_STRING, dn_data(dn), NULL, false, false));
            dc_ret();

        case DN_IDENTIFIER:
        {
            ResObj symbol = dang_env_get(de, dn_data_as(dn, string));

            if (dc_is_ok2(symbol)) dc_ret_ok(dc_unwrap2(symbol));

            return find_builtin(dn_data_as(dn, string));
        }

        case DN_BLOCK_STATEMENT:
            return eval_block_statements(dn, de);

        case DN_IF_EXPRESSION:
            return eval_if_expression(dn, de);

        case DN_RETURN_STATEMENT:
        {
            DNode* ret_val = (dn_child_count(dn) > 0) ? dn_child(dn, 0) : NULL;

            if (!ret_val) dc_ret_ok(&dobj_null_return);

            dc_try_or_fail_with3(ResObj, value, perform_evaluation_process(ret_val, de), {});

            dobj_mark_as_return(dc_unwrap2(value));

            dc_ret_ok(dc_unwrap2(value));
        }

        case DN_LET_STATEMENT:
            return eval_let_statement(dn, de);

        case DN_FUNCTION_LITERAL:
            // function literal holds actual node in object's dv
            // also holds the pointer to the environment it's being evaluated
            dc_try_fail(dang_obj_new(DOBJ_FUNCTION, dc_dv(DNodePtr, dn), de, false, false));
            dc_ret();

        case DN_ARRAY_LITERAL:
        {
            dc_try_fail(dang_obj_new(DOBJ_ARRAY, dc_dv_nullptr(), NULL, false, true));
            dc_try_or_fail_with3(DCResVoid, temp_res, eval_children_nodes(dn, dc_unwrap(), 0, de),
                                 dc_try_fail_temp(DCResVoid, dang_obj_free(dc_unwrap())));
            dc_ret();
        }

        case DN_INDEX_EXPRESSION:
        {
            dc_try_or_fail_with3(ResObj, left_res, perform_evaluation_process(dn_child(dn, 0), de), {});
            DObjPtr left = dc_unwrap2(left_res);

            dc_try_or_fail_with3(ResObj, index_res, perform_evaluation_process(dn_child(dn, 1), de),
                                 dc_try_fail_temp(DCResVoid, dang_obj_free(left)));
            DObjPtr index = dc_unwrap2(index_res);

            if (left->type != DOBJ_ARRAY || index->type != DOBJ_INTEGER)
            {
                dc_try_fail_temp(DCResVoid, dang_obj_free(left));
                dc_try_fail_temp(DCResVoid, dang_obj_free(index));

                dc_dbg_log("indexing of '%s' is not supporting on '%s'", tostr_DObjType(index), tostr_DObjType(left));
                dc_ret_e(-1, "indexing is not supporting on this type");
            }

            if (dobj_as_int(*index) < 0)
            {
                dc_try_fail_temp(DCResVoid, dang_obj_free(left));
                dc_try_fail_temp(DCResVoid, dang_obj_free(index));

                dc_ret_ok(&dobj_null);
            }

            return eval_index_expression(left, index);
        }

        case DN_CALL_EXPRESSION:
        {
            // function node is the first child
            DNode* fn_node = dn_child(dn, 0);

            // evaluating it must return a function object
            // that is in fact the function literal node saved in the environment before
            // it also holds pointer to its environment
            dc_try_or_fail_with3(ResObj, fn_res, perform_evaluation_process(fn_node, de), {});

            DObj* fn_obj = dc_unwrap2(fn_res);

            if (fn_obj->type != DOBJ_FUNCTION && fn_obj->type != DOBJ_BUILTIN)
            {
                dc_dbg_log("not a function got: '%s'", tostr_DObjType(fn_obj->type));

                dc_ret_ea(-1, "not a function got: '%s'", tostr_DObjType(fn_obj->type));
            }

            // this is a temporary object to hold the evaluated children and the env
            dc_try_or_fail_with3(ResObj, call_obj_res, dang_obj_new(DOBJ_NULL, dc_dv_nullptr(), fn_obj->env, false, true),
                                 dc_try_fail_temp(DCResVoid, dang_obj_free(fn_obj)));

            DObjPtr call_obj = dc_unwrap2(call_obj_res);

            // eval arguments (first element is function symbol the rest is arguments)
            // so we start evaluating children at index 1
            dc_try_fail_temp(DCResVoid, eval_children_nodes(dn, call_obj, 1, de));

            if (fn_obj->type == DOBJ_BUILTIN)
            {
                DBuiltinFunction fn = dc_dv_as(fn_obj->dv, DBuiltinFunction);
                return fn(call_obj);
            }

            return apply_function(call_obj, dobj_get_node(*fn_obj));
        }

        default:
            break;
    };

    dc_ret_ea(-1, "Unimplemented or unsupported node type: %s", tostr_DNType(dn->type));
}

// ***************************************************************************************
// * PUBLIC FUNCTIONS
// ***************************************************************************************

ResObj dang_eval(DNode* dn, DEnv* de)
{
    DC_RES2(ResObj);

    if (!dn) dc_ret_e(dc_e_code(NV), "cannot evaluate NULL node");
    if (!de) dc_ret_e(dc_e_code(NV), "cannot run evaluation on NULL environment");

    dc_try_fail_temp(DCResVoid, dc_da_push(&de->nodes, dc_dv(DNodePtr, dn)));

    return perform_evaluation_process(dn, de);
}

DCResVoid dang_obj_init(DObj* dobj, DObjType dobjt, DCDynVal dv, DEnv* de, bool is_returned, bool has_children)
{
    DC_RES_void();

    dobj->type = dobjt;
    dobj->dv = dv;
    dobj->is_returned = is_returned;

    dobj->env = de;

    dobj->children = (DCDynArr){0};

    if (has_children) dc_try_fail(dc_da_init2(&dobj->children, 10, 3, dobj_child_free));

    dc_ret();
}


ResObj dang_obj_new(DObjType dobjt, DCDynVal dv, DEnv* de, bool is_returned, bool has_children)
{
    DC_RES2(ResObj);

    DObj* dobj = (DObj*)malloc(sizeof(DObj));
    if (!dobj)
    {
        dc_dbg_log("Memory allocation failed");

        dc_ret_e(2, "Memory allocation failed");
    }

    dc_try_fail_temp(DCResVoid, dang_obj_init(dobj, dobjt, dv, de, is_returned, has_children));

    dc_ret_ok(dobj);
}

ResObj dang_obj_copy(DObj* dobj)
{
    DC_RES2(ResObj);

    if (!dobj)
    {
        dc_dbg_log("Cannot initialize dang object from uninitialized objects");

        dc_ret_e(2, "cannot initialize dang object from uninitialized objects");
    }

    bool has_children = dobj->children.count > 0;

    dc_try_fail(dang_obj_new(dobj->type, dobj->dv, dobj->env, dobj->is_returned, has_children));

    if (has_children) dc_da_for(dobj->children, dc_da_push(&dc_unwrap()->children, dc_dv(DObjPtr, dobj_child(dobj, _idx))));

    dc_ret();
}

DCResVoid dang_obj_free(DObj* dobj)
{
    DC_TRY_DEF2(DCResVoid, dc_dv_free(&dobj->dv, NULL));

    if (dobj->children.cap != 0) dc_try(dc_da_free(&dobj->children));

    // free the end only if 1. it is not NULL and 2. it is an enclosed env
    if (dobj->env && dobj->env->outer)
    {
        dc_try(dang_env_free(dobj->env));
        dobj->env = NULL;
    }

    dc_ret();
}

DC_DV_FREE_FN_DECL(dobj_child_free)
{
    DC_RES_void();

    if (dc_dv_is(*_value, DObjPtr) && dc_dv_as(*_value, DObjPtr) != NULL) dc_try(dang_obj_free(dc_dv_as(*_value, DObjPtr)));

    dc_ret();
}

ResEnv dang_env_new()
{
    DC_RES2(ResEnv);

    dc_try_fail_temp(DCResVoid, dang_obj_init(&dobj_null, DOBJ_NULL, dc_dv_nullptr(), NULL, false, false));
    dc_try_fail_temp(DCResVoid, dang_obj_init(&dobj_null_return, DOBJ_NULL, dc_dv_nullptr(), NULL, true, false));

    dc_try_fail_temp(DCResVoid, dang_obj_init(&dobj_true, DOBJ_BOOLEAN, dc_dv_true(), NULL, false, false));
    dc_try_fail_temp(DCResVoid, dang_obj_init(&dobj_false, DOBJ_BOOLEAN, dc_dv_false(), NULL, false, false));

    DEnv* de = (DEnv*)malloc(sizeof(DEnv));
    if (de == NULL)
    {
        dc_dbg_log("Memory allocation failed");

        dc_ret_e(2, "Memory allocation failed");
    }

    dc_try_or_fail_with3(DCResVoid, res, dc_ht_init(&de->store, 17, string_hash, string_key_cmp, env_store_free), {
        dc_dbg_log("cannot initialize dang environment hash table");

        free(de);
    });

    dc_try_or_fail_with2(res, dc_da_init2(&de->nodes, 10, 3, dn_child_free), {
        dc_dbg_log("cannot initialize dang environment hash table");

        free(de);
    });

    de->outer = NULL;

    dc_ret_ok(de);
}

ResEnv dang_env_new_enclosed(DEnv* outer)
{
    DC_TRY_DEF2(ResEnv, dang_env_new());

    dc_unwrap()->outer = outer;

    dc_ret();
}

DCResVoid dang_env_free(DEnv* de)
{
    DC_RES_void();

    dc_try_fail(dc_ht_free(&de->store));

    dc_try_fail(dc_da_free(&de->nodes));

    free(de);

    dc_ret();
}


ResObj dang_env_get(DEnv* de, string name)
{
    DC_RES2(ResObj);

    DCDynVal* found = NULL;

    dc_try_fail_temp(DCResUsize, dc_ht_find_by_key(&de->store, dc_dv(string, name), &found));

    if (found) dc_ret_ok(dc_dv_as(*found, DObjPtr));

    if (de->outer) return dang_env_get(de->outer, name);

    dc_dbg_log("key '%s' not found in the environment", name);

    dc_ret_ea(6, "'%s' is not defined", name);
}

ResObj dang_env_set(DEnv* de, string name, DObj* dobj, bool update_only)
{
    DC_RES2(ResObj);

    DCDynVal value = dobj ? dc_dva(DObjPtr, dobj) : dc_dv(DObjPtr, &dobj_null);

    DCResVoid res =
        dc_ht_set(&de->store, dc_dv(string, name), value, update_only ? DC_HT_SET_UPDATE_OR_FAIL : DC_HT_SET_CREATE_OR_FAIL);

    if (dc_is_err2(res))
    {
        if (dc_err_code2(res) == dc_e_code(HT_SET))
            dc_ret_ea(dc_e_code(HT_SET), "'%s' %s", name, (update_only ? "is not defined." : "is already defined."));

        dc_err_cpy(res);
        dc_ret();
    }

    dc_ret_ok(dobj);
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
            return "(unknown object type)";
    }
}
