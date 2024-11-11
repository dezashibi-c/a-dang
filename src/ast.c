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
