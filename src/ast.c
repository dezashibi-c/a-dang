// ***************************************************************************************
//    Project: Dang Compiler -> https://github.com/dezashibi-c/dang
//    File: ast.c
//    Date: 2024-09-14
//    Author: Navid Dezashibi
//    Contact: navid@dezashibi.com
//    Website: https://dezashibi.com | https://github.com/dezashibi
//    License:
//     Please refer to the LICENSE file, repository or website for more
//     information about the licensing of this work. If you have any questions
//     or concerns, please feel free to contact me at the email address provided
//     above.
// ***************************************************************************************
// *  Description: ast structs and related functionalities
// ***************************************************************************************

#include "ast.h"

#define pool_last_el() &dc_da_get2(*pool, pool->count - 1)

static DCResVoid array_inspector(DCDynArrPtr darr, string prefix, string postfix, string delimiter, b1 no_delim_for_last,
                                 string* result)
{
    DC_RES_void();

    if (prefix && prefix[0] != '\0') dc_sappend(result, "%s", prefix);

    if (darr != NULL)
    {
        dc_da_for(arr_inspector_loop, *darr, {
            dc_try_fail(dang_node_inspect(_it, result));

            if (delimiter && delimiter[0] != '\0')
            {
                if (!no_delim_for_last)
                    dc_sappend(result, "%s", delimiter);

                else if (_idx < darr->count - 1)
                    dc_sappend(result, "%s", delimiter);
            }
        });
    }

    if (postfix && postfix[0] != '\0') dc_sappend(result, "%s", postfix);

    dc_ret();
}

DCResVoid dang_program_inspect(DNodeProgram* program, string* result)
{
    DC_RES_void();

    dc_try_fail(array_inspector(program->statements, NULL, NULL, "\n", false, result));

    dc_ret();
}

DCResVoid dang_node_inspect(DCDynValPtr dn, string* result)
{
#define append_data_str(FMT)                                                                                                   \
    do                                                                                                                         \
    {                                                                                                                          \
        dc_try_or_fail_with3(DCResString, data_str_res, dc_tostr_dv(dn), {});                                                  \
        dc_sappend(result, FMT, dc_unwrap2(data_str_res));                                                                     \
        if (dc_unwrap2(data_str_res)) free(dc_unwrap2(data_str_res));                                                          \
    } while (0)


    DC_RES_void();

    if (!dn)
    {
        dc_dbg_log("%s", "cannot inspect null node");

        dc_ret_e(dc_e_code(NV), "cannot inspect null node");
    }

    dc_dbg_log("inspecting node type: %s", tostr_DNType(dn->type));

    switch (dn->type)
    {
        case dc_dvt(DCDynValPtr):
            return dang_node_inspect(dc_dv_as(*dn, DCDynValPtr), result);

        case dc_dvt(DNodeIdentifier):
            dc_sappend(result, "%s", dc_dv_as(*dn, DNodeIdentifier).value);
            break;

        case dc_dvt(DNodeProgram):
        {
            DCDynArrPtr statements = dc_dv_as(*dn, DNodeProgram).statements;

            dc_try_fail(array_inspector(statements, NULL, NULL, "\n", false, result));

            break;
        }

        case dc_dvt(DNodeLetStatement):
        {
            DNodeLetStatement let_stmt = dc_dv_as(*dn, DNodeLetStatement);

            dc_sappend(result, "let %s", let_stmt.name);

            if (let_stmt.value)
            {
                dc_sappend(result, "%s", " ");
                dc_try_fail(dang_node_inspect(let_stmt.value, result));
            }

            break;
        }

        case dc_dvt(DNodeReturnStatement):
        {
            dc_sappend(result, "%s", "return");

            DNodeReturnStatement ret_stmt = dc_dv_as(*dn, DNodeReturnStatement);

            if (ret_stmt.ret_val)
            {
                dc_sappend(result, "%s", " ");
                dc_try_fail(dang_node_inspect(ret_stmt.ret_val, result));
            }

            break;
        }

        case dc_dvt(DNodePrefixExpression):
        {
            DNodePrefixExpression prefix_exp = dc_dv_as(*dn, DNodePrefixExpression);

            dc_sappend(result, "(%s", prefix_exp.op);
            dc_try_fail(dang_node_inspect(prefix_exp.operand, result));
            dc_sappend(result, "%s", ")");
            break;
        }

        case dc_dvt(DNodeInfixExpression):
        {
            DNodeInfixExpression infix_exp = dc_dv_as(*dn, DNodeInfixExpression);

            dc_sappend(result, "%s", "(");
            dc_try_fail(dang_node_inspect(infix_exp.left, result));
            dc_sappend(result, " %s ", infix_exp.op);
            dc_try_fail(dang_node_inspect(infix_exp.right, result));
            dc_sappend(result, "%s", ")");
            break;
        }

        case dc_dvt(DNodeIfExpression):
        {
            DNodeIfExpression if_exp = dc_dv_as(*dn, DNodeIfExpression);

            dc_sappend(result, "%s", "if ");

            dc_try_fail(dang_node_inspect(if_exp.condition, result));

            dc_sappend(result, "%s", " ");

            dc_try_fail(array_inspector(if_exp.consequence, "{ ", "}", "; ", false, result));

            if (if_exp.alternative)
            {
                dc_sappend(result, "%s", " else ");
                dc_try_fail(array_inspector(if_exp.alternative, "{ ", "}", "; ", false, result));
            }

            break;
        }

        case dc_dvt(DNodeBlockStatement):
        {
            DCDynArrPtr statements = dc_dv_as(*dn, DNodeBlockStatement).statements;

            dc_try_fail(array_inspector(statements, "{ ", "}", "; ", false, result));

            break;
        }

        case dc_dvt(DNodeFunctionLiteral):
        {
            DNodeFunctionLiteral func_lit = dc_dv_as(*dn, DNodeFunctionLiteral);

            dc_try_fail(array_inspector(func_lit.parameters, "Fn (", ") ", ", ", true, result));

            // The Body
            dc_try_fail(array_inspector(func_lit.body, "{ ", "}", "; ", false, result));

            break;
        }

        case dc_dvt(DNodeMacro):
        {
            DNodeMacro macro = dc_dv_as(*dn, DNodeMacro);

            dc_try_fail(array_inspector(macro.parameters, "MACRO (", ") ", ", ", true, result));

            // The Body
            dc_try_fail(array_inspector(macro.body, "{ ", "}", "; ", false, result));

            break;
        }

        case dc_dvt(DNodeCallExpression):
        {
            DNodeCallExpression call_exp = dc_dv_as(*dn, DNodeCallExpression);

            if (call_exp.function->type == dc_dvt(DNodeIdentifier) &&
                strcmp(dc_dv_as(*call_exp.function, DNodeIdentifier).value, QUOTE) == 0)
                dc_sappend(result, "%s", "QUOTE");
            else
                dc_try_fail(dang_node_inspect(call_exp.function, result));

            dc_try_fail(array_inspector(call_exp.arguments, "(", ")", ", ", true, result));

            break;
        }

        case dc_dvt(DNodeArrayLiteral):
        {
            DCDynArrPtr arr_lit = dc_dv_as(*dn, DNodeArrayLiteral).array;
            dc_try_fail(array_inspector(arr_lit, "[", "]", ", ", true, result));

            break;
        }

        case dc_dvt(DNodeIndexExpression):
        {
            DNodeIndexExpression index_exp = dc_dv_as(*dn, DNodeIndexExpression);

            dc_sappend(result, "%s", "(");
            dc_try_fail(dang_node_inspect(index_exp.operand, result));

            dc_sappend(result, "%s", "[");
            dc_try_fail(dang_node_inspect(index_exp.index, result));
            dc_sappend(result, "%s", "])");

            break;
        }

        case dc_dvt(string):
            append_data_str("\"%s\"");
            break;

        case dc_dvt(DNodeHashTableLiteral):
        {
            DCDynArr key_values = *dc_dv_as(*dn, DNodeHashTableLiteral).key_values;

            dc_sappend(result, "%s", "{");

            dc_da_for(hash_table_loop, key_values, {
                dc_try_fail(dang_node_inspect(_it, result));

                if (_idx % 2 == 0)
                    dc_sappend(result, "%s", ": ");
                else if (_idx < key_values.count - 1)
                    dc_sappend(result, "%s", ", ");
            });

            dc_sappend(result, "%s", "}");
            break;
        }

        default:
            append_data_str("%s");
            break;


            // DN_WHILE_EXPRESSION,
            // DN_MACRO_LITERAL,
    };

    dc_ret();

#undef append_data_str
}

DCRes dang_node_copy(DCDynValPtr dn, DCDynArrPtr pool)
{
    DC_RES();

    switch (dn->type)
    {
        case dc_dvt(DCDynValPtr):
            return dang_node_copy(dc_dv_as(*dn, DCDynValPtr), pool);

        case dc_dvt(DCDynArrPtr):
        {
            DCDynArrPtr array = dc_dv_as(*dn, DCDynArrPtr);

            dc_try_or_fail_with3(DCResDa, da_res, dc_da_new(NULL), {});

            dc_da_for(array_copy_loop, *array, {
                dc_try_or_fail_with3(DCRes, copy_res, dang_node_copy(_it, pool),
                                     dc_try_fail_temp(DCResVoid, dc_da_free(dc_unwrap2(da_res))));

                dc_try_or_fail_with3(DCResVoid, push_res, dc_da_push(dc_unwrap2(da_res), dc_unwrap2(copy_res)),
                                     dc_try_fail_temp(DCResVoid, dc_da_free(dc_unwrap2(da_res))));
            });

            dc_ret_ok_dva(DCDynArrPtr, dc_unwrap2(da_res));
        }

        case dc_dvt(DNodeIdentifier):
        {
            DNodeIdentifier ident = dc_dv_as(*dn, DNodeIdentifier);

            string new_value = malloc(strlen(ident.value) + 1);
            strcpy(new_value, ident.value);

            dc_try_or_fail_with3(DCResVoid, push_res, dc_da_push(pool, dc_dva(string, new_value)), {});

            ident.value = new_value;

            dc_ret_ok_dv(DNodeIdentifier, ident);
        }

        case dc_dvt(DNodeProgram):
        {
            DNodeProgram program = dc_dv_as(*dn, DNodeProgram);

            dc_try_or_fail_with3(DCRes, statement_copy, dang_node_copy(&dc_dv(DCDynArrPtr, program.statements), pool), {});

            dc_try_or_fail_with3(DCResVoid, push_res, dc_da_push(pool, dc_unwrap2(statement_copy)), {});

            program.statements = dc_dv_as(dc_unwrap2(statement_copy), DCDynArrPtr);

            dc_ret_ok_dv(DNodeProgram, program);
        }

        case dc_dvt(DNodeLetStatement):
        {
            DNodeLetStatement let_stmt = dc_dv_as(*dn, DNodeLetStatement);

            dc_try_or_fail_with3(DCRes, value_res, dang_node_copy(let_stmt.value, pool), {});

            dc_try_or_fail_with3(DCResVoid, push_res, dc_da_push(pool, dc_unwrap2(value_res)), {});

            let_stmt.value = pool_last_el();

            dc_ret_ok_dv(DNodeLetStatement, let_stmt);
        }

        case dc_dvt(DNodeReturnStatement):
        {
            DNodeReturnStatement ret_stmt = dc_dv_as(*dn, DNodeReturnStatement);

            dc_try_or_fail_with3(DCRes, ret_val_res, dang_node_copy(ret_stmt.ret_val, pool), {});

            dc_try_or_fail_with3(DCResVoid, push_res, dc_da_push(pool, dc_unwrap2(ret_val_res)), {});

            ret_stmt.ret_val = pool_last_el();

            dc_ret_ok_dv(DNodeReturnStatement, ret_stmt);
        }

        case dc_dvt(DNodePrefixExpression):
        {
            DNodePrefixExpression prefix_exp = dc_dv_as(*dn, DNodePrefixExpression);

            dc_try_or_fail_with3(DCRes, operand_res, dang_node_copy(prefix_exp.operand, pool), {});

            dc_try_or_fail_with3(DCResVoid, push_res, dc_da_push(pool, dc_unwrap2(operand_res)), {});

            prefix_exp.operand = pool_last_el();

            dc_ret_ok_dv(DNodePrefixExpression, prefix_exp);
        }

        case dc_dvt(DNodeInfixExpression):
        {
            DNodeInfixExpression infix_exp = dc_dv_as(*dn, DNodeInfixExpression);

            dc_try_or_fail_with3(DCRes, left_res, dang_node_copy(infix_exp.left, pool), {});

            dc_try_or_fail_with3(DCResVoid, push_res, dc_da_push(pool, dc_unwrap2(left_res)), {});

            infix_exp.left = pool_last_el();

            dc_try_or_fail_with3(DCRes, right_res, dang_node_copy(infix_exp.right, pool), {});

            dc_try_or_fail_with2(push_res, dc_da_push(pool, dc_unwrap2(right_res)), {});

            infix_exp.right = pool_last_el();

            dc_ret_ok_dv(DNodeInfixExpression, infix_exp);
        }

        case dc_dvt(DNodeIfExpression):
        {
            DNodeIfExpression if_exp = dc_dv_as(*dn, DNodeIfExpression);

            dc_try_or_fail_with3(DCRes, condition_res, dang_node_copy(if_exp.condition, pool), {});

            dc_try_or_fail_with3(DCResVoid, push_res, dc_da_push(pool, dc_unwrap2(condition_res)), {});

            if_exp.condition = pool_last_el();


            dc_try_or_fail_with3(DCRes, consequence_res, dang_node_copy(&dc_dv(DCDynArrPtr, if_exp.consequence), pool), {});

            dc_try_or_fail_with2(push_res, dc_da_push(pool, dc_unwrap2(consequence_res)), {});

            if_exp.consequence = dc_dv_as(dc_unwrap2(consequence_res), DCDynArrPtr);


            if (if_exp.alternative)
            {
                dc_try_or_fail_with3(DCRes, alternative_res, dang_node_copy(&dc_dv(DCDynArrPtr, if_exp.alternative), pool), {});

                dc_try_or_fail_with2(push_res, dc_da_push(pool, dc_unwrap2(alternative_res)), {});

                if_exp.alternative = dc_dv_as(dc_unwrap2(alternative_res), DCDynArrPtr);
            }


            dc_ret_ok_dv(DNodeIfExpression, if_exp);
        }

        case dc_dvt(DNodeBlockStatement):
        {
            DNodeBlockStatement block = dc_dv_as(*dn, DNodeBlockStatement);

            dc_try_or_fail_with3(DCRes, statement_copy, dang_node_copy(&dc_dv(DCDynArrPtr, block.statements), pool), {});

            dc_try_or_fail_with3(DCResVoid, push_res, dc_da_push(pool, dc_unwrap2(statement_copy)), {});

            block.statements = dc_dv_as(dc_unwrap2(statement_copy), DCDynArrPtr);

            dc_ret_ok_dv(DNodeBlockStatement, block);
        }

        case dc_dvt(DNodeFunctionLiteral):
        {
            DNodeFunctionLiteral func_lit = dc_dv_as(*dn, DNodeFunctionLiteral);

            dc_try_or_fail_with3(DCRes, parameters_copy, dang_node_copy(&dc_dv(DCDynArrPtr, func_lit.parameters), pool), {});

            dc_try_or_fail_with3(DCResVoid, push_res, dc_da_push(pool, dc_unwrap2(parameters_copy)), {});

            func_lit.parameters = dc_dv_as(dc_unwrap2(parameters_copy), DCDynArrPtr);

            dc_try_or_fail_with3(DCRes, body_copy, dang_node_copy(&dc_dv(DCDynArrPtr, func_lit.body), pool), {});

            dc_try_or_fail_with2(push_res, dc_da_push(pool, dc_unwrap2(body_copy)), {});

            func_lit.body = dc_dv_as(dc_unwrap2(body_copy), DCDynArrPtr);

            dc_ret_ok_dv(DNodeFunctionLiteral, func_lit);
        }

        case dc_dvt(DNodeMacro):
        {
            DNodeMacro macro = dc_dv_as(*dn, DNodeMacro);

            dc_try_or_fail_with3(DCRes, parameters_copy, dang_node_copy(&dc_dv(DCDynArrPtr, macro.parameters), pool), {});

            dc_try_or_fail_with3(DCResVoid, push_res, dc_da_push(pool, dc_unwrap2(parameters_copy)), {});

            macro.parameters = dc_dv_as(dc_unwrap2(parameters_copy), DCDynArrPtr);

            dc_try_or_fail_with3(DCRes, body_copy, dang_node_copy(&dc_dv(DCDynArrPtr, macro.body), pool), {});

            dc_try_or_fail_with2(push_res, dc_da_push(pool, dc_unwrap2(body_copy)), {});

            macro.body = dc_dv_as(dc_unwrap2(body_copy), DCDynArrPtr);

            dc_ret_ok_dv(DNodeMacro, macro);
        }

        case dc_dvt(DNodeCallExpression):
        {
            DNodeCallExpression call_exp = dc_dv_as(*dn, DNodeCallExpression);

            dc_try_or_fail_with3(DCRes, function_res, dang_node_copy(call_exp.function, pool), {});

            dc_try_or_fail_with3(DCResVoid, push_res, dc_da_push(pool, dc_unwrap2(function_res)), {});

            call_exp.function = pool_last_el();

            dc_try_or_fail_with3(DCRes, arguments_copy, dang_node_copy(&dc_dv(DCDynArrPtr, call_exp.arguments), pool), {});

            dc_try_or_fail_with2(push_res, dc_da_push(pool, dc_unwrap2(arguments_copy)), {});

            call_exp.arguments = dc_dv_as(dc_unwrap2(arguments_copy), DCDynArrPtr);

            dc_ret_ok_dv(DNodeCallExpression, call_exp);
        }

        case dc_dvt(DNodeArrayLiteral):
        {
            DCDynArrPtr arr_lit = dc_dv_as(*dn, DNodeArrayLiteral).array;

            break;
        }

        case dc_dvt(DNodeIndexExpression):
        {
            DNodeIndexExpression index_exp = dc_dv_as(*dn, DNodeIndexExpression);


            break;
        }

        case dc_dvt(DNodeHashTableLiteral):
        {
            DCDynArr key_values = *dc_dv_as(*dn, DNodeHashTableLiteral).key_values;


            break;
        }

        case dc_dvt(DoQuote):
        {
            DoQuote quote = dc_dv_as(*dn, DoQuote);

            dc_try_or_fail_with3(DCRes, node_res, dang_node_copy(quote.node, pool), {});

            dc_try_or_fail_with3(DCResVoid, push_res, dc_da_push(pool, dc_unwrap2(node_res)), {});

            quote.node = pool_last_el();

            dc_ret_ok_dv(DoQuote, quote);
        }

        default:
            break;
    };

    dc_ret_ok(*dn);
}
