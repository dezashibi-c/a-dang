// ***************************************************************************************
//    Project: Dang Compiler -> https://github.com/dezashibi-c/dang
//    File: types.h
//    Date: 2024-10-14
//    Author: Navid Dezashibi
//    Contact: navid@dezashibi.com
//    Website: https://dezashibi.com | https://github.com/dezashibi
//    License:
//     Please refer to the LICENSE file, repository or website for more
//     information about the licensing of this work. If you have any questions
//     or concerns, please feel free to contact me at the email address provided
//     above.
// ***************************************************************************************
// *  Description: Declaration of all types and structs used in `Dang`
// ***************************************************************************************

#ifndef DANG_TYPES_H
#define DANG_TYPES_H

// todo:: delete me

#include "dcommon/dcommon_primitives.h"

// ***************************************************************************************
// * NODES
// *    Nodes are only those which cannot be used as a valid general data type other as a
// *    parser output and evaluation input
// *    All the fields in any "node"s is a Dynamic Value Pointer which points to the actual
// *    dynamic value in the pool (any dynamic array consist of various dynamic value)
// ***************************************************************************************

typedef struct
{
    string value;
} DNodeIdentifier;


typedef struct
{
    string name;
    DCDynValPtr value;
} DNodeLetStatement;


typedef struct
{
    DCDynValPtr ret_val;
} DNodeReturnStatement;


typedef struct
{
    string op;
    DCDynValPtr operand;
} DNodePrefixExpression;


typedef struct
{
    string op;
    DCDynValPtr left;
    DCDynValPtr right;
} DNodeInfixExpression;


typedef struct
{
    DCDynArrPtr statements;
} DNodeBlockStatement;


typedef struct
{
    DCDynValPtr condition;
    DNodeBlockStatement consequence;
    DNodeBlockStatement alternative;
} DNodeIfExpression;


typedef struct
{
    DCDynArrPtr array;
} DNodeArrayLiteral;


typedef struct
{
    DCHashTablePtr ht;
} DNodeHashTableLiteral;


typedef struct
{
    DCDynArrPtr parameters;
    DNodeBlockStatement body;
} DNodeFunctionLiteral;


typedef struct
{
    DCDynValPtr expression;
    DCDynArrPtr arguments;
} DNodeCallExpression;


typedef struct
{
    DCDynValPtr operand;
    DCDynValPtr index;
} DNodeIndexExpression;


// ***************************************************************************************
// *
// ***************************************************************************************
typedef struct DEnv DEnv;

/**
 * Function pointer type for all dang builtin functions
 *
 * NOTE: As DCDynVal is forward declared at this stage, I couldn't use DCRes
 *       So simply I've used DCDynVal for output and DCError as a pointer
 *       Now if it returns an error (error is not null), I can continue the flow
 *       The normal way with redirecting the error
 */
typedef DCDynVal (*DBuiltinFunction)(DCDynValPtr call_obj, DCError* error);

#define DC_DV_EXTRA_TYPES                                                                                                      \
    dc_dvt(DBuiltinFunction), dc_dvt(DNodeIdentifier), dc_dvt(DNodeLetStatement), dc_dvt(DNodeReturnStatement),                \
        dc_dvt(DNodePrefixExpression), dc_dvt(DNodeInfixExpression), dc_dvt(DNodeBlockStatement), dc_dvt(DNodeIfExpression),   \
        dc_dvt(DNodeArrayLiteral), dc_dvt(DNodeHashTableLiteral), dc_dvt(DNodeFunctionLiteral), dc_dvt(DNodeCallExpression),   \
        dc_dvt(DNodeIndexExpression),

#define DC_DV_EXTRA_UNION_FIELDS                                                                                               \
    dc_dvf_decl(DBuiltinFunction);                                                                                             \
    dc_dvf_decl(DNodeIdentifier);                                                                                              \
    dc_dvf_decl(DNodeLetStatement);                                                                                            \
    dc_dvf_decl(DNodeReturnStatement);                                                                                         \
    dc_dvf_decl(DNodePrefixExpression);                                                                                        \
    dc_dvf_decl(DNodeInfixExpression);                                                                                         \
    dc_dvf_decl(DNodeBlockStatement);                                                                                          \
    dc_dvf_decl(DNodeIfExpression);                                                                                            \
    dc_dvf_decl(DNodeArrayLiteral);                                                                                            \
    dc_dvt_decl(DNodeHashTableLiteral);                                                                                        \
    dc_dvf_decl(DNodeFunctionLiteral);                                                                                         \
    dc_dvf_decl(DNodeCallExpression);                                                                                          \
    dc_dvf_decl(DNodeIndexExpression);

#define DC_DV_EXTRA_FIELDS                                                                                                     \
    b1 quoted;                                                                                                                 \
    DEnv* env;

#include "dcommon/dcommon.h"

#define MACRO_PACK "quote"
#define MACRO_UNPACK "unquote"

#endif // DANG_TYPES_H
