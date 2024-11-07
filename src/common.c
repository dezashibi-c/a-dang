// ***************************************************************************************
//    Project: Dang Compiler -> https://github.com/dezashibi-c/dang
//    File: common.c
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

#define DCOMMON_IMPL

#include "common.h"

void configure(b1 init_pool, string log_file, b1 append_logs)
{
    if (log_file) dc_error_logs_init(log_file, append_logs);

    if (init_pool) dc_cleanup_pool_init2(DANG_MAX_BATCH, 10);
}

string dv_type_tostr(DCDynValType type)
{
    switch (type)
    {
        case dc_dvt(i64):
            return "integer";

        case dc_dvt(string):
            return "string";

        case dc_dvt(b1):
            return "boolean";

        case dc_dvt(DCDynArrPtr):
            return "array";

        case dc_dvt(DCHashTablePtr):
            return "hash table";

        case dc_dvt(DBuiltinFunction):
            return "builtin function";

        case dc_dvt(DNodeIdentifier):
            return "identifier node";

        case dc_dvt(DNodeLetStatement):
            return "let statement node";

        case dc_dvt(DNodeReturnStatement):
            return "return statement node";

        case dc_dvt(DNodePrefixExpression):
            return "prefix expression node";

        case dc_dvt(DNodeInfixExpression):
            return "infix expression node";

        case dc_dvt(DNodeBlockStatement):
            return "block statement node";

        case dc_dvt(DNodeIfExpression):
            return "if expression node";

        case dc_dvt(DNodeArrayLiteral):
            return "array literal node";

        case dc_dvt(DNodeHashTableLiteral):
            return "hash table node";

        case dc_dvt(DNodeFunctionLiteral):
            return "function literal node";

        case dc_dvt(DNodeCallExpression):
            return "call expression node";

        case dc_dvt(DNodeIndexExpression):
            return "index expression node";

        case dc_dvt(DEnvPtr):
            return "environment pointer";

        default:
            break;
    };

    return "(unknown or unimplemented type)";
}
