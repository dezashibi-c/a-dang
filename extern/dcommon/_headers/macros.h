// ***************************************************************************************
//    Project: dcommon -> https://github.com/dezashibi-c/dcommon
//    File: macros.h
//    Date: 2024-09-08
//    Author: Navid Dezashibi
//    Contact: navid@dezashibi.com
//    Website: https://dezashibi.com | https://github.com/dezashibi
//    License:
//     Please refer to the LICENSE file, repository or website for more
//     information about the licensing of this work. If you have any questions
//     or concerns, please feel free to contact me at the email address provided
//     above.
// ***************************************************************************************
// *  Description: private header file for definition of common macros
// *               DO NOT USE THIS DIRECTLY
// ***************************************************************************************

#ifndef DC_MACROS_H
#define DC_MACROS_H

#ifndef __DC_BYPASS_PRIVATE_PROTECTION
#error                                                                         \
    "You cannot use this header (macros.h) directly, please consider including dcommon.h"
#endif

#define dc_halt_when(CONDITION, FAILURE_ACTION, ...)                           \
    do                                                                         \
    {                                                                          \
        if (CONDITION)                                                         \
        {                                                                      \
            fprintf(dc_error_logs ? dc_error_logs : stderr, __VA_ARGS__);      \
            fprintf(dc_error_logs ? dc_error_logs : stderr, "\n");             \
            if (dc_error_mode == DC_ERR_MODE_ABORT)                            \
            {                                                                  \
                abort();                                                       \
            }                                                                  \
            else                                                               \
            {                                                                  \
                FAILURE_ACTION;                                                \
            }                                                                  \
        }                                                                      \
    } while (0)


#define dc_def_enum(NAME, ...)                                                 \
    typedef enum                                                               \
    {                                                                          \
        __VA_ARGS__                                                            \
    } NAME

#define tostr_enum_scase(ITEM)                                                 \
    case ITEM:                                                                 \
        return #ITEM

// ***************************************************************************************
// * NON-POINTER ARRAY MACROS
// ***************************************************************************************

#define dc_arr_lit(TYPE, ...)                                                  \
    (TYPE[])                                                                   \
    {                                                                          \
        __VA_ARGS__, DC_ARR_TERMINATOR_##TYPE                                  \
    }

#define dc_array(NAME, TYPE, ...)                                              \
    TYPE NAME[] = {__VA_ARGS__, DC_ARR_TERMINATOR_##TYPE}

#define dc_count(ARR) (size)(sizeof(ARR) / sizeof(*(ARR)))

#define dc_len(ARR) (dc_count(ARR) - 1)

#define dc_last(ARR) ARR[(dc_len(ARR) - 1)]

#define dc_foreach(ARR, TYPE, IT)                                              \
    for (TYPE* IT = ARR; !DC_IS_ARR_TERMINATOR_##TYPE(*IT); ++IT)

#define dc_oneach(ARR, TYPE, FN) dc_foreach(ARR, TYPE, _element) FN(_element)

#define dc_foreach_lit(TYPE, IT, ...)                                          \
    for (TYPE* IT = dc_arr_lit(TYPE, __VA_ARGS__);                             \
         !DC_IS_ARR_TERMINATOR_##TYPE(*IT); ++IT)

#define dc_oneach_lit(TYPE, FN, ...)                                           \
    dc_foreach_lit(TYPE, item, __VA_ARGS__) FN(item)


// ***************************************************************************************
// * POINTER ARRAY MACROS
// ***************************************************************************************

#define dc_parr_lit(TYPE, ...)                                                 \
    (TYPE[])                                                                   \
    {                                                                          \
        __VA_ARGS__, DC_ARR_TERMINATOR_VOIDPTR                                 \
    }
#define dc_parray(NAME, TYPE, ...) TYPE** NAME = dc_parr_lit(TYPE*, __VA_ARGS__)

#define dc_pforeach(ARR, TYPE, IT)                                             \
    for (TYPE** IT = ARR; !DC_IS_ARR_TERMINATOR_PTR(*IT); ++IT)

#define dc_poneach(ARR, TYPE, FN) dc_pforeach(ARR, TYPE, _element) FN(_element)

#define dc_pforeach_lit(TYPE, IT, ...)                                         \
    for (TYPE* IT = dc_parr_lit(TYPE, __VA_ARGS__);                            \
         !DC_IS_ARR_TERMINATOR_PTR(*IT); ++IT)

#define dc_poneach_lit(TYPE, FN, ...)                                          \
    dc_pforeach_lit(TYPE*, item, __VA_ARGS__) FN(*item)

// ***************************************************************************************
// * STRUCT ARRAY MACROS
// ***************************************************************************************

#define dc_sarr_lit(TYPE, ...)                                                 \
    (TYPE[])                                                                   \
    {                                                                          \
        __VA_ARGS__                                                            \
    }
#define dc_sarray(NAME, TYPE, ...) TYPE NAME[] = {__VA_ARGS__}

#define dc_sforeach(ARR, TYPE, IT, TERMINATION_CONDITION)                      \
    for (TYPE* IT = ARR; TERMINATION_CONDITION; ++IT)

#define dc_soneach(ARR, TYPE, TERMINATION_CONDITION, FN)                       \
    dc_sforeach(ARR, TYPE, _element, TERMINATION_CONDITION) FN(_element)

#define dc_sforeach_lit(TYPE, IT, TERMINATION_CONDITION, ...)                  \
    for (TYPE* IT = dc_sarr_lit(TYPE, __VA_ARGS__); TERMINATION_CONDITION; ++IT)

#define dc_soneach_lit(TYPE, TERMINATION_CONDITION, FN, ...)                   \
    dc_sforeach_lit(TYPE, _element, TERMINATION_CONDITION, __VA_ARGS__)        \
        FN(_element)

// ***************************************************************************************
// * DYNAMIC ARRAY MACROS
// ***************************************************************************************

#ifndef DC_DYNARR_INITIAL_CAP
#define DC_DYNARR_INITIAL_CAP 4
#endif

#define dc_value_type(TYPE) DC_DYN_VAL_TYPE_##TYPE

#define dc_dynval_set(NAME, TYPE, VALUE)                                       \
    NAME.type = dc_value_type(TYPE);                                           \
    NAME.value.TYPE##_val = VALUE

#define dc_dynval_lit(TYPE, VALUE)                                             \
    (DCDynValue)                                                               \
    {                                                                          \
        .type = dc_value_type(TYPE), .value.TYPE##_val = VALUE                 \
    }

#define dc_dynval_make(NAME, TYPE, VALUE)                                      \
    DCDynValue NAME = {.type = dc_value_type(TYPE), .value.TYPE##_val = VALUE}

#define dc_dynval_is(NAME, TYPE) (NAME.type == dc_value_type(TYPE))

#define dc_dynval_get(NAME, TYPE) (NAME.value.TYPE##_val)

#endif // DC_MACROS_H
