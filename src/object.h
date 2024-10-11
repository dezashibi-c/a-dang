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

#include "dcommon/dcommon.h"

// ***************************************************************************************
// * TYPES
// ***************************************************************************************

typedef enum
{
    DOBJ_INTEGER,
    DOBJ_BOOLEAN,
    DOBJ_FUNCTION,
    DOBJ_STRING,
    DOBJ_NULL,
} DObjType;

typedef struct DEnv
{
    DCHashTable store;
    struct DEnv* outer;
} DEnv;

DCResultType(DEnv*, DEnvResult);

typedef struct
{
    DObjType type;
    DCDynVal dv;
    bool is_returned;

    DEnv* env;
    DCDynArr children;
} DObject;

DCResultType(DObject, DObjResult);
DCResultType(DObject*, DObjPResult);

// ***************************************************************************************
// * MACROS
// ***************************************************************************************
#define dobj(OBJ_TYPE, VAL_TYPE, VAL)                                                                                          \
    (DObject)                                                                                                                  \
    {                                                                                                                          \
        .type = OBJ_TYPE, .dv = dc_dv(VAL_TYPE, VAL), .is_returned = false, .env = NULL, .children = (DCDynArr)                \
        {                                                                                                                      \
            0                                                                                                                  \
        }                                                                                                                      \
    }

#define dobj_fn(NODE, DENV)                                                                                                    \
    (DObject)                                                                                                                  \
    {                                                                                                                          \
        .type = DOBJ_FUNCTION, .dv = dc_dv(voidptr, NODE), .is_returned = false, .env = (DENV), .children = (DCDynArr)         \
        {                                                                                                                      \
            0                                                                                                                  \
        }                                                                                                                      \
    }

#define dobj_get_node(DOBJ) ((DNode*)dc_dv_as((DOBJ).dv, voidptr))

#define dobj_int(INT_VAL) dobj(DOBJ_INTEGER, i64, INT_VAL)
#define dobj_string(STR_VAL) dobj(DOBJ_STRING, string, STR_VAL)
#define dobj_bool(BOOL_VAL) dobj(DOBJ_BOOLEAN, u8, BOOL_VAL)

#define dobj_mark_as_return(DOBJ) (DOBJ).is_returned = true

#define dobj_return_null()                                                                                                     \
    (DObject)                                                                                                                  \
    {                                                                                                                          \
        .type = DOBJ_NULL, .dv = dc_dv(voidptr, NULL), .is_returned = true, .env = NULL, .children = (DCDynArr)                \
        {                                                                                                                      \
            0                                                                                                                  \
        }                                                                                                                      \
    }

#define dobj_null() dobj(DOBJ_NULL, voidptr, NULL)
#define dobj_true() dobj_bool(true)
#define dobj_false() dobj_bool(false)

#define dobj_as_int(DOBJ) (dc_dv_as((DOBJ).dv, i64))
#define dobj_as_bool(DOBJ) (!!dc_dv_as((DOBJ).dv, u8))
#define dobj_as_string(DOBJ) (dc_dv_as((DOBJ).dv, string))

#define dobj_is_int(DOBJ) ((DOBJ).type == DOBJ_INTEGER)
#define dobj_is_string(DOBJ) ((DOBJ).type == DOBJ_STRING)
#define dobj_is_bool(DOBJ) ((DOBJ).type == DOBJ_BOOLEAN)
#define dobj_is_return(DOBJ) ((DOBJ).is_returned)
#define dobj_is_null(DOBJ) ((DOBJ).type == DOBJ_NULL)

#define dobj_child(DOBJ, INDEX) ((DObject*)dc_da_get_as((DOBJ)->children, INDEX, voidptr))

#endif // DANG_OBJECT_H
