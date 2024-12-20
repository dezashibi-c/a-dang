// ***************************************************************************************
//    Project: Dang Compiler -> https://github.com/dezashibi-c/dang
//    File: common.h
//    Date: 2024-09-26
//    Author: Navid Dezashibi
//    Contact: navid@dezashibi.com
//    Website: https://dezashibi.com | https://github.com/dezashibi
//    License:
//     Please refer to the LICENSE file, repository or website for more
//     information about the licensing of this work. If you have any questions
//     or concerns, please feel free to contact me at the email address provided
//     above.
// ***************************************************************************************
// *  Description: This is for initializing dcommon lib
// ***************************************************************************************

#ifndef DANG_COMMON_H
#define DANG_COMMON_H

#include "dcommon/dcommon_primitives.h"

// ***************************************************************************************
// * NODES
// *    Nodes are only those which cannot be used as a valid general data type other as a
// *    parser output and evaluation input
// *    All the fields in any "node"s is a Dynamic Value Pointer which points to the actual
// *    dynamic value in the pool (any dynamic array consist of various dynamic value)
// ***************************************************************************************

#define dn_field_as(NODE, NODE_TYPE, FIELD, FIELD_TYPE) dc_dv_as(*dc_dv_as(NODE, NODE_TYPE).FIELD, FIELD_TYPE)

typedef struct
{
    string value;
} DNodeIdentifier;

#define dn_identifier(V)                                                                                                       \
    (DNodeIdentifier)                                                                                                          \
    {                                                                                                                          \
        .value = (V)                                                                                                           \
    }

DCResType(DNodeIdentifier, ResDNodeIdentifier);

typedef struct
{
    string name;
    DCDynValPtr value;
} DNodeLetStatement;

#define dn_let(N, V)                                                                                                           \
    (DNodeLetStatement)                                                                                                        \
    {                                                                                                                          \
        .name = (N), .value = (V)                                                                                              \
    }

DCResType(DNodeLetStatement, ResDNodeLetStatement);

typedef struct
{
    DCDynValPtr ret_val;
} DNodeReturnStatement;

#define dn_return(V)                                                                                                           \
    (DNodeReturnStatement)                                                                                                     \
    {                                                                                                                          \
        .ret_val = (V)                                                                                                         \
    }

DCResType(DNodeReturnStatement, ResDNodeReturnStatement);

typedef struct
{
    string op;
    DCDynValPtr operand;
} DNodePrefixExpression;

#define dn_prefix(OP, OPND)                                                                                                    \
    (DNodePrefixExpression)                                                                                                    \
    {                                                                                                                          \
        .op = (OP), .operand = (OPND)                                                                                          \
    }

DCResType(DNodePrefixExpression, ResDNodePrefixExpression);

typedef struct
{
    string op;
    DCDynValPtr left;
    DCDynValPtr right;
} DNodeInfixExpression;

#define dn_infix(OP, L, R)                                                                                                     \
    (DNodeInfixExpression)                                                                                                     \
    {                                                                                                                          \
        .op = (OP), .left = (L), .right = (R)                                                                                  \
    }

DCResType(DNodeInfixExpression, ResDNodeInfixExpression);

typedef struct
{
    DCDynArrPtr statements;
} DNodeBlockStatement;

#define dn_block(S)                                                                                                            \
    (DNodeBlockStatement)                                                                                                      \
    {                                                                                                                          \
        .statements = (S)                                                                                                      \
    }

DCResType(DNodeBlockStatement, ResDNodeBlockStatement);

typedef struct
{
    DCDynValPtr condition;
    DCDynArrPtr consequence;
    DCDynArrPtr alternative;
} DNodeIfExpression;

#define dn_if(C, CNS, ALT)                                                                                                     \
    (DNodeIfExpression)                                                                                                        \
    {                                                                                                                          \
        .condition = (C), .consequence = (CNS), .alternative = (ALT)                                                           \
    }

DCResType(DNodeIfExpression, ResDNodeIfExpression);

typedef struct
{
    DCDynArrPtr array;
} DNodeArrayLiteral;

#define dn_array(A)                                                                                                            \
    (DNodeArrayLiteral)                                                                                                        \
    {                                                                                                                          \
        .array = (A)                                                                                                           \
    }

DCResType(DNodeArrayLiteral, ResDNodeArrayLiteral);

typedef struct
{
    DCDynArrPtr key_values; // it is a dynamic array of key, values together
} DNodeHashTableLiteral;

#define dn_hash_table(A)                                                                                                       \
    (DNodeHashTableLiteral)                                                                                                    \
    {                                                                                                                          \
        .key_values = (A)                                                                                                      \
    }

DCResType(DNodeHashTableLiteral, ResDNodeHashTableLiteral);

typedef struct
{
    DCDynArrPtr parameters;
    DCDynArrPtr body;
} DNodeFunctionLiteral;

#define dn_function(P, B)                                                                                                      \
    (DNodeFunctionLiteral)                                                                                                     \
    {                                                                                                                          \
        .parameters = (P), .body = (B)                                                                                         \
    }

DCResType(DNodeFunctionLiteral, ResDNodeFunctionLiteral);

typedef struct
{
    DCDynValPtr function;
    DCDynArrPtr arguments;
} DNodeCallExpression;

#define dn_call(F, A)                                                                                                          \
    (DNodeCallExpression)                                                                                                      \
    {                                                                                                                          \
        .function = (F), .arguments = (A)                                                                                      \
    }

DCResType(DNodeCallExpression, ResDNodeCallExpression);

typedef struct
{
    DCDynValPtr operand;
    DCDynValPtr index;
} DNodeIndexExpression;

#define dn_index(O, I)                                                                                                         \
    (DNodeIndexExpression)                                                                                                     \
    {                                                                                                                          \
        .operand = (O), .index = (I)                                                                                           \
    }

DCResType(DNodeIndexExpression, ResDNodeIndexExpression);

typedef struct
{
    DCDynArrPtr statements;
} DNodeProgram;

#define dn_program(S)                                                                                                          \
    (DNodeProgram)                                                                                                             \
    {                                                                                                                          \
        .statements = (S)                                                                                                      \
    }

DCResType(DNodeProgram, ResDNodeProgram);

// ***************************************************************************************
// * OBJECTS
// *    Objects are returned from evaluator, they are some extended types for dynamic value
// *    which will be needed from evaluator state.
// ***************************************************************************************

typedef struct
{
    DCDynValPtr ret_val;
} DoReturn;

#define do_return(V)                                                                                                           \
    (DoReturn)                                                                                                                 \
    {                                                                                                                          \
        .ret_val = (V)                                                                                                         \
    }

// ***************************************************************************************
// * FORWARD DECLARATIONS
// ***************************************************************************************
typedef struct DEvaluator DEvaluator;
typedef struct DEnv DEnv;
typedef DEnv* DEnvPtr;

/**
 * Function pointer type for all dang builtin functions
 *
 * NOTE: As DCDynVal is forward declared at this stage, I couldn't use DCRes
 *       So simply I've used DCDynVal for output and DCError as a pointer
 *       Now if it returns an error (error is not null), I can continue the flow
 *       The normal way with redirecting the error
 */
typedef DCDynVal (*DBuiltinFunction)(DEvaluator* de, DCDynValPtr call_obj, DCError* error);

#define DC_DV_EXTRA_TYPES                                                                                                      \
    dc_dvt(DEnvPtr), dc_dvt(DBuiltinFunction), dc_dvt(DNodeProgram), dc_dvt(DNodeLetStatement), dc_dvt(DNodeReturnStatement),  \
        dc_dvt(DNodeBlockStatement), dc_dvt(DNodeIdentifier), dc_dvt(DNodePrefixExpression), dc_dvt(DNodeInfixExpression),     \
        dc_dvt(DNodeIfExpression), dc_dvt(DNodeArrayLiteral), dc_dvt(DNodeHashTableLiteral), dc_dvt(DNodeFunctionLiteral),     \
        dc_dvt(DNodeCallExpression), dc_dvt(DNodeIndexExpression), dc_dvt(DoReturn),

#define DC_DV_EXTRA_UNION_FIELDS                                                                                               \
    dc_dvf_decl(DEnvPtr);                                                                                                      \
    dc_dvf_decl(DBuiltinFunction);                                                                                             \
    /* DNodeProgram is the first node type*/                                                                                   \
    dc_dvf_decl(DNodeProgram);                                                                                                 \
    dc_dvf_decl(DNodeLetStatement);                                                                                            \
    dc_dvf_decl(DNodeReturnStatement);                                                                                         \
    dc_dvf_decl(DNodeBlockStatement);                                                                                          \
    /* all types after this line are expressions */                                                                            \
    dc_dvf_decl(DNodeIdentifier);                                                                                              \
    dc_dvf_decl(DNodePrefixExpression);                                                                                        \
    dc_dvf_decl(DNodeInfixExpression);                                                                                         \
    dc_dvf_decl(DNodeIfExpression);                                                                                            \
    dc_dvf_decl(DNodeArrayLiteral);                                                                                            \
    dc_dvf_decl(DNodeHashTableLiteral);                                                                                        \
    dc_dvf_decl(DNodeFunctionLiteral);                                                                                         \
    dc_dvf_decl(DNodeCallExpression);                                                                                          \
    dc_dvf_decl(DNodeIndexExpression);                                                                                         \
    /* DNodeIndexExpression is the last node type */                                                                           \
    dc_dvf_decl(DoReturn);

#define DC_DV_EXTRA_FIELDS DEnvPtr env;

#include "dcommon/dcommon.h"

// ***************************************************************************************
// * CONFIGS AND GENERAL MACROS
// ***************************************************************************************

typedef enum
{
    DANG_MAIN_BATCH,
    DANG_TEMP_BATCH,

    DANG_MAX_BATCH,
} DangCleanupBatch;

// ***************************************************************************************
// * FUNCTION DECLARATIONS
// ***************************************************************************************

void configure(b1 init_pool, string log_file, b1 append_logs);

string dv_type_tostr(DCDynValPtr dv);

#endif // DANG_COMMON_H
