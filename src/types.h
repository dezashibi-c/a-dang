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

#include "dcommon/dcommon_primitives.h"

typedef struct DNode DNode;
typedef struct DNode* DNodePtr;
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

#define DC_DV_EXTRA_TYPES dc_dvt(DNodePtr), dc_dvt(DBuiltinFunction),
#define DC_DV_EXTRA_UNION_FIELDS                                                                                               \
    dc_dvf_decl(DNodePtr);                                                                                                     \
    dc_dvf_decl(DBuiltinFunction);

#define DC_DV_EXTRA_FIELDS                                                                                                     \
    b1 is_returned;                                                                                                            \
    DEnv* env;

#include "dcommon/dcommon.h"

#endif // DANG_TYPES_H
