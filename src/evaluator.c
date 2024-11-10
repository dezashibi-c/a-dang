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
// * MACROS
// ***************************************************************************************

#define pool_last_el(DE) &dc_da_get2((DE)->pool, (DE)->pool.count - 1)

// ***************************************************************************************
// * FORWARD DECLARATIONS
// ***************************************************************************************

static DCRes perform_evaluation_process(DEvaluator* de, DCDynValPtr dn, DEnv* env);

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

    switch (_key->type)
    {
        case DO_STRING:
            dc_ret_ok(string_hash(do_as_string(*_key)));

        case DO_INTEGER:
            dc_ret_ok(integer_hash(do_as_int(*_key)));

        case DO_BOOLEAN:
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

    if (_key->type != DO_STRING) dc_ret_e(dc_e_code(TYPE), dc_e_msg(TYPE));

    dc_ret_ok(string_hash(do_as_string(*_key)));
}

static DC_HT_KEY_CMP_FN_DECL(string_key_cmp)
{
    return dc_dv_eq(_key1, _key2);
}

// ***************************************************************************************
// * PRIVATE FUNCTIONS
// ***************************************************************************************

static ResEnv _env_new()
{
    DC_RES2(ResEnv);

    DEnv* env = (DEnv*)malloc(sizeof(DEnv));
    if (env == NULL)
    {
        dc_dbg_log("Memory allocation failed");

        dc_ret_e(2, "Memory allocation failed");
    }

    dc_try_or_fail_with3(DCResVoid, res, dang_env_init(env), free(env));

    dc_ret_ok(env);
}

static ResEnv _env_new_enclosed(DEvaluator* de, DEnv* outer)
{
    DC_TRY_DEF2(ResEnv, _env_new());

    dc_unwrap()->outer = outer;

    dc_try_or_fail_with3(DCResVoid, res, dc_da_push(&de->pool, dc_dva(DEnvPtr, dc_unwrap())), {
        dc_try_fail_temp(DCResVoid, dang_env_free(dc_unwrap()));
        free(dc_unwrap());
    });

    dc_ret();
}

static DCRes eval_program_statements(DEvaluator* de, DCDynArrPtr statements, DEnv* env)
{
    DC_RES();

    if (!statements || statements->count == 0) dc_ret_ok_dv_nullptr();

    dc_da_for(program_eval_loop, *statements, {
        dc_try_fail(perform_evaluation_process(de, _it, env));

        if_dv_is_DoReturn_return_unwrapped();
    });

    dc_ret();
}

static DCRes eval_block_statements(DEvaluator* de, DCDynArrPtr statements, DEnv* env)
{
    DC_RES();

    if (!statements || statements->count == 0) dc_ret_ok_dv_nullptr();

    dc_da_for(block_eval_loop, *statements, {
        dc_try_fail(perform_evaluation_process(de, _it, env));

        if (dc_unwrap().type == DO_RETURN) DC_BREAK(block_eval_loop);
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

    if (!do_is_int(*right)) dc_ret_ea(-1, "'-' operator does not support right value of type '%s'", dv_type_tostr(right));

    dc_ret_ok(do_int(-do_as_int(*right)));
}

static DCRes eval_prefix_expression(string op, DCDynValPtr operand)
{
    DC_RES();

    if (strcmp(op, "!") == 0)
        return eval_bang_operator(operand);

    else if (strcmp(op, "-") == 0)
        return eval_minus_prefix_operator(operand);


    dc_ret_ea(-1, "unimplemented infix operator '%s'", op);
}

static DCRes eval_integer_infix_expression(string op, DCDynValPtr left, DCDynValPtr right)
{
    DC_RES();

    i64 lval = do_as_int(*left);
    i64 rval = do_as_int(*right);

    DCDynVal value = {0};

    if (strcmp(op, "+") == 0)
        value = dc_dv(i64, (lval + rval));

    else if (strcmp(op, "-") == 0)
        value = dc_dv(i64, (lval - rval));

    else if (strcmp(op, "*") == 0)
        value = dc_dv(i64, (lval * rval));

    else if (strcmp(op, "/") == 0)
        value = dc_dv(i64, (lval / rval));

    else if (strcmp(op, "<") == 0)
        dc_ret_ok_dv_bool(lval < rval);

    else if (strcmp(op, ">") == 0)
        dc_ret_ok_dv_bool(lval > rval);

    else if (strcmp(op, "==") == 0)
        dc_ret_ok_dv_bool(lval == rval);

    else if (strcmp(op, "!=") == 0)
        dc_ret_ok_dv_bool(lval != rval);

    else
        dc_ret_ea(-1, "unimplemented infix operator '%s' for '%s' and '%s'", op, dv_type_tostr(left), dv_type_tostr(right));

    dc_ret_ok(value);
}

static DCRes eval_boolean_infix_expression(string op, DCDynValPtr left, DCDynValPtr right)
{
    DC_RES();

    dc_try_or_fail_with3(DCResBool, lval_bool, dc_dv_to_bool(left), {});
    dc_try_or_fail_with3(DCResBool, rval_bool, dc_dv_to_bool(right), {});

    b1 lval = dc_unwrap2(lval_bool);
    b1 rval = dc_unwrap2(rval_bool);

    if (strcmp(op, "==") == 0)
        dc_ret_ok_dv_bool(lval == rval);

    else if (strcmp(op, "!=") == 0)
        dc_ret_ok_dv_bool(lval != rval);

    dc_ret_ea(-1, "unimplemented infix operator '%s' for '%s' and '%s'", op, dv_type_tostr(left), dv_type_tostr(right));
}

static DCRes eval_string_infix_expression(DEvaluator* de, string op, DCDynValPtr left, DCDynValPtr right)
{
    DC_RES();

    string lval = do_as_string(*left);
    string rval = do_as_string(*right);

    if (strcmp(op, "+") == 0)
    {
        string result;
        dc_sprintf(&result, "%s%s", lval, rval);

        if (result)
        {
            dc_try_or_fail_with3(DCResVoid, res, dc_da_push(&de->pool, dc_dva(string, result)), {
                dc_dbg_log("failed to push result array to the pool");
                free(result);
            });
        }

        dc_ret_ok_dv(string, result);
    }

    else if (strcmp(op, "==") == 0)
        dc_ret_ok_dv_bool(strcmp(lval, rval) == 0);

    dc_ret_ea(-1, "unimplemented infix operator '%s' for '%s' and '%s'", op, dv_type_tostr(left), dv_type_tostr(right));
}

static DCRes eval_infix_expression(DEvaluator* de, string op, DCDynValPtr left, DCDynValPtr right)
{
    DC_RES();

    if (do_is_int(*left) && do_is_int(*right))
        return eval_integer_infix_expression(op, left, right);

    else if (dc_dv_is(*left, b1) || dc_dv_is(*right, b1))
    {
        return eval_boolean_infix_expression(op, left, right);
    }

    else if (do_is_string(*right) && do_is_string(*left))
        return eval_string_infix_expression(de, op, left, right);

    else if (do_is_string(*left) && strcmp(op, "==") != 0)
    {
        DCDynVal right_converted = dc_dva(string, dc_unwrap2(dc_tostr_dv(right)));
        DCRes res = eval_string_infix_expression(de, op, left, &right_converted);

        // free the allocated string (in conversion)
        dc_dv_free(&right_converted, NULL);

        return res;
    }

    else if (do_is_string(*right) && strcmp(op, "==") != 0)
    {
        DCDynVal left_converted = dc_dva(string, dc_unwrap2(dc_tostr_dv(left)));
        DCRes res = eval_string_infix_expression(de, op, &left_converted, right);

        // free the allocated string (in conversion)
        dc_dv_free(&left_converted, NULL);

        return res;
    }

    dc_ret_ea(-1, "unimplemented infix operator '%s' for '%s' and '%s'", op, dv_type_tostr(left), dv_type_tostr(right));
}

static DCRes eval_array_index_expression(DCDynValPtr left, DCDynValPtr index)
{
    DC_RES();

    usize idx = (usize)(do_as_int(*index));
    DCDynArrPtr arr = dc_dv_as(*left, DCDynArrPtr);

    if (idx > arr->count - 1) dc_ret_ok_dv_nullptr();

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

static DCRes eval_if_expression(DEvaluator* de, DNodeIfExpression* if_node, DEnv* env)
{
    DC_RES();

    dc_try_or_fail_with3(DCRes, condition_evaluated, perform_evaluation_process(de, if_node->condition, env), {});

    dc_try_or_fail_with3(DCResBool, condition_as_bool, dc_dv_to_bool(&dc_unwrap2(condition_evaluated)), {});

    if (dc_unwrap2(condition_as_bool))
        return perform_evaluation_process(de, &dc_dv(DNodeBlockStatement, dn_block(if_node->consequence)), env);

    else if (if_node->alternative)
        return perform_evaluation_process(de, &dc_dv(DNodeBlockStatement, dn_block(if_node->alternative)), env);

    dc_ret_ok_dv_nullptr();
}

static DCRes eval_let_statement(DEvaluator* de, DNodeLetStatement* let_node, DEnv* env)
{
    DC_RES();

    DCDynVal value = de->nullptr;
    if (let_node->value)
    {
        dc_try_or_fail_with3(DCRes, v_res, perform_evaluation_process(de, let_node->value, env), {});

        value = dc_unwrap2(v_res);
    }

    dc_try_fail_temp(DCRes, dang_env_set(env, let_node->name, &value, false));

    dc_ret_ok_dv_nullptr();
}

static DCRes eval_hash_literal(DEvaluator* de, DNodeHashTableLiteral* ht_node, DEnv* env)
{
    DC_RES();

    if (ht_node->key_values->count % 2 != 0) dc_ret_e(-1, "wrong hash literal node");

    dc_try_or_fail_with3(DCResHt, ht_res, dc_ht_new(17, hash_obj_hash_fn, hash_obj_hash_key_cmp_fn, NULL), {});

    DCHashTablePtr ht = dc_unwrap2(ht_res);

    dc_da_for(hash_lit_eval_loop, *ht_node->key_values, {
        if (_idx % 2 != 0) continue; // odd numbers are values

        // this child is the key
        dc_try_or_fail_with3(DCRes, key_obj, perform_evaluation_process(de, _it, env), {
            dc_try_fail_temp(DCResVoid, dc_ht_free(ht));
            free(ht);
        });

        // next child is the value
        dc_try_or_fail_with3(DCRes, value_obj, perform_evaluation_process(de, _it + 1, env), {
            dc_try_fail_temp(DCResVoid, dc_ht_free(ht));
            free(ht);
        });

        dc_try_or_fail_with3(DCResVoid, res,
                             dc_ht_set(ht, dc_unwrap2(key_obj), dc_unwrap2(value_obj), DC_HT_SET_CREATE_OR_UPDATE), {
                                 dc_try_fail_temp(DCResVoid, dc_ht_free(ht));
                                 free(ht);
                             });
    });

    dc_try_or_fail_with3(DCResVoid, res, dc_da_push(&de->pool, dc_dva(DCHashTablePtr, ht)), {
        dc_dbg_log("failed to push result array to the pool");
        dc_try_fail_temp(DCResVoid, dc_ht_free(ht));
        free(ht);
    });

    dc_ret_ok_dv(DCHashTablePtr, ht);
}

static DCResDa eval_children_nodes(DEvaluator* de, DCDynArrPtr source, DEnv* env)
{
    DC_RES_da();

    dc_try_fail(dc_da_new2(10, 3, NULL));

    // first child is the function identifier or expression
    // the rest is function argument
    for (usize i = 0; i < source->count; ++i)
    {
        dc_try_or_fail_with3(DCRes, arg_res, perform_evaluation_process(de, &dc_da_get2(*source, i), env), {
            dc_dbg_log("failed to evaluate the child from source");
            dc_try_fail_temp(DCResVoid, dc_da_free(dc_unwrap()));
            free(dc_unwrap());
        });

        dc_try_or_fail_with3(DCResVoid, res, dc_da_push(dc_unwrap(), dc_unwrap2(arg_res)), {
            dc_dbg_log("failed to push object to result dynamic array");
            dc_try_fail_temp(DCResVoid, dc_da_free(dc_unwrap()));
            free(dc_unwrap());
        });
    }

    dc_try_or_fail_with3(DCResVoid, res, dc_da_push(&de->pool, dc_dva(DCDynArrPtr, dc_unwrap())), {
        dc_dbg_log("failed to push result array to the pool");
        dc_try_fail_temp(DCResVoid, dc_da_free(dc_unwrap()));
        free(dc_unwrap());
    });

    dc_ret();
}

static ResEnv extend_function_env(DEvaluator* de, DCDynValPtr call_obj, DCDynArrPtr params)
{
    DC_TRY_DEF2(ResEnv, _env_new_enclosed(de, call_obj->env));

    DCDynArrPtr arr = dc_dv_as(*call_obj, DCDynArrPtr);

    if (arr->count != params->count)
        dc_ret_ea(-1, "function needs at least " dc_fmt(usize) " arguments, got=" dc_fmt(usize), params->count, arr->count);

    // extending the environment by defining arguments
    // with given evaluated objects assigning to them
    dc_da_for(extend_env_loop, *params, {
        string arg_name = dc_dv_as(*_it, DNodeIdentifier).value;

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
static DCRes apply_function(DEvaluator* de, DCDynValPtr call_obj, DNodeFunctionLiteral* fn)
{
    DC_RES();

    dc_try_or_fail_with3(ResEnv, fn_env_res, extend_function_env(de, call_obj, fn->parameters), {});

    DEnv* fn_env = dc_unwrap2(fn_env_res);

    dc_try_fail(perform_evaluation_process(de, &dc_dv(DNodeBlockStatement, dn_block(fn->body)), fn_env));

    // if we've returned of a function that's ok
    // but we don't need to pass it on to the upper level
    if_dv_is_DoReturn_return_unwrapped();

    dc_ret();
}

#if 0
static b1 is_unquote_call(DNodePtr dn)
{
    if (dn->type != DN_CALL_EXPRESSION) return false;

    DNodePtr fn = dn_child(dn, 0);

    return (fn->type == DN_IDENTIFIER) && strcmp(dn_data_as(fn, string), MACRO_UNPACK) == 0;
}

ResNode convert_do_to_nodeptr(DCDynValPtr obj)
{
    DC_RES2(ResNode);

    // DNodePtr result = NULL;

    switch (do_get_type(obj))
    {
        case DO_INTEGER:
            return dn_new(DN_INTEGER_LITERAL, *obj, false);

        case DO_BOOLEAN:
            return dn_new(DN_BOOLEAN_LITERAL, *obj, false);

        case DO_QUOTE:
        {
            DNodePtr node = dc_dv_as(*obj, DNodePtr);
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

    // convert the obj to node
    dc_try_or_fail_with3(ResNode, new_node_res, convert_do_to_nodeptr(&dc_unwrap2(unquoted_res)), {});
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

    dc_ret_ok(do_def(DNodePtr, dc_unwrap2(unquote_res), NULL));
}

#endif

// ***************************************************************************************
// * BUILTIN FUNCTIONS
// ***************************************************************************************

static DECL_DBUILTIN_FUNCTION(len)
{
    (void)de;

    BUILTIN_FN_GET_ARGS_VALIDATE("len", 1);

    DCDynVal arg = dc_da_get2(_args, 0);

    if (arg.type != DO_STRING && arg.type != DO_ARRAY)
    {
        dc_error_inita(*error, -1, "cannot calculate length of arg of type '%s'", dv_type_tostr(&arg));
        return dc_dv_nullptr();
    }

    i64 len = 0;

    if (arg.type == DO_STRING)
    {
        if (do_as_string(arg)) len = (i64)strlen(do_as_string(arg));
    }
    else if (arg.type == DO_ARRAY)
    {
        len = (i64)dc_dv_as(arg, DCDynArrPtr)->count;
    }

    return do_int(len);
}

static DECL_DBUILTIN_FUNCTION(first)
{
    (void)de;

    BUILTIN_FN_GET_ARGS_VALIDATE("first", 1);

    BUILTIN_FN_GET_ARG_NO(0, DO_ARRAY, "first argument must be an array");

    DCDynArr arr = *dc_dv_as(arg0, DCDynArrPtr);

    if (arr.count == 0) return dc_dv_nullptr();

    return dc_da_get2(arr, 0);
}

static DECL_DBUILTIN_FUNCTION(last)
{
    (void)de;

    BUILTIN_FN_GET_ARGS_VALIDATE("last", 1);

    BUILTIN_FN_GET_ARG_NO(0, DO_ARRAY, "first argument must be an array");

    DCDynArr arr = *dc_dv_as(arg0, DCDynArrPtr);

    if (arr.count == 0) return dc_dv_nullptr();

    return dc_da_get2(arr, arr.count - 1);
}

static DECL_DBUILTIN_FUNCTION(rest)
{
    (void)de;

    BUILTIN_FN_GET_ARGS_VALIDATE("rest", 1);

    BUILTIN_FN_GET_ARG_NO(0, DO_ARRAY, "first argument must be an array");

    DCDynArr arr = *dc_dv_as(arg0, DCDynArrPtr);

    if (arr.count == 0) return dc_dv_nullptr();

    DCResDa res = dc_da_new2(10, 3, NULL);
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

    DCResVoid push_res = dc_da_push(&de->pool, dc_dva(DCDynArrPtr, result_arr));
    if (dc_is_err2(push_res))
    {
        dc_error_init(*error, -1, "'rest' error: cannot push the result array to the pool");

        dc_da_free(result_arr);
        free(result_arr);

        return dc_dv_nullptr();
    }

    return dc_dv(DCDynArrPtr, result_arr);
}

static DECL_DBUILTIN_FUNCTION(push)
{
    (void)de;

    BUILTIN_FN_GET_ARGS_VALIDATE("push", 2);

    BUILTIN_FN_GET_ARG_NO(0, DO_ARRAY, "first argument must be an array");

    dc_da_push(dc_dv_as(arg0, DCDynArrPtr), dc_da_get2(_args, 1));

    return dc_dv_nullptr();
}

static DECL_DBUILTIN_FUNCTION(print)
{
    (void)de;

    (void)error;

    BUILTIN_FN_GET_ARGS;

    dc_da_for(print_loop, _args, {
        do_print(_it);
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

static DCRes perform_evaluation_process(DEvaluator* de, DCDynValPtr dn, DEnv* env)
{
    DC_RES();

    if (!dn) dc_ret_e(-1, "got NULL node");

    dc_dbg_log("evaluating node of type: '%s'", tostr_DNType(dn->type));

    switch (dn->type)
    {
        case dc_dvt(DNodeProgram):
            return eval_program_statements(de, dc_dv_as(*dn, DNodeProgram).statements, env);

        case dc_dvt(DNodePrefixExpression):
        {
            DNodePrefixExpression prefix_node = dc_dv_as(*dn, DNodePrefixExpression);

            dc_try_or_fail_with3(DCRes, operand, perform_evaluation_process(de, prefix_node.operand, env), {});

            return eval_prefix_expression(prefix_node.op, &dc_unwrap2(operand));
        }

        case dc_dvt(DNodeInfixExpression):
        {
            DNodeInfixExpression infix_node = dc_dv_as(*dn, DNodeInfixExpression);

            dc_try_or_fail_with3(DCRes, left, perform_evaluation_process(de, infix_node.left, env), {});
            dc_try_or_fail_with3(DCRes, right, perform_evaluation_process(de, infix_node.right, env), {});

            return eval_infix_expression(de, infix_node.op, &dc_unwrap2(left), &dc_unwrap2(right));
        }

        case DO_BOOLEAN:
        case DO_INTEGER:
        case DO_STRING:
        {
            DCDynVal result = *dn;
            result.allocated = false;
            result.env = NULL;
            dc_ret_ok(result);
        }

        case dc_dvt(DNodeIdentifier):
        {
            string name = dc_dv_as(*dn, DNodeIdentifier).value;

            DCRes symbol = dang_env_get(env, name);

            if (dc_is_ok2(symbol)) return symbol;

            return find_builtin(name);
        }

        case dc_dvt(DNodeBlockStatement):
        {
            DCDynArrPtr statements = dc_dv_as(*dn, DNodeBlockStatement).statements;
            return eval_block_statements(de, statements, env);
        }

        case dc_dvt(DNodeIfExpression):
            return eval_if_expression(de, &dc_dv_as(*dn, DNodeIfExpression), env);

        case dc_dvt(DNodeReturnStatement):
        {
            DNodeReturnStatement ret_node = dc_dv_as(*dn, DNodeReturnStatement);

            if (!ret_node.ret_val) dc_ret_ok_dv(DoReturn, do_return(&de->nullptr));

            dc_try_or_fail_with3(DCRes, value, perform_evaluation_process(de, ret_node.ret_val, env), {});

            dc_try_fail_temp(DCResVoid, dc_da_push(&de->pool, dc_unwrap2(value)));

            dc_ret_ok_dv(DoReturn, do_return(pool_last_el(de)));
        }

        case dc_dvt(DNodeLetStatement):
            return eval_let_statement(de, &dc_dv_as(*dn, DNodeLetStatement), env);

        case dc_dvt(DNodeFunctionLiteral):
        {
            // function literal holds actual node in object's dv
            // also holds the pointer to the environment it's being evaluated
            DCDynVal res = *dn;
            res.env = env;
            dc_ret_ok(res);
        }

        case dc_dvt(DNodeArrayLiteral):
        {
            DCDynArrPtr arr = dc_dv_as(*dn, DNodeArrayLiteral).array;

            dc_try_or_fail_with3(DCResDa, temp_res, eval_children_nodes(de, arr, env), {});

            dc_ret_ok_dv(DCDynArrPtr, dc_unwrap2(temp_res));
        }

        case dc_dvt(DNodeHashTableLiteral):
            return eval_hash_literal(de, &dc_dv_as(*dn, DNodeHashTableLiteral), env);

        case dc_dvt(DNodeIndexExpression):
        {
            DNodeIndexExpression index_exp = dc_dv_as(*dn, DNodeIndexExpression);

            dc_try_or_fail_with3(DCRes, operand_res, perform_evaluation_process(de, index_exp.operand, env), {});
            DCDynVal operand = dc_unwrap2(operand_res);

            dc_try_or_fail_with3(DCRes, index_res, perform_evaluation_process(de, index_exp.index, env), {});
            DCDynVal index = dc_unwrap2(index_res);

            if (operand.type != DO_ARRAY && operand.type != DO_HASH_TABLE)
            {
                dc_dbg_log("indexing of '%s' is not supporting on '%s'", dv_type_tostr(index), dv_type_tostr(operand));
                dc_ret_e(-1, "indexing is not supporting on this type");
            }

            else if (operand.type == DO_ARRAY && index.type != DO_INTEGER)
            {
                dc_dbg_log("indexing of '%s' is not supporting on '%s'", dv_type_tostr(index), dv_type_tostr(operand));
                dc_ret_e(-1, "indexing is not supporting on this type");
            }

            if (do_as_int(index) < 0) dc_ret_ok_dv_nullptr();

            if (operand.type == DO_ARRAY) return eval_array_index_expression(&operand, &index);

            return eval_hash_index_expression(&operand, &index);
        }

        case dc_dvt(DNodeCallExpression):
        {
            DNodeCallExpression call_exp = dc_dv_as(*dn, DNodeCallExpression);

            if (call_exp.function->type == dc_dvt(DNodeIdentifier) &&
                strcmp(dc_dv_as(*call_exp.function, DNodeIdentifier).value, "quote") == 0)
            {
                // todo:: finish this
                // return process_quoted(dn, de, main_de);
            }

            // evaluating it must return a function object
            // that is in fact the function literal node saved in the environment before
            // it also holds pointer to its environment
            dc_try_or_fail_with3(DCRes, fn_res, perform_evaluation_process(de, call_exp.function, env), {});

            DCDynVal fn_obj = dc_unwrap2(fn_res);

            if (fn_obj.type != DO_FUNCTION && fn_obj.type != DO_BUILTIN_FUNCTION)
            {
                dc_dbg_log("not a function got: '%s'", dv_type_tostr(&fn_obj));

                dc_ret_ea(-1, "not a function got: '%s'", dv_type_tostr(&fn_obj));
            }

            // eval arguments (first element is function symbol the rest is arguments)
            // so we start evaluating children at index 1
            dc_try_or_fail_with3(DCResDa, call_obj_res, eval_children_nodes(de, call_exp.arguments, env), {});

            // this is a temporary object to hold the evaluated children and the env
            DCDynVal call_obj = dc_dv(DCDynArrPtr, dc_unwrap2(call_obj_res));

            if (fn_obj.type == DO_BUILTIN_FUNCTION)
            {
                DBuiltinFunction fn = dc_dv_as(fn_obj, DBuiltinFunction);

                DCError error = (DCError){0};

                DCDynVal result = fn(de, &call_obj, &error);

                if (error.code != 0)
                {
                    dc_status() = DC_RES_ERR;
                    dc_err() = error;
                    dc_ret();
                }

                dc_ret_ok(result);
            }

            call_obj.env = fn_obj.env;
            return apply_function(de, &call_obj, &dc_dv_as(fn_obj, DNodeFunctionLiteral));
        }

        default:
            break;
    };

    dc_ret_ea(-1, "Unimplemented or unsupported node type: %s", dv_type_tostr(dn));
}

// ***************************************************************************************
// * PUBLIC FUNCTIONS
// ***************************************************************************************

DCResVoid dang_env_init(DEnvPtr env)
{
    DC_RES_void();

    dc_try_or_fail_with3(DCResVoid, res, dc_ht_init(&env->memory, 17, env_hash_fn, string_key_cmp, NULL),
                         { dc_dbg_log("cannot initialize dang environment hash table"); });

    env->outer = NULL;

    dc_ret();
}

DCResVoid de_init(DEvaluator* de)
{
    DC_RES_void();

    dc_try_fail(dang_env_init(&de->main_env));

    dc_try_fail(dc_da_init2(&de->pool, 50, 3, NULL));
    dc_try_fail(dc_da_init2(&de->errors, 20, 2, NULL));

    dc_try_or_fail_with(dang_parser_init(&de->parser, &de->pool, &de->errors),
                        dc_try_fail_temp(DCResVoid, dang_env_free(&de->main_env)));

    de->nullptr = dc_dv_nullptr();

    dc_ret();
}

DCResVoid de_free(DEvaluator* de)
{
    DC_RES_void();

    dc_try_fail(dang_env_free(&de->main_env));

    dc_try_fail(dc_da_free(&de->pool));

    dc_try_fail(dc_da_free(&de->errors));

    dc_ret();
}

ResEvaluated dang_eval(DEvaluator* de, const string source, b1 inspect)
{
    DC_RES2(ResEvaluated);

    if (!de) dc_ret_e(dc_e_code(NV), "cannot evaluate using NULL evaluator");
    if (!source) dc_ret_e(dc_e_code(NV), "cannot run evaluation on NULL source");

    dc_try_or_fail_with3(ResDNodeProgram, program_res, dang_parse(&de->parser, source), {});

    DNodeProgram program = dc_unwrap2(program_res);

    dc_try_or_fail_with3(DCRes, result, perform_evaluation_process(de, &dc_dv(DNodeProgram, program), &de->main_env), {});

    string inspect_str = NULL;

    if (inspect)
    {
        dc_try_fail_temp(DCResVoid, dang_program_inspect(&program, &inspect_str));

        if (inspect_str)
        {
            dc_try_or_fail_with3(DCResVoid, res, dc_da_push(&de->pool, dc_dva(string, inspect_str)), free(inspect_str));
        }
    }

    dc_ret_ok(dang_evaluated(dc_unwrap2(result), inspect_str));
}

DCResString do_tostr(DCDynValPtr obj)
{
    DC_RES_string();

    string result = NULL;

    switch (obj->type)
    {
        case DO_HASH_TABLE:
        case DO_ARRAY:
        case DO_BOOLEAN:
        case DO_INTEGER:
        case DO_STRING:
            return dc_tostr_dv(obj);
            break;

            // case DO_QUOTE:
            //     // todo:: it must inspect the 'node' of the object
            //     // dang_node_inspect(dc_dv_as(*obj, DNodePtr), &result);
            //     break;

            // case DO_MACRO:
            //     dc_sprintf(&result, "%s", "(macro)");
            //     break;

        case DO_FUNCTION:
            dc_sprintf(&result, "%s", "(function)");
            break;

        case DO_BUILTIN_FUNCTION:
            dc_sprintf(&result, "%s", "(builtin function)");
            break;

        case dc_dvt(voidptr):
            if (dc_dv_as(*obj, voidptr) == NULL) dc_sprintf(&result, "%s", "(null)");
            break;

        default:
            dc_sprintf(&result, "%s", "(unknown object)");
            break;
    }

    dc_ret_ok(result);
}

void do_print(DCDynValPtr obj)
{
    DCResString res = do_tostr(obj);

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
    return _env_new();
}

DCResVoid dang_env_free(DEnv* de)
{
    DC_RES_void();

    dc_dbg_log("number of memory elements: " dc_fmt(usize) ", number of cleanup elements: " dc_fmt(usize), de->memory.key_count,
               de->cleanups.count);

    dc_try_fail(dc_ht_free(&de->memory));

    dc_ret();
}


DCRes dang_env_get(DEnv* env, string name)
{
    DC_RES();

    DCDynValPtr found = NULL;

    dc_try_fail_temp(DCResUsize, dc_ht_find_by_key(&env->memory, dc_dv(string, name), &found));

    if (found) dc_ret_ok(*found);

    if (env->outer) return dang_env_get(env->outer, name);

    dc_dbg_log("key '%s' not found in the environment", name);

    dc_ret_ea(6, "'%s' is not defined", name);
}

DCRes dang_env_set(DEnv* env, string name, DCDynValPtr value, b1 update_only)
{
    DC_RES();

    DCDynVal val_to_save = value ? *value : dc_dv_nullptr();

    DCResVoid res = dc_ht_set(&env->memory, dc_dv(string, name), val_to_save,
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

#if 0
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
#endif