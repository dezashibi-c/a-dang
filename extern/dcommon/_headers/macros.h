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

#if defined(__GNUC__) || defined(__clang__)
#define ATTRIB(A) __attribute__(A)
#else
#define ATTRIB(A)
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

#define tostr_enum_scase(ITEM)                                                 \
    case ITEM:                                                                 \
        return #ITEM

// ***************************************************************************************
// * NON-POINTER ARRAY MACROS
// ***************************************************************************************

#define dc_arr_terminator(TYPE) (DC_ARR_TERMINATOR_##TYPE)

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

#ifndef DC_DYNARR_CAP_MULTIPLIER
#define DC_DYNARR_CAP_MULTIPLIER 2
#endif

#define dc_value_type(TYPE) DC_DYN_VAL_TYPE_##TYPE

#define dc_dynval_set(NAME, TYPE, VALUE)                                       \
    (NAME).type = dc_value_type(TYPE);                                         \
    (NAME).value.TYPE##_val = VALUE

#define dc_dynval_lit(TYPE, VALUE)                                             \
    (DCDynValue)                                                               \
    {                                                                          \
        .type = dc_value_type(TYPE), .value.TYPE##_val = VALUE                 \
    }

#define dc_dynval_make(NAME, TYPE, VALUE)                                      \
    DCDynValue NAME = {.type = dc_value_type(TYPE), .value.TYPE##_val = VALUE}

#define dc_dynval_is(NAME, TYPE) ((NAME).type == dc_value_type(TYPE))

#define dc_dynval_is_not(NAME, TYPE) ((NAME).type != dc_value_type(TYPE))

#define dc_dynval_get(NAME, TYPE) ((NAME).value.TYPE##_val)

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


// ***************************************************************************************
// * STRING_VIEW MACROS
// ***************************************************************************************

#define DC_SV_FMT "%.*s"
#define dc_sv_fmt_val(SV) (u32)((SV).len), (SV).str

#define dc_sv_str_eq(SV, STR)                                                  \
    ((strlen(STR) == (SV).len) && (strncmp((SV).str, STR, (SV).len) == 0))

#define dc_sv_sv_eq(SV1, SV2)                                                  \
    (((SV1).len == (SV2).len) &&                                               \
     (strncmp((SV1).str, (SV2).str, (SV1).len) == 0))

// ***************************************************************************************
// * COLOR MACROS
// ***************************************************************************************

#define DC_FG_BLACK "\x1B[30m"
#define DC_FG_RED "\x1B[31m"
#define DC_FG_GREEN "\x1B[32m"
#define DC_FG_YELLOW "\x1B[33m"
#define DC_FG_BLUE "\x1B[34m"
#define DC_FG_MAGENTA "\x1B[35m"
#define DC_FG_CYAN "\x1B[36m"
#define DC_FG_WHITE "\x1B[37m"
#define DC_FG_GRAY "\x1B[90m"
#define DC_FG_LRED "\x1B[91m"
#define DC_FG_LGREEN "\x1B[92m"
#define DC_FG_LYELLOW "\x1B[93m"
#define DC_FG_LBLUE "\x1B[94m"
#define DC_FG_LMAGENTA "\x1B[95m"
#define DC_FG_LCYAN "\x1B[96m"
#define DC_FG_LWHITE "\x1B[97m"

#define DC_BG_BLACK "\x1B[40m"
#define DC_BG_RED "\x1B[41m"
#define DC_BG_GREEN "\x1B[42m"
#define DC_BG_YELLOW "\x1B[43m"
#define DC_BG_BLUE "\x1B[44m"
#define DC_BG_MAGENTA "\x1B[45m"
#define DC_BG_CYAN "\x1B[46m"
#define DC_BG_WHITE "\x1B[47m"
#define DC_BG_GRAY "\x1B[100m"
#define DC_BG_LRED "\x1B[101m"
#define DC_BG_LGREEN "\x1B[102m"
#define DC_BG_LYELLOW "\x1B[103m"
#define DC_BG_LBLUE "\x1B[104m"
#define DC_BG_LMAGENTA "\x1B[105m"
#define DC_BG_LCYAN "\x1B[106m"
#define DC_BG_LWHITE "\x1B[107m"

#define DC_COLOR_RESET "\x1B[0m"

#define dc_colorize(BG_COLOR, FG_COLOR, TEXT)                                  \
    DC_BG_##BG_COLOR operator DC_FG_##FG_COLOR TEXT operator DC_COLOR_RESET

#define dc_colorize_fg(FG_COLOR, TEXT) DC_FG_##FG_COLOR TEXT DC_COLOR_RESET

#define dc_colorize_bg(BG_COLOR, TEXT) DC_BG_##BG_COLOR TEXT DC_COLOR_RESET

#endif // DC_MACROS_H
