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

void dn_string_init(DNode* dn)
{
    dc_action_on(!dn, exit(-1), "%s", "got NULL node");

    dc_action_on(!dn_type_is_valid(dn->type), exit(-1), "got invalid node type: %s", tostr_DNType(dn->type));

    if (dn->text != NULL) return;

    switch (dn->type)
    {
        case DN_PROGRAM:
            dc_da_for(dn->children)
            {
                dn_string_init(dn_child(dn, _idx));
                dc_sappend(&dn->text, "%s\n", dn_child(dn, _idx)->text);
            }

            break;

        case DN_LET_STATEMENT:
        {
            dn_string_init(dn_child(dn, 0));
            string value = "";
            if (dn_child_count(dn) > 1)
            {
                dn_string_init(dn_child(dn, 1));
                value = dn_child(dn, 1)->text;
            }

            dc_sprintf(&dn->text, DCPRIsv " %s %s", dc_sv_fmt(dn_text(dn)), dn_child(dn, 0)->text, value);

            break;
        }

        case DN_RETURN_STATEMENT:
        {
            string value = NULL;
            if (dn_child_count(dn) > 0)
            {
                dn_string_init(dn_child(dn, 0));
                value = dn_child(dn, 0)->text;
            }
            dc_sprintf(&dn->text, DCPRIsv " %s", dc_sv_fmt(dn_text(dn)), (value == NULL ? "" : value));
            break;
        }

        case DN_EXPRESSION_STATEMENT:
        {
            string value = NULL;
            if (dn_child_count(dn) > 0)
            {
                dn_string_init(dn_child(dn, 0));
                value = dn_child(dn, 0)->text;
            }
            dc_sprintf(&dn->text, "%s", (value == NULL ? "" : value));
            break;
        }

        case DN_PREFIX_EXPRESSION:
            dn_string_init(dn_child(dn, 0));
            dc_sprintf(&dn->text, "(" DCPRIsv "%s)", dc_sv_fmt(dn_text(dn)), dn_child(dn, 0)->text);
            break;

        case DN_INFIX_EXPRESSION:
            dn_string_init(dn_child(dn, 0));
            dn_string_init(dn_child(dn, 1));
            dc_sprintf(&dn->text, "(%s " DCPRIsv " %s)", dn_child(dn, 0)->text, dc_sv_fmt(dn_text(dn)), dn_child(dn, 1)->text);
            break;

        case DN_IF_EXPRESSION:
        {
            dn_string_init(dn_child(dn, 0));
            dn_string_init(dn_child(dn, 1));
            string value = NULL;
            if (dn_child_count(dn) > 2)
            {
                dn_string_init(dn_child(dn, 2));
                value = dn_child(dn, 2)->text;
            }

            dc_sprintf(&dn->text, DCPRIsv " %s %s%s%s", dc_sv_fmt(dn_text(dn)), dn_child(dn, 0)->text, dn_child(dn, 1)->text,
                       (value ? " else " : ""), (value ? value : ""));

            break;
        }

        case DN_BLOCK_STATEMENT:
            dc_sprintf(&dn->text, "%s", "{ ");
            dc_da_for(dn->children)
            {
                dn_string_init(dn_child(dn, _idx));
                dc_sappend(&dn->text, "%s; ", dn_child(dn, _idx)->text);
            }
            dc_sappend(&dn->text, "%s", "}");

            break;

        case DN_FUNCTION_LITERAL:
            dc_sprintf(&dn->text, DCPRIsv " %s", dc_sv_fmt(dn_text(dn)), "(");
            dc_da_for(dn->children)
            {
                dn_string_init(dn_child(dn, _idx));

                if (_idx == dn_child_count(dn) - 1) break;

                dc_sappend(&dn->text, "%s", dn_child(dn, _idx)->text);

                if (_idx < dn_child_count(dn) - 2) dc_sappend(&dn->text, "%s", ", ");
            }

            dc_sappend(&dn->text, "%s %s", ")", dn_child(dn, dn_child_count(dn) - 1)->text);

            break;

        case DN_CALL_EXPRESSION:
            dc_da_for(dn->children)
            {
                dn_string_init(dn_child(dn, _idx));

                if (_idx == 0)
                {
                    dc_sappend(&dn->text, "%s(", dn_child(dn, _idx)->text);
                    continue;
                }

                dc_sappend(&dn->text, "%s", dn_child(dn, _idx)->text);

                if (_idx < dn_child_count(dn) - 1) dc_sappend(&dn->text, "%s", ", ");
            }

            dc_sappend(&dn->text, "%s", ")");

            break;

        default:
            dc_sprintf(&dn->text, DCPRIsv, dc_sv_fmt(dn_text(dn)));
            break;


            // DN_IDENTIFIER,
            // DN_WHILE_EXPRESSION,
            // DN_INDEX_EXPRESSION,

            // DN_ARRAY_LITERAL,
            // DN_HASH_LITERAL,
            // DN_MACRO_LITERAL,
            // DN_BOOLEAN_LITERAL,
            // DN_INTEGER_LITERAL,
    };
}

ResultDNode dn_new(DNType type, DToken* token, bool has_children)
{
    DC_RES2(ResultDNode);

    DNode* node = malloc(sizeof(DNode));
    if (node == NULL)
    {
        dc_dbg_log("DNode Memory allocation failed");

        dc_res_ret_e(2, "DNode Memory allocation failed");
    }

    node->type = type;
    node->token = token;
    node->text = NULL;

    node->children = (DCDynArr){0};

    if (has_children) dc_try_fail_temp(DCResultVoid, dc_da_init(&node->children, dn_child_free));

    dc_res_ret_ok(node);
}

DCResultVoid dn_program_free(DNode* program)
{
    DC_RES_void();

    if (program)
    {
        dc_try(dn_free(program));
        free(program);
    }

    dc_res_ret();
}

DCResultVoid dn_free(DNode* dn)
{
    DC_RES_void();

    if (dn->text != NULL) free(dn->text);

    if (dn->children.cap != 0) dc_try(dc_da_free(&dn->children));

    dc_res_ret();
}

DC_CLEANUP_FN_DECL(dn_cleanup)
{
    return dn_free((DNode*)_value);
}

DC_DV_FREE_FN_DECL(dn_child_free)
{
    DC_RES_void();

    if (dc_dv_is(*_value, voidptr) && dc_dv_as(*_value, voidptr) != NULL) dc_try(dn_free((DNode*)dc_dv_as(*_value, voidptr)));

    dc_res_ret();
}
