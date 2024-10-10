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

typedef enum
{
    DOBJ_INTEGER,
    DOBJ_BOOLEAN,
    DOBJ_NULL,
} DObjType;

typedef struct DObject
{
    bool is_returned;
    DObjType type;
    DCDynVal dv;
} DObject;

#define dobj(OBJ_TYPE, VAL_TYPE, VAL)                                                                                          \
    (DObject)                                                                                                                  \
    {                                                                                                                          \
        .type = OBJ_TYPE, .dv = dc_dv(VAL_TYPE, VAL), .is_returned = false                                                     \
    }

#define dobj_int(INT_VAL) dobj(DOBJ_INTEGER, i64, INT_VAL)
#define dobj_bool(BOOL_VAL) dobj(DOBJ_BOOLEAN, u8, BOOL_VAL)
#define dobj_return(DOBJ)                                                                                                      \
    (DObject)                                                                                                                  \
    {                                                                                                                          \
        .type = (DOBJ).type, .dv = (DOBJ).dv, .is_returned = true                                                              \
    }
#define dobj_return_null()                                                                                                     \
    (DObject)                                                                                                                  \
    {                                                                                                                          \
        .type = DOBJ_NULL, .dv = dc_dv(voidptr, NULL), .is_returned = true                                                     \
    }

#define dobj_null() dobj(DOBJ_NULL, voidptr, NULL)
#define dobj_true() dobj_bool(true)
#define dobj_false() dobj_bool(false)

#define dobj_as_int(DOBJ) (dc_dv_as((DOBJ).dv, i64))
#define dobj_as_bool(DOBJ) (!!dc_dv_as((DOBJ).dv, u8))

#define dobj_is_int(DOBJ) ((DOBJ).type == DOBJ_INTEGER)
#define dobj_is_bool(DOBJ) ((DOBJ).type == DOBJ_BOOLEAN)
#define dobj_is_return(DOBJ) ((DOBJ).is_returned)
#define dobj_is_null(DOBJ) ((DOBJ).type == DOBJ_NULL)

DCResultType(DObject, DObjResult);
DCResultType(DObject*, DObjPResult);

#endif // DANG_OBJECT_H
