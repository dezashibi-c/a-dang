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

#include "dcommon/dcommon.h"

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
} DangNodeType;

#define __dnode_group_is(NODE_TYPE, GROUP)                                     \
    (NODE_TYPE > DN__START_##GROUP && NODE_TYPE < DN__END_##GROUP)
#define dnode_group_is_statement(NODE_TYPE)                                    \
    __dnode_group_is(NODE_TYPE, STATEMENT)
#define dnode_group_is_expression(NODE_TYPE)                                   \
    __dnode_group_is(NODE_TYPE, EXPRESSION)
#define dnode_group_is_literal(NODE_TYPE) __dnode_group_is(NODE_TYPE, LITERAL)
#define dnode_type_is_valid(NODE_TYPE)                                         \
    ((NODE_TYPE) == DN_PROGRAM || dnode_group_is_expression(NODE_TYPE) ||      \
     dnode_group_is_statement(NODE_TYPE) || dnode_group_is_literal(NODE_TYPE))

#define dn_child(NODE, INDEX)                                                  \
    ((DNode*)dc_da_get_as(&((NODE)->children), INDEX, voidptr))

#define dn_child_count(NODE) ((NODE)->children.count)

#define dn_text(NODE) (NODE)->token->text

typedef struct
{
    DangNodeType type;
    Token* token;
    string text;
    DCDynArr children;
} DNode;

string tostr_DangNodeType(DangNodeType dnt);
void dnode_string_init(DNode* dn);

DNode* dnode_create(DangNodeType type, Token* token, bool has_children);
void dnode_free(DNode* dn);
void dnode_child_free(DCDynValue* child);

#endif // DANG_AST_H
