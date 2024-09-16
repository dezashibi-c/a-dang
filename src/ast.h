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
    ((NODE_TYPE) > DN__UNKNOWN && (NODE_TYPE) < DN__MAX &&                     \
     dnode_group_is_expression(NODE_TYPE) &&                                   \
     dnode_group_is_statement(NODE_TYPE) && dnode_group_is_literal(NODE_TYPE))

#define __NODE_TYPE_DEFAULT_FIELDS                                             \
    DangNodeType type;                                                         \
    Token* token;

typedef struct
{
    __NODE_TYPE_DEFAULT_FIELDS
} DNode;

typedef struct
{
    __NODE_TYPE_DEFAULT_FIELDS

    DCDynArr statements;
} DNodeProgram;

typedef struct
{
    __NODE_TYPE_DEFAULT_FIELDS

    DNode name;
    DNode value;
} DNodeLetStatement;

typedef struct
{
    __NODE_TYPE_DEFAULT_FIELDS

    DNode return_value_expr;
} DNodeReturnStatement;

typedef struct
{
    __NODE_TYPE_DEFAULT_FIELDS

    DNode expression;
} DNodeExpressionStatement;

typedef struct
{
    __NODE_TYPE_DEFAULT_FIELDS

    DCDynValue value;
} DNodeLiteral;

typedef struct
{
    __NODE_TYPE_DEFAULT_FIELDS

    DNode right;
} DNodePrefixExpression;

typedef struct
{
    __NODE_TYPE_DEFAULT_FIELDS

    DNode left;
    DNode right;
} DNodeInfixExpression;

typedef struct
{
    __NODE_TYPE_DEFAULT_FIELDS

    DCDynArr statements;
} DNodeBlockStatement;

typedef struct
{
    __NODE_TYPE_DEFAULT_FIELDS

    DNode condition;
    DNodeBlockStatement consequence;
    DNodeBlockStatement alternative;
} DNodeIfExpression;

typedef struct
{
    __NODE_TYPE_DEFAULT_FIELDS

    DNode condition;
    DNodeBlockStatement body;
} DNodeWhileExpression;

typedef struct
{
    __NODE_TYPE_DEFAULT_FIELDS

    DCDynArr identifiers;
    DNodeBlockStatement body;
} DNodeFunctionLiteral;

typedef struct
{
    __NODE_TYPE_DEFAULT_FIELDS

    DNode function;
    DCDynArr expressions;
} DNodeCallExpression;

typedef struct
{
    __NODE_TYPE_DEFAULT_FIELDS

    DCDynArr expressions;
} DNodeArrayLiteral;

typedef struct
{
    __NODE_TYPE_DEFAULT_FIELDS

    DCHashTable expression_pairs;
} DNodeHashLiteral;

typedef struct
{
    __NODE_TYPE_DEFAULT_FIELDS

    DNode left;
    DNode index;
} DNodeIndexExpression;

typedef struct
{
    __NODE_TYPE_DEFAULT_FIELDS

    DCDynArr identifiers;
    DNodeBlockStatement body;
} DNodeMacroLiteral;

string tostr_DangNodeType(DangNodeType dnt);
DCStringView* dnode_get_token_text(DNode* dn);

#endif // DANG_AST_H
