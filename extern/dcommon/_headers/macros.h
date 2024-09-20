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

// ***************************************************************************************
// * PRIMITIVE TYPES MACROS
// ***************************************************************************************

#define DC_ARR_TERMINATOR_u8 UINT8_MAX   // 255
#define DC_ARR_TERMINATOR_i32 INT32_MAX  // 2147483647
#define DC_ARR_TERMINATOR_u32 UINT32_MAX // 4294967295
#define DC_ARR_TERMINATOR_u64 UINT64_MAX // 18446744073709551615

#define DC_ARR_TERMINATOR_f32 NAN
#define DC_ARR_TERMINATOR_f64 NAN

#define DC_ARR_TERMINATOR_uptr (uptr) NULL
#define DC_ARR_TERMINATOR_string NULL
#define DC_ARR_TERMINATOR_voidptr NULL

#define DC_ARR_TERMINATOR_char '\0'

#define DC_ARR_TERMINATOR_size -1
#define DC_ARR_TERMINATOR_usize SIZE_MAX

// Terminator checks for each type
#define DC_IS_ARR_TERMINATOR_u8(EL) (EL == DC_ARR_TERMINATOR_u8)
#define DC_IS_ARR_TERMINATOR_i32(EL) (EL == DC_ARR_TERMINATOR_i32)
#define DC_IS_ARR_TERMINATOR_u32(EL) (EL == DC_ARR_TERMINATOR_u32)
#define DC_IS_ARR_TERMINATOR_u64(EL) (EL == DC_ARR_TERMINATOR_u64)

#define DC_IS_ARR_TERMINATOR_f32(EL) (isnan(EL))
#define DC_IS_ARR_TERMINATOR_f64(EL) (isnan(EL))

#define DC_IS_ARR_TERMINATOR_uptr(EL) (EL == DC_ARR_TERMINATOR_uptr)
#define DC_IS_ARR_TERMINATOR_string(EL) (EL == DC_ARR_TERMINATOR_string)
#define DC_IS_ARR_TERMINATOR_voidptr(EL) (EL == DC_ARR_TERMINATOR_voidptr)

#define DC_IS_ARR_TERMINATOR_char(EL) (EL == DC_ARR_TERMINATOR_char)

#define DC_IS_ARR_TERMINATOR_size(EL) (EL == DC_ARR_TERMINATOR_size)
#define DC_IS_ARR_TERMINATOR_usize(EL) (EL == DC_ARR_TERMINATOR_usize)

// ***************************************************************************************
// * UTILITY MACROS
// ***************************************************************************************

#if defined(__GNUC__) || defined(__clang__)
#define __dc_attribute(A) __attribute__(A)
#else
#define __dc_attribute(A)
#endif

#define dc_system(OUT_VAL, ...)                                                \
    do                                                                         \
    {                                                                          \
        string __cmd_string;                                                   \
        dc_sprintf(&__cmd_string, __VA_ARGS__);                                \
        OUT_VAL = system(__cmd_string);                                        \
        free(__cmd_string);                                                    \
    } while (0)

#define dc_str_case(ITEM)                                                      \
    case ITEM:                                                                 \
        return #ITEM

// ***************************************************************************************
// * DETECT OPERATING SYSTEM MACROS
// ***************************************************************************************
// clang-format off
#if defined(_WIN32) || defined(_WIN64)
    #define DC_WINDOWS
    #if defined(_M_X64)
        #define DC_WINDOWS_X64
    #elif defined(_M_IX86)
        #define DC_WINDOWS_X86
    #elif defined(_M_ARM64)
        #define DC_WINDOWS_ARM64
    #elif defined(_M_ARM)
        #define DC_WINDOWS_ARM
    #endif
#elif defined(__APPLE__) && defined(__MACH__)
    #include <TargetConditionals.h>
    #if TARGET_OS_MAC == 1
        #define DC_MACOS
        #if defined(__x86_64__) || defined(__aarch64__)
            #define DC_MACOS_X64
        #else
            #define DC_MACOS_X86
        #endif
    #endif
#elif defined(__linux__)
    #define DC_LINUX
    #if defined(__x86_64__)
        #define DC_LINUX_X64
    #elif defined(__aarch64__)
        #define DC_LINUX_ARM64
    #elif defined(__arm__)
        #define DC_LINUX_ARM
    #else
        #define DC_LINUX_X86
    #endif
#elif defined(__ANDROID__)
    #define DC_ANDROID
    #if defined(__x86_64__)
        #define DC_ANDROID_X64
    #elif defined(__aarch64__)
        #define DC_ANDROID_ARM64
    #elif defined(__arm__)
        #define DC_ANDROID_ARM
    #else
        #define DC_ANDROID_X86
    #endif
#elif defined(__unix__)
    #define DC_UNIX
    #if defined(__x86_64__)
        #define DC_UNIX_X64
    #elif defined(__aarch64__)
        #define DC_UNIX_ARM64
    #elif defined(__arm__)
        #define DC_UNIX_ARM
    #else
        #define DC_UNIX_X86
    #endif
#endif
// clang-format on

// ***************************************************************************************
// * INTERNAL LOG MACROS MACROS
// ***************************************************************************************

#define ____dc_log(CAT, ...)                                                   \
    do                                                                         \
    {                                                                          \
        fprintf(dc_error_logs ? dc_error_logs : stderr, "[%s] %s:%d: ", CAT,   \
                __FILE__, __LINE__);                                           \
        fprintf(dc_error_logs ? dc_error_logs : stderr, __VA_ARGS__);          \
        fprintf(dc_error_logs ? dc_error_logs : stderr, "\n");                 \
    } while (0)

#define ____dc_log_if(CAT, CONDITION, ...)                                     \
    do                                                                         \
    {                                                                          \
        if (CONDITION)                                                         \
        {                                                                      \
            ____dc_log(CAT, __VA_ARGS__);                                      \
        }                                                                      \
    } while (0)

#define ____dc_action_on(CAT, CONDITION, FAILURE_ACTION, ...)                  \
    do                                                                         \
    {                                                                          \
        if (CONDITION)                                                         \
        {                                                                      \
            ____dc_log(CAT, __VA_ARGS__);                                      \
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


// ***************************************************************************************
// * LOG MACROS
// ***************************************************************************************

#define dc_log(...) ____dc_log("LOG", __VA_ARGS__)

#define dc_log_if(CONDITION, ...) ____dc_log_if("LOG", CONDITION, __VA_ARGS__)

#define dc_action_on(CONDITION, FAILURE_ACTION, ...)                           \
    ____dc_action_on("LOG", CONDITION, FAILURE_ACTION, __VA_ARGS__)


#ifdef DC_DEBUG

#define dc_dbg_log(...) ____dc_log("DEBUG", __VA_ARGS__)

#define dc_dbg_log_if(CONDITION, ...)                                          \
    ____dc_log_if("DEBUG", CONDITION, __VA_ARGS__)

#define dc_dbg_action_on(CONDITION, FAILURE_ACTION, ...)                       \
    ____dc_action_on("DEBUG", CONDITION, FAILURE_ACTION, __VA_ARGS__)

#else

#define dc_dbg_log(...)

#define dc_dbg_log_if(CONDITION, ...)

#define dc_dbg_action_on(CONDITION, FAILURE_ACTION, ...)

#endif

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
    for (TYPE** _it = ARR; !DC_IS_ARR_TERMINATOR_voidptr(*_it); ++_it)

#define dc_poneach(ARR, TYPE, FN) dc_pforeach(ARR, TYPE) FN(_it)

#define dc_pforeach_lit(TYPE, ...)                                             \
    for (TYPE* _it = dc_parr_lit(TYPE, __VA_ARGS__);                           \
         !DC_IS_ARR_TERMINATOR_voidptr(*_it); ++_it)

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

#define dc_dv_free_func_decl(NAME) void NAME(DCDynValue* _value)

#define dc_dvt(TYPE) DC_DYN_VAL_TYPE_##TYPE

#define dc_dv(TYPE, VALUE)                                                     \
    (DCDynValue)                                                               \
    {                                                                          \
        .type = dc_dvt(TYPE), .value.TYPE##_val = VALUE                        \
    }

#define dc_dv_make(NAME, TYPE, VALUE)                                          \
    DCDynValue NAME = {.type = dc_dvt(TYPE), .value.TYPE##_val = VALUE}

#define dc_dv_set(NAME, TYPE, VALUE)                                           \
    (NAME).type = dc_dvt(TYPE);                                                \
    (NAME).value.TYPE##_val = VALUE

#define dc_dv_is(NAME, TYPE) ((NAME).type == dc_dvt(TYPE))

#define dc_dv_is_not(NAME, TYPE) ((NAME).type != dc_dvt(TYPE))

#define dc_dv_as(NAME, TYPE) ((NAME).value.TYPE##_val)

#define dc_da_get_as(DYNARR, INDEX, TYPE)                                      \
    dc_dv_as(*dc_da_get(DYNARR, INDEX), TYPE)

#define dc_da_is(DYNARR, INDEX, TYPE) dc_dv_is((DYNARR).elements[INDEX], TYPE)

#define dc_da_is_not(DYNARR, INDEX, TYPE)                                      \
    dc_dv_is_not((DYNARR).elements[INDEX], TYPE)

#define dc_da_for(DYNARR) for (usize _idx = 0; _idx < (DYNARR).count; _idx++)

// Function to initialize the dynamic array with initial values
#define dc_da_init_with_values(DYNARRPTR, FREE_FUNC, ...)                      \
    do                                                                         \
    {                                                                          \
        dc_sarray(__initial_values, DCDynValue, __VA_ARGS__);                  \
        ___dc_da_init_with_values(DYNARRPTR, dc_count(__initial_values),       \
                                  FREE_FUNC, __initial_values);                \
    } while (0)

#define dc_da_append_values(DYNARRPTR, ...)                                    \
    do                                                                         \
    {                                                                          \
        dc_sarray(__initial_values, DCDynValue, __VA_ARGS__);                  \
        ___dc_da_append_values(DYNARRPTR, dc_count(__initial_values),          \
                               __initial_values);                              \
    } while (0)

#define dc_da_insert_values(DYNARRPTR, INDEX, ...)                             \
    do                                                                         \
    {                                                                          \
        dc_sarray(__initial_values, DCDynValue, __VA_ARGS__);                  \
        ___dc_da_insert_values(DYNARRPTR, INDEX, dc_count(__initial_values),   \
                               __initial_values);                              \
    } while (0)

#define ___dc_da_converters_decl(ORIGIN_TYPE)                                  \
    usize dc_##ORIGIN_TYPE##_da_to_flat_arr(                                   \
        DCDynArr* arr, ORIGIN_TYPE** out_arr, bool must_fail)

#define ___dc_da_converters_impl(ORIGIN_TYPE)                                  \
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
        if (elem->type != dc_dvt(ORIGIN_TYPE))                                 \
        {                                                                      \
            if (must_fail)                                                     \
            {                                                                  \
                free(*out_arr);                                                \
                *out_arr = NULL;                                               \
                return 0;                                                      \
            }                                                                  \
            continue;                                                          \
        }                                                                      \
        (*out_arr)[dest_index] = dc_dv_as((*elem), ORIGIN_TYPE);               \
        dest_index++;                                                          \
    }                                                                          \
    (*out_arr)[dest_index] = dc_arr_terminator(ORIGIN_TYPE);                   \
    return dest_index

// ***************************************************************************************
// * HASH TABLE MACROS
// ***************************************************************************************

#define dc_ht_hash_func_decl(NAME) u32 NAME(voidptr _key)
#define dc_ht_key_comp_func_decl(NAME) bool NAME(voidptr _key1, voidptr _key2)

#define dc_ht_get_hash(VAR_NAME, HT, KEY)                                      \
    u32 VAR_NAME = (HT).hash_func((KEY)) % (HT).cap

#define dc_ht_get_container_row(VAR_NAME, HT, HASH)                            \
    DCDynArr* VAR_NAME = &((HT).container[HASH])

#define dc_ht_entry(KEY, VAL)                                                  \
    (DCHashEntry)                                                              \
    {                                                                          \
        .key = (KEY), .value = (VAL)                                           \
    }

#define dc_ht_set_multiple(HT, ...)                                            \
    do                                                                         \
    {                                                                          \
        dc_sarray(__initial_values, DCHashEntry, __VA_ARGS__);                 \
        ___dc_ht_set_multiple(HT, dc_count(__initial_values),                  \
                              __initial_values);                               \
    } while (0)

// ***************************************************************************************
// * STRING VIEW MACROS
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

// ***************************************************************************************
// * CLEANUP MACROS
// ***************************************************************************************

#define dc_cleanups_func_decl(NAME) void NAME(voidptr _value)

#define dc_cleanup_do(ENTRY) (ENTRY).cleanup_func(((ENTRY).element))

#define ___dc_cleanups_arr_init(CLEANUPS, CAPACITY)                            \
    if ((CLEANUPS).cap == 0) dc_da_init_custom(&(CLEANUPS), CAPACITY, 3, NULL)

#define dc_global_cleanups_init(CAPACITY)                                      \
    do                                                                         \
    {                                                                          \
        ___dc_cleanups_arr_init(dc_cleanups, CAPACITY);                        \
        atexit(___dc_perform_global_cleanup);                                  \
        signal(SIGINT, ___dc_handle_signal);                                   \
    } while (0)

#define dc_local_cleanup_decl(RETVAL_TYPE, RETVAL_INIT, CAPACITY)              \
    RETVAL_TYPE ___dc_ret_val = RETVAL_INIT;                                   \
    DCCleanups ____dc_local_cleanups = {0};                                    \
    dc_da_init_custom(&____dc_local_cleanups, CAPACITY, 3, NULL)

#define dc_cleanups_push(ELEMENT, CLEAN_FUNC)                                  \
    dc_dbg_log("global cleanup push: %p", (voidptr)ELEMENT);                   \
    ____dc_cleanups_custom_push(&dc_cleanups, ELEMENT, CLEAN_FUNC)

#define dc_local_cleanup_push(ELEMENT, CLEAN_FUNC)                             \
    dc_dbg_log("local cleanup push: %p", (voidptr)ELEMENT);                    \
    ____dc_cleanups_custom_push(&____dc_local_cleanups, ELEMENT, CLEAN_FUNC)

#define dc_perform_local_cleanups()                                            \
    dc_dbg_log("performing local cleanups");                                   \
    ___dc_perform_cleanup(&____dc_local_cleanups)

#define dc_cleanups_push_ht(ELEMENT) dc_cleanups_push(ELEMENT, dc_ht_free__)
#define dc_local_cleanups_push_ht(ELEMENT)                                     \
    dc_local_cleanup_push(ELEMENT, dc_ht_free__)

#define dc_cleanups_push_da(ELEMENT) dc_cleanups_push(ELEMENT, dc_da_free__)
#define dc_local_cleanups_push_da(ELEMENT)                                     \
    dc_local_cleanup_push(ELEMENT, dc_da_free__)

#define dc_cleanups_push_dv(ELEMENT) dc_cleanups_push(ELEMENT, dc_dv_free__)
#define dc_local_cleanups_push_dv(ELEMENT)                                     \
    dc_local_cleanup_push(ELEMENT, dc_dv_free__)

#define dc_def_local_cleanups_label()                                          \
    ___dc_perform_local_cleanups_label:                                        \
    dc_perform_local_cleanups();                                               \
    return ___dc_ret_val;

#define dc_return(RETVAL)                                                      \
    ___dc_ret_val = RETVAL;                                                    \
    goto ___dc_perform_local_cleanups_label

#endif // DC_MACROS_H
