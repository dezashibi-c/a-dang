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
#include "parser.h"

// ***************************************************************************************
// * TYPES
// ***************************************************************************************

struct DEnv
{
    DCHashTable memory;
    struct DEnv* outer;
};

DCResType(DEnv*, ResEnv);

struct DEvaluator
{
    DEnv main_env;

    DParser parser;

    DCDynArr pool;
    DCDynArr errors;

    DCDynVal nullptr;
    DCDynVal bool_true;
    DCDynVal bool_false;
};

typedef struct
{
    DCDynVal result;
    string inspect;
} Evaluated;

DCResType(Evaluated, ResEvaluated);

typedef DCRes (*DNodeModifierFn)(DEvaluator* de, DCRes dn, DEnv* env);

// ***************************************************************************************
// * MACROS
// ***************************************************************************************

#define DO_STRING dc_dvt(string)
#define DO_INTEGER dc_dvt(i64)
#define DO_BOOLEAN dc_dvt(b1)
#define DO_ARRAY dc_dvt(DCDynArrPtr)
#define DO_HASH_TABLE dc_dvt(DCHashTablePtr)
#define DO_FUNCTION dc_dvt(DNodeFunctionLiteral)
#define DO_BUILTIN_FUNCTION dc_dvt(DBuiltinFunction)
#define DO_RETURN dc_dvt(DoReturn)

#define dang_evaluated(RES, INSPECT)                                                                                           \
    (Evaluated)                                                                                                                \
    {                                                                                                                          \
        .result = (RES), .inspect = (INSPECT)                                                                                  \
    }

#define do_def(TYPE, VALUE, ENV)                                                                                               \
    (DCDynVal)                                                                                                                 \
    {                                                                                                                          \
        .type = dc_dvt(TYPE), .value.dc_dvf(TYPE) = VALUE, .allocated = false, .env = ENV                                      \
    }

#define do_defa(TYPE, VALUE, ENV)                                                                                              \
    (DCDynVal)                                                                                                                 \
    {                                                                                                                          \
        .type = dc_dvt(TYPE), .value.dc_dvf(TYPE) = VALUE, .allocated = true, .env = ENV                                       \
    }

#define do_int(NUM) do_def(i64, (NUM), NULL)
#define do_string(STR) do_def(string, (STR), NULL)

#define do_as_arr(DO) (*dc_dv_as((DO), DCDynArrPtr))
#define do_as_int(DO) dc_dv_as((DO), i64)
#define do_as_string(DO) dc_dv_as((DO), string)

#define do_is_int(DO) (dc_dv_is((DO), i64))
#define do_is_string(DO) (dc_dv_is((DO), string))

#define if_dv_is_DoReturn_return_unwrapped()                                                                                   \
    if (dc_unwrap().type == dc_dvt(DoReturn)) dc_ret_ok(*(dc_dv_as(dc_unwrap(), DoReturn).ret_val))

#define DECL_DBUILTIN_FUNCTION(NAME) DCDynVal NAME(DEvaluator* de, DCDynValPtr call_obj, DCError* error)

#define BUILTIN_FN_GET_ARGS DCDynArr _args = do_as_arr(*call_obj);

#define BUILTIN_FN_GET_ARGS_VALIDATE(FN_NAME, NUM)                                                                             \
    DCDynArr _args = do_as_arr(*call_obj);                                                                                     \
    if (_args.count != (NUM))                                                                                                  \
    {                                                                                                                          \
        dc_error_inita(*error, -1, "invalid number of argument passed to '" FN_NAME "', expected=%d, got=" dc_fmt(usize),      \
                       (NUM), _args.count);                                                                                    \
        return dc_dv_nullptr();                                                                                                \
    }

#define BUILTIN_FN_GET_ARG_NO(NUM, TYPE, ERR_MSG)                                                                              \
    DCDynVal arg##NUM = dc_da_get2(_args, NUM);                                                                                \
    if (arg0.type != TYPE)                                                                                                     \
    {                                                                                                                          \
        dc_error_inita(*error, -1, ERR_MSG ", got arg of type '%s'", dv_type_tostr(&arg##NUM));                                \
        return dc_dv_nullptr();                                                                                                \
    }

#define DECL_DNODE_MODIFIER_FN(NAME) ResNode NAME(DNodePtr dn, DEnv* de, DEnv* main_de)

// ***************************************************************************************
// * FUNCTION DECLARATIONS
// ***************************************************************************************

DCResVoid de_init(DEvaluator* de);
DCResVoid de_free(DEvaluator* de);

ResEvaluated dang_eval(DEvaluator* de, const string source, b1 inspect);

DCResString do_tostr(DCDynValPtr obj);
void do_print(DCDynValPtr obj);

DCResVoid dang_env_init(DEnvPtr env);
ResEnv dang_env_new();
DCResVoid dang_env_free(DEnv* de);
DCRes dang_env_get(DEnv* env, string name);
DCRes dang_env_set(DEnv* env, string name, DCDynValPtr value, b1 update_only);

string tostr_DoType(DCDynValPtr obj);

DCRes dn_modify(DCDynValPtr dn, DEnv* de, DEnv* main_de, DNodeModifierFn modifier);

#endif // DANG_EVAL_H
