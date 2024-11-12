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

#include "common.h"
#include "token.h"

#define node_is_quote(NODE)                                                                                                    \
    ((NODE).type == dc_dvt(DNodeCallExpression) &&                                                                             \
     dc_dv_as((NODE), DNodeCallExpression).function->type == dc_dvt(DNodeIdentifier) &&                                        \
     strcmp(dc_dv_as(*dc_dv_as((NODE), DNodeCallExpression).function, DNodeIdentifier).value, QUOTE) == 0)

#define node_is_unquote(NODE)                                                                                                  \
    ((NODE).type == dc_dvt(DNodeCallExpression) &&                                                                             \
     dc_dv_as((NODE), DNodeCallExpression).function->type == dc_dvt(DNodeIdentifier) &&                                        \
     strcmp(dc_dv_as(*dc_dv_as((NODE), DNodeCallExpression).function, DNodeIdentifier).value, UNQUOTE) == 0)

DCResVoid dang_program_inspect(DNodeProgram* program, string* result);

DCResVoid dang_node_inspect(DCDynValPtr dn, string* result);

DCRes dang_node_copy(DCDynValPtr dn, DCDynArrPtr pool);

DC_DV_FREE_FN_DECL(dn_child_free);

#endif // DANG_AST_H
