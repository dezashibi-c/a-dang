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

#define dc_action_on(CONDITION, FAILURE_ACTION, ...)                           \
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

#define dc_arr_terminator(TYPE) (DC_ARR_TERMINATOR_##TYPE)

// ***************************************************************************************
// * NON-POINTER ARRAY MACROS
// ***************************************************************************************

#define dc_arr_lit(TYPE, ...)                                                  \
    (TYPE[])                                                                   \
    {                                                                          \
        __VA_ARGS__, dc_arr_terminator(TYPE)                                   \
    }

#define dc_array(NAME, TYPE, ...)                                              \
    TYPE NAME[] = {__VA_ARGS__, dc_arr_terminator(TYPE)}

#define dc_count(ARR) (size)(sizeof(ARR) / sizeof(*(ARR)))

#define dc_len(ARR) (dc_count(ARR) - 1)

#define dc_last(ARR) ARR[(dc_len(ARR) - 1)]

#define dc_foreach(ARR, TYPE)                                                  \
    for (TYPE* _it = ARR; !DC_IS_ARR_TERMINATOR_##TYPE(*_it); ++_it)

#define dc_oneach(ARR, TYPE, FN) dc_foreach(ARR, TYPE) FN(_it)

#define dc_foreach_lit(TYPE, ...)                                              \
    for (TYPE* _it = dc_arr_lit(TYPE, __VA_ARGS__);                            \
         !DC_IS_ARR_TERMINATOR_##TYPE(*_it); ++_it)

#define dc_oneach_lit(TYPE, FN, ...) dc_foreach_lit(TYPE, __VA_ARGS__) FN(_it)


// ***************************************************************************************
// * POINTER ARRAY MACROS
// ***************************************************************************************

#define dc_parr_lit(TYPE, ...)                                                 \
    (TYPE[])                                                                   \
    {                                                                          \
        __VA_ARGS__, DC_ARR_TERMINATOR_voidptr                                 \
    }
#define dc_parray(NAME, TYPE, ...) TYPE** NAME = dc_parr_lit(TYPE*, __VA_ARGS__)

#define dc_pforeach(ARR, TYPE)                                                 \
    for (TYPE** _it = ARR; !DC_IS_ARR_TERMINATOR_PTR(*_it); ++_it)

#define dc_poneach(ARR, TYPE, FN) dc_pforeach(ARR, TYPE) FN(_it)

#define dc_pforeach_lit(TYPE, ...)                                             \
    for (TYPE* _it = dc_parr_lit(TYPE, __VA_ARGS__);                           \
         !DC_IS_ARR_TERMINATOR_PTR(*_it); ++_it)

#define dc_poneach_lit(TYPE, FN, ...)                                          \
    dc_pforeach_lit(TYPE*, __VA_ARGS__) FN(*_it)

// ***************************************************************************************
// * STRUCT ARRAY MACROS
// ***************************************************************************************

#define dc_sarr_lit(TYPE, ...)                                                 \
    (TYPE[])                                                                   \
    {                                                                          \
        __VA_ARGS__                                                            \
    }
#define dc_sarray(NAME, TYPE, ...) TYPE NAME[] = {__VA_ARGS__}

#define dc_sforeach(ARR, TYPE, TERMINATION_CONDITION)                          \
    for (TYPE* _it = ARR; TERMINATION_CONDITION; ++_it)

#define dc_soneach(ARR, TYPE, TERMINATION_CONDITION, FN)                       \
    dc_sforeach(ARR, TYPE, TERMINATION_CONDITION) FN(_it)

#define dc_sforeach_lit(TYPE, TERMINATION_CONDITION, ...)                      \
    for (TYPE* _it = dc_sarr_lit(TYPE, __VA_ARGS__); TERMINATION_CONDITION;    \
         ++_it)

#define dc_soneach_lit(TYPE, TERMINATION_CONDITION, FN, ...)                   \
    dc_sforeach_lit(TYPE, TERMINATION_CONDITION, __VA_ARGS__) FN(_it)

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

#define dc_dynval_is_not(NAME, TYPE) (NAME.type != dc_value_type(TYPE))

#define dc_dynval_get(NAME, TYPE) (NAME.value.TYPE##_val)

// Function to initialize the dynamic array with initial values
#define dc_dynarr_init_with_values(DYNARRPTR, ...)                             \
    do                                                                         \
    {                                                                          \
        dc_sarray(__initial_values, DCDynValue, __VA_ARGS__);                  \
        ___dc_dynarr_init_with_values(DYNARRPTR, dc_count(__initial_values),   \
                                      __initial_values);                       \
    } while (0)

#define ___dc_dynval_converters_decl(ORIGIN_TYPE)                              \
    usize dc_##ORIGIN_TYPE##_dynarr_to_flat_arr(                               \
        DCDynArr* arr, ORIGIN_TYPE** out_arr, bool must_fail)

#define ___dc_dynval_converters_impl(ORIGIN_TYPE)                              \
    if (!arr || arr->count == 0 || !out_arr)                                   \
    {                                                                          \
        return 0;                                                              \
    }                                                                          \
    *out_arr = (ORIGIN_TYPE*)malloc((arr->count + 1) * sizeof(ORIGIN_TYPE));   \
    if (!(*out_arr))                                                           \
    {                                                                          \
        return 0;                                                              \
    }                                                                          \
    usize dest_index = 0;                                                      \
    for (usize i = 0; i < arr->count; ++i)                                     \
    {                                                                          \
        DCDynValue* elem = &arr->elements[i];                                  \
        if (elem->type != dc_value_type(ORIGIN_TYPE))                          \
        {                                                                      \
            if (must_fail)                                                     \
            {                                                                  \
                free(*out_arr);                                                \
                *out_arr = NULL;                                               \
                return 0;                                                      \
            }                                                                  \
            continue;                                                          \
        }                                                                      \
        (*out_arr)[dest_index] = dc_dynval_get((*elem), ORIGIN_TYPE);          \
        dest_index++;                                                          \
    }                                                                          \
    (*out_arr)[dest_index] = dc_arr_terminator(ORIGIN_TYPE);                   \
    return dest_index


#endif // DC_MACROS_H
