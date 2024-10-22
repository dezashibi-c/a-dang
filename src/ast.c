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

string tostr_DNType(DNType dnt)
{
    switch (dnt)
    {
        dc_str_case(DN__UNKNOWN);
        dc_str_case(DN_PROGRAM);
        dc_str_case(DN__START_STATEMENT);
        dc_str_case(DN_LET_STATEMENT);
        dc_str_case(DN_RETURN_STATEMENT);
        dc_str_case(DN_EXPRESSION_STATEMENT);
        dc_str_case(DN_BLOCK_STATEMENT);
        dc_str_case(DN__END_STATEMENT);
        dc_str_case(DN__START_EXPRESSION);
        dc_str_case(DN_IDENTIFIER);
        dc_str_case(DN_PREFIX_EXPRESSION);
        dc_str_case(DN_INFIX_EXPRESSION);
        dc_str_case(DN_IF_EXPRESSION);
        dc_str_case(DN_WHILE_EXPRESSION);
        dc_str_case(DN_CALL_EXPRESSION);
        dc_str_case(DN_INDEX_EXPRESSION);
        dc_str_case(DN__END_EXPRESSION);
        dc_str_case(DN__START_LITERAL);
        dc_str_case(DN_FUNCTION_LITERAL);
        dc_str_case(DN_ARRAY_LITERAL);
        dc_str_case(DN_HASH_LITERAL);
        dc_str_case(DN_MACRO_LITERAL);
        dc_str_case(DN_BOOLEAN_LITERAL);
        dc_str_case(DN_STRING_LITERAL);
        dc_str_case(DN_INTEGER_LITERAL);
        dc_str_case(DN__END_LITERAL);
        dc_str_case(DN__MAX);
    };

    return NULL;
}

DCResVoid dang_node_inspect(DNode* dn, string* result)
{
    DC_RES_void();

    if (!dn)
    {
        dc_dbg_log("%s", "cannot inspect null node");

        dc_ret_e(dc_e_code(NV), "cannot inspect null node");
    }

    if (!dn_type_is_valid(dn->type))
    {
        dc_dbg_log("cannot inspect invalid node type: %s", tostr_DNType(dn->type));

        dc_ret_e(dc_e_code(TYPE), "cannot inspect invalid node type");
    }

    dc_dbg_log("inspecting node type: %s", tostr_DNType(dn->type));

    switch (dn->type)
    {
        case DN_PROGRAM:
            dc_da_for(dn->children, {
                dc_try_fail(dang_node_inspect(dn_child(dn, _idx), result));
                dc_sappend(result, "%s", "\n");
            });

            break;

        case DN_LET_STATEMENT:
            dc_sappend(result, "%s", "let ");
            dc_try_fail(dang_node_inspect(dn_child(dn, 0), result));

            if (dn_child_count(dn) > 1)
            {
                dc_sappend(result, "%s", " ");
                dc_try_fail(dang_node_inspect(dn_child(dn, 1), result));
            }

            break;

        case DN_RETURN_STATEMENT:
            dc_sappend(result, "%s", "return");

            if (dn_child_count(dn) > 0)
            {
                dc_sappend(result, "%s", " ");
                dc_try_fail(dang_node_inspect(dn_child(dn, 0), result));
            }

            break;

        case DN_EXPRESSION_STATEMENT:
            if (dn_child_count(dn) > 0) dc_try_fail(dang_node_inspect(dn_child(dn, 0), result));

            break;

        case DN_PREFIX_EXPRESSION:
        {
            dc_try_or_fail_with3(DCResString, data_str_res, dc_tostr_dv(&dn->data), {});
            dc_sappend(result, "(%s", dc_unwrap2(data_str_res));
            dc_try_fail(dang_node_inspect(dn_child(dn, 0), result));
            dc_sappend(result, "%s", ")");
            break;
        }

        case DN_INFIX_EXPRESSION:
        {
            dc_sappend(result, "%s", "(");
            dc_try_fail(dang_node_inspect(dn_child(dn, 0), result));
            dc_try_or_fail_with3(DCResString, data_str_res, dc_tostr_dv(&dn->data), {});
            dc_sappend(result, " %s ", dc_unwrap2(data_str_res));
            dc_try_fail(dang_node_inspect(dn_child(dn, 1), result));
            dc_sappend(result, "%s", ")");
            break;
        }

        case DN_IF_EXPRESSION:
            dc_sappend(result, "%s", "if ");

            dc_try_fail(dang_node_inspect(dn_child(dn, 0), result));

            dc_sappend(result, "%s", " ");

            dc_try_fail(dang_node_inspect(dn_child(dn, 1), result));

            if (dn_child_count(dn) > 2)
            {
                dc_sappend(result, "%s", " else ");
                dc_try_fail(dang_node_inspect(dn_child(dn, 2), result));
            }

            break;

        case DN_BLOCK_STATEMENT:
            dc_sappend(result, "%s", "{ ");
            dc_da_for(dn->children, {
                dc_try_fail(dang_node_inspect(dn_child(dn, _idx), result));
                dc_sappend(result, "%s", "; ");
            });
            dc_sappend(result, "%s", "}");

            break;

        case DN_FUNCTION_LITERAL:
            dc_sappend(result, "%s", "Fn (");

            dc_da_for(dn->children, {
                if (_idx == dn_child_count(dn) - 1) break;

                dc_try_fail(dang_node_inspect(dn_child(dn, _idx), result));

                if (_idx < dn_child_count(dn) - 2) dc_sappend(result, "%s", ", ");
            });

            dc_sappend(result, "%s", ") ");

            // The Body
            dc_try_fail(dang_node_inspect(dn_child(dn, dn_child_count(dn) - 1), result));

            break;

        case DN_CALL_EXPRESSION:
            dc_da_for(dn->children, {
                dc_try_fail(dang_node_inspect(dn_child(dn, _idx), result));

                if (_idx == 0)
                {
                    dc_sappend(result, "%s", "(");
                    continue;
                }

                if (_idx < dn_child_count(dn) - 1) dc_sappend(result, "%s", ", ");
            });

            dc_sappend(result, "%s", ")");

            break;

        case DN_ARRAY_LITERAL:
            dc_sappend(result, "%s", "[");
            dc_da_for(dn->children, {
                dc_try_fail(dang_node_inspect(dn_child(dn, _idx), result));

                if (_idx < dn_child_count(dn) - 1) dc_sappend(result, "%s", ", ");
            });

            dc_sappend(result, "%s", "]");

            break;

        case DN_INDEX_EXPRESSION:
            dc_sappend(result, "%s", "(");
            dc_try_fail(dang_node_inspect(dn_child(dn, 0), result));

            dc_sappend(result, "%s", "[");
            dc_try_fail(dang_node_inspect(dn_child(dn, 1), result));
            dc_sappend(result, "%s", "])");

            break;

        case DN_STRING_LITERAL:
        {
            dc_try_or_fail_with3(DCResString, data_str_res, dc_tostr_dv(&dn->data), {});

            dc_sappend(result, "\"%s\"", dc_unwrap2(data_str_res));
            break;
        }

        case DN_BOOLEAN_LITERAL:
        {
            dc_try_or_fail_with3(DCResBool, data_bool_res, dc_dv_as_bool(&dn->data), {});

            dc_sappend(result, "%s", dc_tostr_bool(dc_unwrap2(data_bool_res)));
            break;
        }

        default:
        {
            dc_try_or_fail_with3(DCResString, data_str_res, dc_tostr_dv(&dn->data), {});

            dc_sappend(result, "%s", dc_unwrap2(data_str_res));
            break;
        }


            // DN_IDENTIFIER,
            // DN_WHILE_EXPRESSION,

            // DN_HASH_LITERAL,
            // DN_MACRO_LITERAL,
            // DN_BOOLEAN_LITERAL,
            // DN_INTEGER_LITERAL,
    };

    dc_ret();
}

ResNode dn_new(DNType type, DCDynVal data, bool has_children)
{
    DC_RES2(ResNode);

    DNode* node = malloc(sizeof(DNode));
    if (node == NULL)
    {
        dc_dbg_log("DNode Memory allocation failed");

        dc_ret_e(2, "DNode Memory allocation failed");
    }

    node->type = type;
    node->data = data;

    node->children = (DCDynArr){0};

    if (has_children) dc_try_fail_temp(DCResVoid, dc_da_init(&node->children, dn_child_free));

    dc_ret_ok(node);
}

DCResVoid dn_program_free(DNode* program)
{
    DC_RES_void();

    if (program)
    {
        dc_try(dn_free(program));
        free(program);
    }

    dc_ret();
}

DCResVoid dn_free(DNode* dn)
{
    DC_RES_void();

    dc_try_fail(dc_dv_free(&dn->data, NULL));

    if (dn->children.cap != 0) dc_try(dc_da_free(&dn->children));

    dc_ret();
}

DC_CLEANUP_FN_DECL(dn_cleanup)
{
    return dn_free((DNode*)_value);
}

DC_DV_FREE_FN_DECL(dn_child_free)
{
    DC_RES_void();

    if (dc_dv_is(*_value, DNodePtr) && dc_dv_as(*_value, DNodePtr) != NULL) dc_try(dn_free(dc_dv_as(*_value, DNodePtr)));

    dc_ret();
}
