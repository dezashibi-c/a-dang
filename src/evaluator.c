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

static DCRes perform_evaluation_process(DNodePtr dn, DEnv* de, DEnv* main_de);

// ***************************************************************************************
// * PRIVATE HELPER FUNCTIONS
// ***************************************************************************************

static u32 string_hash(string key)
{
    u32 hash = 5381;
    i32 c;
    while ((c = *key++))
    {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

static u32 bool_hash(b1 key)
{
    return !!key;
}

static u32 integer_hash(i64 key)
{
    return key < 0 ? (-1 * key) : key;
}

static DC_HT_HASH_FN_DECL(hash_obj_hash_fn)
{
    DC_RES_u32();

    switch (dobj_get_type(_key))
    {
        case DOBJ_STRING:
            dc_ret_ok(string_hash(dc_dv_as(*_key, string)));

        case DOBJ_INTEGER:
            dc_ret_ok(integer_hash(dc_dv_as(*_key, i64)));

        case DOBJ_BOOLEAN:
            dc_ret_ok(bool_hash(dc_dv_as(*_key, b1)));

        default:
            break;
    };

    dc_ret_e(-1, "only integer, boolean and strings can be used as hash table key");
}

static DC_HT_KEY_CMP_FN_DECL(hash_obj_hash_key_cmp_fn)
{
    return dc_dv_eq(_key1, _key2);
}

static DC_HT_HASH_FN_DECL(env_hash_fn)
{
    DC_RES_u32();

    if (_key->type != dc_dvt(string)) dc_ret_e(dc_e_code(TYPE), dc_e_msg(TYPE));

    dc_ret_ok(string_hash(dc_dv_as(*_key, string)));
}

static DC_HT_KEY_CMP_FN_DECL(string_key_cmp)
{
    return dc_dv_eq(_key1, _key2);
}

// ***************************************************************************************
// * PRIVATE FUNCTIONS
// ***************************************************************************************

static ResEnv _env_new(b1 is_main)
{
    DC_RES2(ResEnv);

    DEnv* de = (DEnv*)malloc(sizeof(DEnv));
    if (de == NULL)
    {
        dc_dbg_log("Memory allocation failed");

        dc_ret_e(2, "Memory allocation failed");
    }

    dc_try_or_fail_with3(DCResVoid, res, dc_ht_init(&de->memory, 17, env_hash_fn, string_key_cmp, NULL), {
        dc_dbg_log("cannot initialize dang environment hash table");

        free(de);
    });

    if (is_main)
    {
        dc_try_or_fail_with2(res, dc_da_init2(&de->cleanups, 10, 3, dang_obj_free), {
            dc_dbg_log("cannot initialize dang environment hash table");

            free(de);
        });
    }
    else
    {
        de->cleanups = (DCDynArr){0};
    }

    de->outer = NULL;

    dc_ret_ok(de);
}

static ResEnv _env_new_enclosed(DEnv* outer)
{
    DC_TRY_DEF2(ResEnv, _env_new(false));

    dc_unwrap()->outer = outer;

    dc_ret();
}


static DCRes eval_program_statements(DNodePtr dn, DEnv* de, DEnv* main_de)
{
    DC_RES();

    if (dn_child_count(dn) == 0) dc_ret_ok_dv_nullptr();

    dc_da_for(program_eval_loop, dn->children, {
        DNodePtr stmt = dn_child(dn, _idx);
        dc_try_fail(perform_evaluation_process(stmt, de, main_de));

        // This means the actual syntax of the current node must be 'return'
        // in order to stop the evaluation
        if (stmt->type == DN_RETURN_STATEMENT) DC_BREAK(program_eval_loop);
    });

    dc_ret();
}

static DCRes eval_block_statements(DNodePtr dn, DEnv* de, DEnv* main_de)
{
    DC_RES();

    if (dn_child_count(dn) == 0) dc_ret_ok_dv_nullptr();

    dc_da_for(block_eval_loop, dn->children, {
        DNodePtr stmt = dn_child(dn, _idx);
        dc_try_fail(perform_evaluation_process(stmt, de, main_de));

        if (dc_unwrap().is_returned) DC_BREAK(block_eval_loop);
    });

    dc_ret();
}

static DCRes eval_bang_operator(DCDynValPtr right)
{
    DC_RES();

    dc_try_or_fail_with3(DCResBool, bool_val, dc_dv_to_bool(right), {});

    dc_ret_ok_dv_bool(!dc_unwrap2(bool_val));
}

static DCRes eval_minus_prefix_operator(DCDynValPtr right)
{
    DC_RES();

    if (dc_dv_is_not(*right, i64))
        dc_ret_ea(-1, "'-' operator does not support right value of type '%s'", tostr_DObjType(right));

    dc_ret_ok_dv(i64, -dc_dv_as(*right, i64));
}

static DCRes eval_prefix_expression(DNodePtr dn, DCDynValPtr right)
{
    DC_RES();

    string node_text = dn_data_as(dn, string);

    if (strcmp(node_text, "!") == 0)
        return eval_bang_operator(right);

    else if (strcmp(node_text, "-") == 0)
        return eval_minus_prefix_operator(right);


    dc_ret_ea(-1, "unimplemented infix operator '%s'", node_text);
}

static DCRes eval_integer_infix_expression(DNodePtr dn, DCDynValPtr left, DCDynValPtr right)
{
    DC_RES();

    i64 lval = dc_dv_as(*left, i64);
    i64 rval = dc_dv_as(*right, i64);

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
        dc_ret_ok_dv_bool(lval < rval);

    else if (strcmp(node_text, ">") == 0)
        dc_ret_ok_dv_bool(lval > rval);

    else if (strcmp(node_text, "==") == 0)
        dc_ret_ok_dv_bool(lval == rval);

    else if (strcmp(node_text, "!=") == 0)
        dc_ret_ok_dv_bool(lval != rval);

    else
        dc_ret_ea(-1, "unimplemented infix operator '%s' for '%s' and '%s'", node_text, tostr_DObjType(left),
                  tostr_DObjType(right));

    dc_ret_ok(value);
}

static DCRes eval_boolean_infix_expression(DNodePtr dn, DCDynValPtr left, DCDynValPtr right)
{
    DC_RES();

    dc_try_or_fail_with3(DCResBool, lval_bool, dc_dv_to_bool(left), {});
    dc_try_or_fail_with3(DCResBool, rval_bool, dc_dv_to_bool(right), {});

    b1 lval = dc_unwrap2(lval_bool);
    b1 rval = dc_unwrap2(rval_bool);

    string node_text = dn_data_as(dn, string);

    if (strcmp(node_text, "==") == 0)
        dc_ret_ok_dv_bool(lval == rval);

    else if (strcmp(node_text, "!=") == 0)
        dc_ret_ok_dv_bool(lval != rval);

    dc_ret_ea(-1, "unimplemented infix operator '%s' for '%s' and '%s'", node_text, tostr_DObjType(left),
              tostr_DObjType(right));
}

static DCRes eval_string_infix_expression(DNodePtr dn, DCDynValPtr left, DCDynValPtr right, DEnv* main_de)
{
    DC_RES();

    string lval = dc_dv_as(*left, string);
    string rval = dc_dv_as(*right, string);

    string node_text = dn_data_as(dn, string);

    if (strcmp(node_text, "+") == 0)
    {
        string result;
        dc_sprintf(&result, "%s%s", lval, rval);

        REGISTER_CLEANUP(string, result, free(result));

        dc_ret_ok_dv(string, result);
    }

    else if (strcmp(node_text, "==") == 0)
        dc_ret_ok_dv_bool(strcmp(lval, rval) == 0);

    dc_ret_ea(-1, "unimplemented infix operator '%s' for '%s' and '%s'", node_text, tostr_DObjType(left),
              tostr_DObjType(right));
}

static DCRes eval_infix_expression(DNodePtr dn, DCDynValPtr left, DCDynValPtr right, DEnv* main_de)
{
    DC_RES();

    string node_text = dn_data_as(dn, string);

    if (dc_dv_is(*left, i64) && dc_dv_is(*right, i64))
        return eval_integer_infix_expression(dn, left, right);

    else if (dc_dv_is(*left, b1) || dc_dv_is(*right, b1))
    {
        return eval_boolean_infix_expression(dn, left, right);
    }

    else if (dc_dv_is(*right, string) && dc_dv_is(*left, string))
        return eval_string_infix_expression(dn, left, right, main_de);

    else if (dc_dv_is(*left, string) && strcmp(node_text, "==") != 0)
    {
        DCDynVal right_converted = dc_dva(string, dc_unwrap2(dc_tostr_dv(right)));
        DCRes res = eval_string_infix_expression(dn, left, &right_converted, main_de);

        // free the allocated string (in conversion)
        dc_dv_free(&right_converted, NULL);

        return res;
    }

    else if (dc_dv_is(*right, string) && strcmp(node_text, "==") != 0)
    {
        DCDynVal left_converted = dc_dva(string, dc_unwrap2(dc_tostr_dv(left)));
        DCRes res = eval_string_infix_expression(dn, &left_converted, right, main_de);

        // free the allocated string (in conversion)
        dc_dv_free(&left_converted, NULL);

        return res;
    }

    dc_ret_ea(-1, "unimplemented infix operator '%s' for '%s' and '%s'", node_text, tostr_DObjType(left),
              tostr_DObjType(right));
}

static DCRes eval_array_index_expression(DCDynValPtr left, DCDynValPtr index)
{
    DC_RES();

    usize idx = (usize)(dc_dv_as(*index, i64));
    DCDynArrPtr arr = dc_dv_as(*left, DCDynArrPtr);

    if (idx > arr->count - 1)
    {
        dc_try_fail_temp(DCResVoid, dang_obj_free(left));
        dc_try_fail_temp(DCResVoid, dang_obj_free(index));

        dc_ret_ok_dv_nullptr();
    }

    dc_ret_ok(dc_da_get2(*arr, idx));
}

static DCRes eval_hash_index_expression(DCDynValPtr left, DCDynValPtr index)
{
    DC_RES();

    DCDynValPtr found = NULL;
    DCHashTablePtr _ht = dc_dv_as(*left, DCHashTablePtr);

    dc_try_fail_temp(DCResUsize, dc_ht_find_by_key(_ht, *index, &found));

    if (!found) dc_ret_ok_dv_nullptr();

    dc_ret_ok(*found);
}

static DCRes eval_if_expression(DNodePtr dn, DEnv* de, DEnv* main_de)
{
    DC_RES();

    DNodePtr condition = dn_child(dn, 0);
    DNodePtr consequence = dn_child(dn, 1);
    DNodePtr alternative = (dn_child_count(dn) > 2) ? dn_child(dn, 2) : NULL;

    dc_try_or_fail_with3(DCRes, condition_evaluated, perform_evaluation_process(condition, de, main_de), {});

    dc_try_or_fail_with3(DCResBool, condition_as_bool, dc_dv_to_bool(&dc_unwrap2(condition_evaluated)), {});

    if (dc_unwrap2(condition_as_bool))
        return perform_evaluation_process(consequence, de, main_de);

    else if (alternative)
        return perform_evaluation_process(alternative, de, main_de);

    dc_ret_ok_dv_nullptr();
}

static DCRes eval_let_statement(DNodePtr dn, DEnv* de, DEnv* main_de)
{
    DC_RES();

    string var_name = dn_child_data_as(dn, 0, string);

    DCDynValPtr value = NULL;
    if (dn_child_count(dn) > 1)
    {
        DNodePtr value_node = dn_child(dn, 1);

        dc_try_or_fail_with3(DCRes, v_res, perform_evaluation_process(value_node, de, main_de), {});

        value = &dc_unwrap2(v_res);

        // It doesn't matter if it was a returned value from some other function, etc.
        // It must be reset as it is now have been saved in the environment.
        value->is_returned = false;
    }

    dc_try_fail_temp(DCRes, dang_env_set(de, var_name, value, false));

    dc_ret_ok_dv_nullptr();
}

static DCRes eval_hash_literal(DNodePtr dn, DEnv* de, DEnv* main_de)
{
    DC_RES();

    if (dn_child_count(dn) % 2 != 0) dc_ret_e(-1, "wrong hash literal node");

    dc_try_or_fail_with3(DCResHt, ht_res, dc_ht_new(17, hash_obj_hash_fn, hash_obj_hash_key_cmp_fn, NULL), {});

    DCHashTablePtr ht = dc_unwrap2(ht_res);

    REGISTER_CLEANUP(DCHashTablePtr, ht, dc_ht_free(ht));

    dc_da_for(hash_lit_eval_loop, dn->children, {
        if (_idx % 2 != 0) continue; // odd numbers are values

        DNodePtr key_node = dn_child(dn, _idx);

        // this child is the key
        dc_try_or_fail_with3(DCRes, key_obj, perform_evaluation_process(key_node, de, main_de), {});

        // next child is the value
        DNodePtr value_node = dn_child(dn, _idx + 1);
        dc_try_or_fail_with3(DCRes, value_obj, perform_evaluation_process(value_node, de, main_de), {});

        dc_try_or_fail_with3(DCResVoid, res,
                             dc_ht_set(ht, dc_unwrap2(key_obj), dc_unwrap2(value_obj), DC_HT_SET_CREATE_OR_UPDATE), {});
    });

    dc_ret_ok_dv(DCHashTablePtr, ht);
}

static DCResVoid eval_children_nodes(DNodePtr call_node, DCDynArrPtr parent_result, usize child_start_index, DEnv* de,
                                     DEnv* main_de)
{
    DC_RES_void();

    // first child is the function identifier or expression
    // the rest is function argument
    for (usize i = child_start_index; i < dn_child_count(call_node); ++i)
    {
        dc_try_or_fail_with3(DCRes, arg_res, perform_evaluation_process(dn_child(call_node, i), de, main_de), {});

        dc_try_or_fail_with3(DCResVoid, res, dc_da_push(parent_result, dc_unwrap2(arg_res)),
                             dc_dbg_log("failed to push object to parent object"));
    }

    dc_ret();
}

static ResEnv extend_function_env(DCDynValPtr call_obj, DNodePtr fn_node)
{
    DC_TRY_DEF2(ResEnv, _env_new_enclosed(call_obj->env));

    DCDynArrPtr arr = dc_dv_as(*call_obj, DCDynArrPtr);

    if (arr->count != dn_child_count(fn_node) - 1)
        dc_ret_ea(-1, "function needs at least " dc_fmt(usize) " arguments, got=" dc_fmt(usize), dn_child_count(fn_node) - 1,
                  arr->count);

    // extending the environment by defining arguments
    // with given evaluated objects assigning to them
    dc_da_for(extend_env_loop, fn_node->children, {
        // we shouldn't get the last child as it is the function body
        if (_idx >= (dn_child_count(fn_node) - 1)) DC_BREAK(extend_env_loop);

        string arg_name = dn_child_data_as(fn_node, _idx, string);

        dc_try_fail_temp(DCRes, dang_env_set(dc_unwrap(), arg_name, &dc_da_get2(*arr, _idx), false));
    });

    dc_ret();
}

/**
 * fn_obj's node is a pointer to the "fn" declaration node that has been saved in the env
 * fn_obj's node's children are arguments except the last one that is the body
 * call_obj holds all the evaluated object arguments in its children field
 * call_obj holds current env as well
 */
static DCRes apply_function(DCDynValPtr call_obj, DNodePtr fn_node, DEnv* main_de)
{
    DC_RES();

    dc_try_or_fail_with3(ResEnv, fn_env_res, extend_function_env(call_obj, fn_node),
                         dc_try_fail_temp(DCResVoid, dang_obj_free(call_obj)));

    DEnv* fn_env = dc_unwrap2(fn_env_res);

    REGISTER_CLEANUP2(dobj_defa(voidptr, NULL, fn_env), dang_env_free(fn_env));

    DNodePtr body = dn_child(fn_node, dn_child_count(fn_node) - 1);

    dc_try(perform_evaluation_process(body, fn_env, main_de));

    // if we've returned of a function that's ok
    // but we don't need to pass it on to the upper level
    if (dc_is_ok()) dc_unwrap().is_returned = false;

    dc_ret();
}

static b1 is_unquote_call(DNodePtr dn)
{
    if (dn->type != DN_CALL_EXPRESSION) return false;

    DNodePtr fn = dn_child(dn, 0);

    return (fn->type == DN_IDENTIFIER) && strcmp(dn_data_as(fn, string), MACRO_UNPACK) == 0;
}

ResNode convert_dobj_to_nodeptr(DCDynValPtr dobj)
{
    DC_RES2(ResNode);

    // DNodePtr result = NULL;

    switch (dobj_get_type(dobj))
    {
        case DOBJ_INTEGER:
            return dn_new(DN_INTEGER_LITERAL, *dobj, false);

        case DOBJ_BOOLEAN:
            return dn_new(DN_BOOLEAN_LITERAL, *dobj, false);

        case DOBJ_QUOTE:
        {
            DNodePtr node = dc_dv_as(*dobj, DNodePtr);
            node->quoted = false; // todo:: check this afterward
            dc_ret_ok(node);
        }

        default:
            break;
    };

    dc_ret_ok(NULL);
}

static DECL_DNODE_MODIFIER_FN(default_modifier)
{
    DC_RES2(ResNode);

    if (!is_unquote_call(dn)) dc_ret_ok(NULL);

    /* If the node is unquote call it must be evaluated and replaced by a node representing the result */

    // hold the first arg
    DNodePtr arg = dn_child(dn, 1);

    // eval the first arg
    dc_try_or_fail_with3(DCRes, unquoted_res, perform_evaluation_process(arg, de, main_de), { dn_full_free(arg); });

    // convert the dobj to node
    dc_try_or_fail_with3(ResNode, new_node_res, convert_dobj_to_nodeptr(&dc_unwrap2(unquoted_res)), {});
    DNodePtr new_node = dc_unwrap2(new_node_res);

    dc_ret_ok(new_node);
}

static ResNode eval_unquote_calls(DNodePtr dn, DEnv* de, DEnv* main_de)
{
    return dn_modify(dn, de, main_de, default_modifier);
}

static DCRes process_quoted(DNodePtr dn, DEnv* de, DEnv* main_de)
{
    DC_RES();

    dc_try_or_fail_with3(ResNode, unquote_res, eval_unquote_calls(dn, de, main_de), {});

    // DNodePtr processed_node = dc_unwrap2(unquote_res) ? dc_unwrap2(unquote_res) : dn;

    // if (dc_unwrap2(unquote_res))
    // {
    //     // add the newly created node to to the cleanups
    //     // REGISTER_CLEANUP(DNodePtr, processed_node, dn_full_free(processed_node));
    // }

    dc_ret_ok(dobj_def(DNodePtr, dc_unwrap2(unquote_res), NULL));
}

// ***************************************************************************************
// * BUILTIN FUNCTIONS
// ***************************************************************************************

static DECL_DBUILTIN_FUNCTION(len)
{
    BUILTIN_FN_GET_ARGS_VALIDATE("len", 1);

    DCDynVal arg = dc_da_get2(_args, 0);

    DObjType arg_type = dobj_get_type(&arg);

    if (arg_type != DOBJ_STRING && arg_type != DOBJ_ARRAY)
    {
        dc_error_inita(*error, -1, "cannot calculate length of arg of type '%s'", tostr_DObjType(&arg));
        return dc_dv_nullptr();
    }

    i64 len = 0;

    if (arg_type == DOBJ_STRING)
    {
        if (dc_dv_as(arg, string)) len = (i64)strlen(dc_dv_as(arg, string));
    }
    else if (arg_type == DOBJ_ARRAY)
    {
        len = (i64)dc_dv_as(arg, DCDynArrPtr)->count;
    }

    return dobj_int(len);
}

static DECL_DBUILTIN_FUNCTION(first)
{
    BUILTIN_FN_GET_ARGS_VALIDATE("first", 1);

    BUILTIN_FN_GET_ARG_NO(0, DOBJ_ARRAY, "first argument must be an array");

    DCDynArr arr = *dc_dv_as(arg0, DCDynArrPtr);

    if (arr.count == 0) return dc_dv_nullptr();

    return dc_da_get2(arr, 0);
}

static DECL_DBUILTIN_FUNCTION(last)
{
    BUILTIN_FN_GET_ARGS_VALIDATE("last", 1);

    BUILTIN_FN_GET_ARG_NO(0, DOBJ_ARRAY, "first argument must be an array");

    DCDynArr arr = *dc_dv_as(arg0, DCDynArrPtr);

    if (arr.count == 0) return dc_dv_nullptr();

    return dc_da_get2(arr, arr.count - 1);
}

static DECL_DBUILTIN_FUNCTION(rest)
{
    BUILTIN_FN_GET_ARGS_VALIDATE("rest", 1);

    BUILTIN_FN_GET_ARG_NO(0, DOBJ_ARRAY, "first argument must be an array");

    DCDynArr arr = *dc_dv_as(arg0, DCDynArrPtr);

    if (arr.count == 0) return dc_dv_nullptr();

    DCResDa res = dc_da_new2(10, 3, dang_obj_free);
    if (dc_is_err2(res))
    {
        *error = dc_err2(res);
        return dc_dv_nullptr();
    }

    DCDynArrPtr result_arr = dc_unwrap2(res);

    dc_da_for(rest_fn_loop, arr, {
        // not the first child but the rest
        if (_idx == 0) continue;

        dc_da_push(result_arr, *_it);
    });

    return dc_dva(DCDynArrPtr, result_arr);
}

static DECL_DBUILTIN_FUNCTION(push)
{
    BUILTIN_FN_GET_ARGS_VALIDATE("push", 2);

    BUILTIN_FN_GET_ARG_NO(0, DOBJ_ARRAY, "first argument must be an array");

    dc_da_push(dc_dv_as(arg0, DCDynArrPtr), dc_da_get2(_args, 1));

    return dc_dv_nullptr();
}

static DECL_DBUILTIN_FUNCTION(print)
{
    (void)error;

    BUILTIN_FN_GET_ARGS;

    dc_da_for(print_loop, _args, {
        dobj_print(_it);
        printf("%s", " ");
    });

    puts("");

    return dc_dv_nullptr();
}

static DCRes find_builtin(string name)
{
    DC_RES();

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

    else if (strcmp(name, "print") == 0)
        fn = print;

    else
    {
        dc_dbg_log("key '%s' not found in the environment", name);

        dc_ret_ea(6, "'%s' is not defined", name);
    }

    dc_ret_ok_dv(DBuiltinFunction, fn);
}

// ***************************************************************************************
// * MAIN EVALUATION PROCESS
// ***************************************************************************************

static DCRes perform_evaluation_process(DNodePtr dn, DEnv* de, DEnv* main_de)
{
    DC_RES();

    if (!dn)
        dc_ret_e(-1, "got NULL node");

    else if (!dn_type_is_valid(dn->type))
        dc_ret_ea(-1, "got invalid node type: %s", tostr_DNType(dn->type));

    dc_dbg_log("evaluating node of type: '%s'", tostr_DNType(dn->type));

    switch (dn->type)
    {
        case DN_PROGRAM:
            return eval_program_statements(dn, de, main_de);

        case DN_EXPRESSION_STATEMENT:
            return perform_evaluation_process(dn_child(dn, 0), de, main_de);

        case DN_PREFIX_EXPRESSION:
        {
            dc_try_or_fail_with3(DCRes, right, perform_evaluation_process(dn_child(dn, 0), de, main_de), {});

            return eval_prefix_expression(dn, &dc_unwrap2(right));
        }

        case DN_INFIX_EXPRESSION:
        {
            dc_try_or_fail_with3(DCRes, left, perform_evaluation_process(dn_child(dn, 0), de, main_de), {});
            dc_try_or_fail_with3(DCRes, right, perform_evaluation_process(dn_child(dn, 1), de, main_de),
                                 dc_try_fail_temp(DCResVoid, dang_obj_free(&dc_unwrap2(left))));

            return eval_infix_expression(dn, &dc_unwrap2(left), &dc_unwrap2(right), main_de);
        }

        case DN_BOOLEAN_LITERAL:
        case DN_INTEGER_LITERAL:
        case DN_STRING_LITERAL:
        {
            DCDynVal result = dn_data(dn);
            result.allocated = false;
            dc_ret_ok(result);
        }

        case DN_IDENTIFIER:
        {
            string name = dn_data_as(dn, string);

            DCRes symbol = dang_env_get(de, name);

            if (dc_is_ok2(symbol)) return symbol;

            return find_builtin(name);
        }

        case DN_BLOCK_STATEMENT:
            return eval_block_statements(dn, de, main_de);

        case DN_IF_EXPRESSION:
            return eval_if_expression(dn, de, main_de);

        case DN_RETURN_STATEMENT:
        {
            DNodePtr ret_val = (dn_child_count(dn) > 0) ? dn_child(dn, 0) : NULL;

            if (!ret_val) dc_ret_ok_dv_nullptr();

            dc_try_or_fail_with3(DCRes, value, perform_evaluation_process(ret_val, de, main_de), {});

            dobj_mark_returned(dc_unwrap2(value));

            return value;
        }

        case DN_LET_STATEMENT:
            return eval_let_statement(dn, de, main_de);

        case DN_FUNCTION_LITERAL:
        {
            // function literal holds actual node in object's dv
            // also holds the pointer to the environment it's being evaluated
            DCDynVal dobj_fn = dobj_def(DNodePtr, dn, de);
            dc_ret_ok(dobj_fn);
        }

        case DN_ARRAY_LITERAL:
        {
            dc_try_or_fail_with3(DCResDa, arr_res, dc_da_new2(10, 3, dang_obj_free), {});

            DCDynArrPtr arr = dc_unwrap2(arr_res);

            REGISTER_CLEANUP(DCDynArrPtr, arr, dc_da_free(arr));

            dc_try_or_fail_with3(DCResVoid, temp_res, eval_children_nodes(dn, arr, 0, de, main_de), {});

            dc_ret_ok_dv(DCDynArrPtr, arr);
        }

        case DN_HASH_LITERAL:
            return eval_hash_literal(dn, de, main_de);

        case DN_INDEX_EXPRESSION:
        {
            dc_try_or_fail_with3(DCRes, left_res, perform_evaluation_process(dn_child(dn, 0), de, main_de), {});
            DCDynVal left = dc_unwrap2(left_res);

            dc_try_or_fail_with3(DCRes, index_res, perform_evaluation_process(dn_child(dn, 1), de, main_de),
                                 dc_try_fail_temp(DCResVoid, dang_obj_free(&left)));
            DCDynVal index = dc_unwrap2(index_res);

            DObjType left_type = dobj_get_type(&left);
            DObjType index_type = dobj_get_type(&index);

            if (left_type != DOBJ_ARRAY && left_type != DOBJ_HASH_TABLE)
            {
                dc_try_fail_temp(DCResVoid, dang_obj_free(&left));
                dc_try_fail_temp(DCResVoid, dang_obj_free(&index));

                dc_dbg_log("indexing of '%s' is not supporting on '%s'", tostr_DObjType(index), tostr_DObjType(left));
                dc_ret_e(-1, "indexing is not supporting on this type");
            }

            else if (left_type == DOBJ_ARRAY && index_type != DOBJ_INTEGER)
            {
                dc_try_fail_temp(DCResVoid, dang_obj_free(&left));
                dc_try_fail_temp(DCResVoid, dang_obj_free(&index));

                dc_dbg_log("indexing of '%s' is not supporting on '%s'", tostr_DObjType(index), tostr_DObjType(left));
                dc_ret_e(-1, "indexing is not supporting on this type");
            }

            if (dobj_as_int(index) < 0)
            {
                dc_try_fail_temp(DCResVoid, dang_obj_free(&left));
                dc_try_fail_temp(DCResVoid, dang_obj_free(&index));

                dc_ret_ok_dv_nullptr();
            }

            if (left_type == DOBJ_ARRAY) return eval_array_index_expression(&left, &index);

            return eval_hash_index_expression(&left, &index);
        }

        case DN_CALL_EXPRESSION:
        {
            // function node is the first child
            DNodePtr fn_node = dn_child(dn, 0);

            if (dn->quoted)
            {
                return process_quoted(dn, de, main_de);
            }

            // evaluating it must return a function object
            // that is in fact the function literal node saved in the environment before
            // it also holds pointer to its environment
            dc_try_or_fail_with3(DCRes, fn_res, perform_evaluation_process(fn_node, de, main_de), {});

            DCDynValPtr fn_obj = &dc_unwrap2(fn_res);

            DObjType fn_obj_type = dobj_get_type(fn_obj);

            if (fn_obj_type != DOBJ_FUNCTION && fn_obj_type != DOBJ_BUILTIN_FUNCTION)
            {
                dc_dbg_log("not a function got: '%s'", tostr_DObjType(fn_obj));

                dc_ret_ea(-1, "not a function got: '%s'", tostr_DObjType(fn_obj));
            }

            // this is a temporary object to hold the evaluated children and the env
            dc_try_or_fail_with3(DCResDa, call_obj_res, dc_da_new2(10, 3, dang_obj_free), {});

            DCDynVal call_obj = dc_dva(DCDynArrPtr, dc_unwrap2(call_obj_res));
            call_obj.env = fn_obj->env;

            // eval arguments (first element is function symbol the rest is arguments)
            // so we start evaluating children at index 1
            dc_try_fail_temp(DCResVoid, eval_children_nodes(dn, dc_unwrap2(call_obj_res), 1, de, main_de));

            if (fn_obj_type == DOBJ_BUILTIN_FUNCTION)
            {
                DBuiltinFunction fn = dc_dv_as(*fn_obj, DBuiltinFunction);

                DCError error = (DCError){0};

                DCDynVal result = fn(&call_obj, &error);

                if (error.code != 0)
                {
                    dc_status() = DC_RES_ERR;
                    dc_err() = error;
                    dc_ret();
                }

                if (dc_dv_is_allocated(result)) REGISTER_CLEANUP2(result, dang_obj_free(&result));

                result.allocated = false;

                dc_ret_ok(result);
            }

            return apply_function(&call_obj, dc_dv_as(*fn_obj, DNodePtr), main_de);
        }

        default:
            break;
    };

    dc_ret_ea(-1, "Unimplemented or unsupported node type: %s", tostr_DNType(dn->type));
}

// ***************************************************************************************
// * PUBLIC FUNCTIONS
// ***************************************************************************************

DCRes dang_eval(DNodePtr program, DEnv* main_de)
{
    DC_RES();

    if (!program) dc_ret_e(dc_e_code(NV), "cannot evaluate NULL node");
    if (!main_de) dc_ret_e(dc_e_code(NV), "cannot run evaluation on NULL environment");

    REGISTER_CLEANUP(DNodePtr, program, dn_full_free(program));

    return perform_evaluation_process(program, main_de, main_de);
}

DCResVoid dang_obj_free(DCDynValPtr dobj)
{
    DC_RES_void();

    // only those envs which are not the main environment
    // in other words enclosed environments only
    if (dobj->env && dobj->env->outer)
    {
        dc_dbg_log("dobj's environment is being cleaned");

        dc_try_fail(dang_env_free(dobj->env));
        dobj->env = NULL;
    }

    if (dobj->type == dc_dvt(DNodePtr))
    {
        dc_dbg_log("node pointer is being cleaned");

        dc_try_fail(dn_full_free(dc_dv_as(*dobj, DNodePtr)));

        dc_dv_set(*dobj, DNodePtr, NULL);

        dc_ret();
    }

    dc_dbg_log("dobj of type %s is being cleaned", tostr_DObjType(dobj));

    return dc_dv_free(dobj, NULL);
}

DObjType dobj_get_type(DCDynValPtr dobj)
{
    switch (dobj->type)
    {
        case dc_dvt(i64):
            return DOBJ_INTEGER;

        case dc_dvt(b1):
            return DOBJ_BOOLEAN;

        case dc_dvt(string):
            return DOBJ_STRING;

        case dc_dvt(DNodePtr):
        {
            DNodePtr dn = dc_dv_as(*dobj, DNodePtr);

            if (dn->quoted) return DOBJ_QUOTE;

            switch (dn->type)
            {
                case DN_FUNCTION_LITERAL:
                    return DOBJ_FUNCTION;

                case DN_MACRO_LITERAL:
                    return DOBJ_MACRO;

                default:
                    break;
            }

            break;
        }

        case dc_dvt(DBuiltinFunction):
            return DOBJ_BUILTIN_FUNCTION;

        case dc_dvt(DCDynArrPtr):
            return DOBJ_ARRAY;

        case dc_dvt(DCHashTablePtr):
            return DOBJ_HASH_TABLE;

        case dc_dvt(voidptr):
            if (dc_dv_is_null(*dobj)) return DOBJ_NULL;

            break;

        case dc_dvt(DCDynValPtr):
            return dobj_get_type(dc_dv_as(*dobj, DCDynValPtr));

        default:
            break;
    }

    return DOBJ_UNKNOWN;
}

DCResString dobj_tostr(DCDynValPtr dobj)
{
    DC_RES_string();

    string result = NULL;

    switch (dobj_get_type(dobj))
    {
        case DOBJ_HASH_TABLE:
        case DOBJ_ARRAY:
        case DOBJ_BOOLEAN:
        case DOBJ_INTEGER:
        case DOBJ_STRING:
            return dc_tostr_dv(dobj);
            break;

        case DOBJ_QUOTE:
            dang_node_inspect(dc_dv_as(*dobj, DNodePtr), &result);
            break;

        case DOBJ_MACRO:
            dc_sprintf(&result, "%s", "(macro)");
            break;

        case DOBJ_FUNCTION:
            dc_sprintf(&result, "%s", "(function)");
            break;

        case DOBJ_BUILTIN_FUNCTION:
            dc_sprintf(&result, "%s", "(builtin function)");
            break;

        case DOBJ_NULL:
            dc_sprintf(&result, "%s", "(null)");
            break;

        default:
            dc_sprintf(&result, "%s", "(unknown object)");
            break;
    }

    dc_ret_ok(result);
}

void dobj_print(DCDynValPtr dobj)
{
    DCResString res = dobj_tostr(dobj);

    if (dc_is_err2(res))
    {
        printf("%s", "(conversion to string error)");
        return;
    }

    printf("%s", dc_unwrap2(res));

    free(dc_unwrap2(res));
}

ResEnv dang_env_new()
{
    return _env_new(true);
}

DCResVoid dang_env_free(DEnv* de)
{
    DC_RES_void();

    dc_dbg_log("number of memory elements: " dc_fmt(usize) ", number of cleanup elements: " dc_fmt(usize), de->memory.key_count,
               de->cleanups.count);

    dc_try_fail(dc_ht_free(&de->memory));

    if (de->cleanups.cap != 0) dc_try_fail(dc_da_free(&de->cleanups));

    free(de);

    dc_ret();
}


DCRes dang_env_get(DEnv* de, string name)
{
    DC_RES();

    DCDynValPtr found = NULL;

    dc_try_fail_temp(DCResUsize, dc_ht_find_by_key(&de->memory, dc_dv(string, name), &found));

    if (found) dc_ret_ok(*found);

    if (de->outer) return dang_env_get(de->outer, name);

    dc_dbg_log("key '%s' not found in the environment", name);

    dc_ret_ea(6, "'%s' is not defined", name);
}

DCRes dang_env_set(DEnv* de, string name, DCDynValPtr value, b1 update_only)
{
    DC_RES();

    DCDynVal val_to_save = value ? *value : dc_dv_nullptr();

    DCResVoid res = dc_ht_set(&de->memory, dc_dv(string, name), val_to_save,
                              update_only ? DC_HT_SET_UPDATE_OR_FAIL : DC_HT_SET_CREATE_OR_FAIL);

    if (dc_is_err2(res))
    {
        if (dc_err_code2(res) == dc_e_code(HT_SET))
            dc_ret_ea(dc_e_code(HT_SET), "'%s' %s", name, (update_only ? "is not defined." : "is already defined."));

        dc_err_cpy(res);
        dc_ret();
    }

    dc_ret_ok(val_to_save);
}

string tostr_DObjType(DCDynValPtr dobj)
{
    switch (dobj_get_type(dobj))
    {
        case DOBJ_INTEGER:
            return "integer";

        case DOBJ_STRING:
            return "string";

        case DOBJ_BOOLEAN:
            return "boolean";

        case DOBJ_MACRO:
            return "macro";

        case DOBJ_QUOTE:
            return "(QUOTE)";

        case DOBJ_FUNCTION:
            return "function";

        case DOBJ_BUILTIN_FUNCTION:
            return "builtin function";

        case DOBJ_ARRAY:
            return "array";

        case DOBJ_HASH_TABLE:
            return "hash table";

        case DOBJ_NULL:
            return "(null)";

        default:
            break;
    }

    return "(unknown object type)";
}

ResNode dn_modify(DNodePtr dn, DEnv* de, DEnv* main_de, DNodeModifierFn modifier)
{
    DC_RES2(ResNode);

    if (!modifier) dc_ret_e(dc_e_code(NV), "modifier function cannot be null");

    dc_da_for(statement_modification_loop, dn->children, {
        // get pointer to the first child
        DNodePtr child = dn_child(dn, _idx);

        // modify the child
        dc_try_or_fail_with3(ResNode, modified_res, dn_modify(child, de, main_de, modifier), {});

        if (!dc_unwrap2(modified_res)) continue;

        // register the node
        // REGISTER_CLEANUP(DNodePtr, child, dn_full_free(child));

        // replace the node
        dn_child(dn, _idx) = dc_unwrap2(modified_res);
    });

    dc_try(modifier(dn, de, main_de));

    dc_ret_ok(dc_unwrap() ? dc_unwrap() : dn);
}
