// ***************************************************************************************
//    Project: Dang Compiler -> https://github.com/dezashibi-c/dang
//    File: evaluator.h
//    Date: 2024-10-08
//    Author: Navid Dezashibi
//    Contact: navid@dezashibi.com
//    Website: https://dezashibi.com | https://github.com/dezashibi
//    License:
//     Please refer to the LICENSE file, repository or website for more
//     information about the licensing of this work. If you have any questions
//     or concerns, please feel free to contact me at the email address provided
//     above.
// ***************************************************************************************
// *  Description: Evaluator header file
// ***************************************************************************************

#ifndef DANG_EVAL_H
#define DANG_EVAL_H

#include "ast.h"

// ***************************************************************************************
// * TYPES
// ***************************************************************************************

typedef enum
{
    DOBJ_NUMBER,
    DOBJ_STRING,
    DOBJ_BOOLEAN,
    DOBJ_QUOTE,
    DOBJ_MACRO,
    DOBJ_FUNCTION,
    DOBJ_BUILTIN_FUNCTION,
    DOBJ_ARRAY,
    DOBJ_HASH_TABLE,
    DOBJ_NULL,

    DOBJ_UNKNOWN,
} DObjType;

struct DEnv
{
    DCHashTable memory;
    struct DEnv* outer;

    DCDynArr cleanups;
};

DCResType(DEnv*, ResEnv);

typedef DCResVoid (*DNodeModifierFn)(DNodePtr dn, DEnv* de, DEnv* main_de);

// ***************************************************************************************
// * MACROS
// ***************************************************************************************
#define dobj_def(TYPE, VALUE, ENV)                                                                                             \
    (DCDynVal)                                                                                                                 \
    {                                                                                                                          \
        .type = dc_dvt(TYPE), .value.dc_dvf(TYPE) = VALUE, .allocated = false, .env = ENV, .is_returned = false,               \
    }

#define dobj_defa(TYPE, VALUE, ENV)                                                                                            \
    (DCDynVal)                                                                                                                 \
    {                                                                                                                          \
        .type = dc_dvt(TYPE), .value.dc_dvf(TYPE) = VALUE, .allocated = true, .env = ENV, .is_returned = false,                \
    }

#define dobj_int(NUM) dobj_def(i64, (NUM), NULL)
#define dobj_string(STR) dobj_def(string, (STR), NULL)

#define dobj_as_arr(DOBJ) (*dc_dv_as((DOBJ), DCDynArrPtr))
#define dobj_as_int(DOBJ) dc_dv_as((DOBJ), i64)

#define dobj_mark_returned(DOBJ) (DOBJ).is_returned = true

#define DECL_DBUILTIN_FUNCTION(NAME) DCDynVal NAME(DCDynValPtr call_obj, DCError* error)

#define BUILTIN_FN_GET_ARGS DCDynArr _args = dobj_as_arr(*call_obj);

#define BUILTIN_FN_GET_ARGS_VALIDATE(FN_NAME, NUM)                                                                             \
    DCDynArr _args = dobj_as_arr(*call_obj);                                                                                   \
    if (_args.count != (NUM))                                                                                                  \
    {                                                                                                                          \
        dc_error_inita(*error, -1, "invalid number of argument passed to '" FN_NAME "', expected=%d, got=" dc_fmt(usize),      \
                       (NUM), _args.count);                                                                                    \
        return dc_dv_nullptr();                                                                                                \
    }

#define BUILTIN_FN_GET_ARG_NO(NUM, TYPE, ERR_MSG)                                                                              \
    DCDynVal arg##NUM = dc_da_get2(_args, NUM);                                                                                \
    if (dobj_get_type(&arg0) != TYPE)                                                                                          \
    {                                                                                                                          \
        dc_error_inita(*error, -1, ERR_MSG ", got arg of type '%s'", tostr_DObjType(&arg##NUM));                               \
        return dc_dv_nullptr();                                                                                                \
    }

#define dobj_resolve_if_ref(NEW_DOBJ, DOBJ)                                                                                    \
    NEW_DOBJ = ((DOBJ).type == dc_dvt(DCDynValPtr) ? dc_dv_as((DOBJ), DCDynValPtr) : &(DOBJ));                                 \
    do                                                                                                                         \
    {                                                                                                                          \
        NEW_DOBJ->is_returned = (DOBJ).is_returned;                                                                            \
        NEW_DOBJ->env = (DOBJ).env;                                                                                            \
    } while (0)

#define REGISTER_CLEANUP2(DOBJ, FAILURE_ACTIONS)                                                                               \
    do                                                                                                                         \
    {                                                                                                                          \
        dc_try_or_fail_with3(DCResVoid, res, dc_da_push(&main_de->cleanups, DOBJ), FAILURE_ACTIONS);                           \
    } while (0)

#define REGISTER_CLEANUP(TYPE, VALUE, FAILURE_ACTIONS)                                                                         \
    do                                                                                                                         \
    {                                                                                                                          \
        dc_try_or_fail_with3(DCResVoid, res, dc_da_push(&main_de->cleanups, dc_dva(TYPE, VALUE)), FAILURE_ACTIONS);            \
    } while (0)

#define DECL_DNODE_MODIFIER_FN(NAME) DCResVoid NAME(DNodePtr dn, DEnv* de, DEnv* main_de)

// ***************************************************************************************
// * FUNCTION DECLARATIONS
// ***************************************************************************************

DCRes dang_eval(DNode* program, DEnv* main_de);

DCResVoid dang_obj_free(DCDynValPtr dobj);

DObjType dobj_get_type(DCDynValPtr dobj);
DCResString dobj_tostr(DCDynValPtr dobj);
void dobj_print(DCDynValPtr obj);

ResEnv dang_env_new();
DCResVoid dang_env_free(DEnv* de);
DCRes dang_env_get(DEnv* de, string name);
DCRes dang_env_set(DEnv* de, string name, DCDynValPtr value, b1 update_only);

string tostr_DObjType(DCDynValPtr dobj);

DCResVoid dn_modify(DNodePtr dn, DEnv* de, DEnv* main_de, DNodeModifierFn modifier);

#endif // DANG_EVAL_H
