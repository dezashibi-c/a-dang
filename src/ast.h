// ***************************************************************************************
//    Project: Dang Compiler -> https://github.com/dezashibi-c/dang
//    File: ast.h
//    Date: 2024-09-13
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

#ifndef DANG_AST_H
#define DANG_AST_H

#include "types.h"

#include "token.h"

typedef enum
{
    DN__UNKNOWN,

    DN_PROGRAM,

    DN__START_STATEMENT,
    DN_LET_STATEMENT,
    DN_RETURN_STATEMENT,
    DN_EXPRESSION_STATEMENT,
    DN_BLOCK_STATEMENT,
    DN__END_STATEMENT,

    DN__START_EXPRESSION,
    DN_IDENTIFIER,
    DN_PREFIX_EXPRESSION,
    DN_INFIX_EXPRESSION,
    DN_IF_EXPRESSION,
    DN_WHILE_EXPRESSION,
    DN_CALL_EXPRESSION,
    DN_INDEX_EXPRESSION,
    DN__END_EXPRESSION,

    DN__START_LITERAL,
    DN_FUNCTION_LITERAL,
    DN_ARRAY_LITERAL,
    DN_HASH_LITERAL,
    DN_MACRO_LITERAL,
    DN_BOOLEAN_LITERAL,
    DN_STRING_LITERAL,
    DN_INTEGER_LITERAL,
    DN__END_LITERAL,

    DN__MAX,
} DNType;

#define __dn_group_is(NODE_TYPE, GROUP) (NODE_TYPE > DN__START_##GROUP && NODE_TYPE < DN__END_##GROUP)
#define dn_group_is_statement(NODE_TYPE) __dn_group_is(NODE_TYPE, STATEMENT)
#define dn_group_is_expression(NODE_TYPE) __dn_group_is(NODE_TYPE, EXPRESSION)
#define dn_group_is_literal(NODE_TYPE) __dn_group_is(NODE_TYPE, LITERAL)
#define dn_type_is_valid(NODE_TYPE)                                                                                            \
    ((NODE_TYPE) == DN_PROGRAM || dn_group_is_expression(NODE_TYPE) || dn_group_is_statement(NODE_TYPE) ||                     \
     dn_group_is_literal(NODE_TYPE))

#define dn_child(NODE, INDEX) (dc_da_get_as(NODE->children, INDEX, DNodePtr))

#define dn_child_dv(NODE, INDEX) (dc_da_get2(NODE->children, INDEX))

#define dn_child_as(NODE, INDEX, TYPE) (dc_da_get_as(((NODE)->children), INDEX, TYPE))

#define dn_child_count(NODE) ((NODE)->children.count)

#define dn_data(NODE) (NODE)->data

#define dn_data_as(NODE, TYPE) dc_dv_as(dn_data(NODE), TYPE)

#define dn_child_data(NODE, INDEX) (dn_child(NODE, INDEX)->data)

#define dn_child_data_as(NODE, INDEX, TYPE) dc_dv_as(dn_child_data(NODE, INDEX), TYPE)

#define dn_child_push(NODE, CHILD) dc_da_push(&NODE->children, dc_dva(DNodePtr, CHILD))

struct DNode
{
    DNType type;
    DCDynVal data;
    DCDynArr children;
};

DCResType(DNode*, ResNode);

string tostr_DNType(DNType dnt);
DCResVoid dang_node_inspect(DNode* dn, string* result);

ResNode dn_new(DNType type, DCDynVal data, bool has_children);
DCResVoid dn_program_free(DNode* program);
DCResVoid dn_free(DNode* dn);

DC_CLEANUP_FN_DECL(dn_cleanup);
DC_DV_FREE_FN_DECL(dn_child_free);

#endif // DANG_AST_H
