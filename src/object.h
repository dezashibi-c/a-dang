// ***************************************************************************************
//    Project: Dang Compiler -> https://github.com/dezashibi-c/dang
//    File: object.h
//    Date: 2024-10-09
//    Author: Navid Dezashibi
//    Contact: navid@dezashibi.com
//    Website: https://dezashibi.com | https://github.com/dezashibi
//    License:
//     Please refer to the LICENSE file, repository or website for more
//     information about the licensing of this work. If you have any questions
//     or concerns, please feel free to contact me at the email address provided
//     above.
// ***************************************************************************************
// *  Description: Objects hold DynVal with extra information (types)
// ***************************************************************************************

#ifndef DANG_OBJECT_H
#define DANG_OBJECT_H

#include "types.h"

// ***************************************************************************************
// * TYPES
// ***************************************************************************************
struct DEnv
{
    DCHashTable memory;
    struct DEnv* outer;

    DCDynArr cleanups;
};

DCResType(DEnv*, ResEnv);

// ***************************************************************************************
// * MACROS
// ***************************************************************************************


#define DECL_DBUILTIN_FUNCTION(NAME) DCDynVal NAME(DCDynValPtr call_obj, DCError* error)

#if 0

#define dobj(OBJ_TYPE, VAL_TYPE, VAL)                                                                                          \
    (DObj)                                                                                                                     \
    {                                                                                                                          \
        .type = OBJ_TYPE, .dv = dc_dv(VAL_TYPE, VAL), .is_returned = false, .env = NULL, .children = (DCDynArr)                \
        {                                                                                                                      \
            0                                                                                                                  \
        }                                                                                                                      \
    }

#define dobj_fn(NODE, DENV)                                                                                                    \
    (DObj)                                                                                                                     \
    {                                                                                                                          \
        .type = DOBJ_FUNCTION, .dv = dc_dv(DNodePtr, NODE), .is_returned = false, .env = (DENV), .children = (DCDynArr)        \
        {                                                                                                                      \
            0                                                                                                                  \
        }                                                                                                                      \
    }

#define dobj_bfn(FN)                                                                                                           \
    (DObj)                                                                                                                     \
    {                                                                                                                          \
        .type = DOBJ_BUILTIN, .dv = dc_dv(DBuiltinFunction, FN), .is_returned = false, .env = NULL, .children = (DCDynArr)     \
        {                                                                                                                      \
            0                                                                                                                  \
        }                                                                                                                      \
    }

#define dobj_get_node(DOBJ) (dc_dv_as((DOBJ).dv, DNodePtr))

#define dobj_int(INT_VAL) dobj(DOBJ_INTEGER, i64, INT_VAL)
#define dobj_string(STR_VAL) dobj(DOBJ_STRING, string, STR_VAL)
#define dobj_bool(BOOL_VAL) dobj(DOBJ_BOOLEAN, u8, BOOL_VAL)

#define dobj_mark_as_return(DOBJ) (DOBJ)->is_returned = true

#define dobj_as_int(DOBJ) (dc_dv_as((DOBJ).dv, i64))
#define dobj_as_bool(DOBJ) (!!dc_dv_as((DOBJ).dv, u8))
#define dobj_as_string(DOBJ) (dc_dv_as((DOBJ).dv, string))

#define dobj_is_int(DOBJ) ((DOBJ).type == DOBJ_INTEGER)
#define dobj_is_string(DOBJ) ((DOBJ).type == DOBJ_STRING)
#define dobj_is_bool(DOBJ) ((DOBJ).type == DOBJ_BOOLEAN)
#define dobj_is_array(DOBJ) ((DOBJ).type == DOBJ_ARRAY)
#define dobj_is_hash(DOBJ) ((DOBJ).type == DOBJ_HASH)
#define dobj_is_return(DOBJ) ((DOBJ)->is_returned)
#define dobj_is_null(DOBJ) ((DOBJ).type == DOBJ_NULL)

#define dobj_child(DOBJ, INDEX) (dc_da_get_as((DOBJ)->children, INDEX, DObjPtr))

#endif

#endif // DANG_OBJECT_H
