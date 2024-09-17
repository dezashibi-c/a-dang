// ***************************************************************************************
//    Project: dcommon -> https://github.com/dezashibi-c/dcommon
//    File: aliases.h
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
// *  Description: private header file for definition of common type aliases
// *               DO NOT USE THIS DIRECTLY
// ***************************************************************************************

#ifndef DC_ALIASES_H
#define DC_ALIASES_H

#ifndef __DC_BYPASS_PRIVATE_PROTECTION
#error                                                                         \
    "You cannot use this header (aliases.h) directly, please consider including dcommon.h"
#endif

#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>

typedef uint8_t u8;
typedef int32_t i32;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef uintptr_t uptr;

typedef ptrdiff_t size;
typedef size_t usize;

typedef char* string;

typedef void* voidptr;

typedef enum
{
    DC_ERR_MODE_NORMAL,
    DC_ERR_MODE_ABORT,
} DC_ERROR_MODE;

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
#define DC_IS_ARR_TERMINATOR_PTR(EL) (EL == DC_ARR_TERMINATOR_voidptr)

#define DC_IS_ARR_TERMINATOR_char(EL) (EL == DC_ARR_TERMINATOR_char)

#define DC_IS_ARR_TERMINATOR_size(EL) (EL == DC_ARR_TERMINATOR_size)
#define DC_IS_ARR_TERMINATOR_usize(EL) (EL == DC_ARR_TERMINATOR_usize)

// Dynamic Array

typedef enum
{
    DC_DYN_VAL_TYPE_u8,
    DC_DYN_VAL_TYPE_i32,
    DC_DYN_VAL_TYPE_u32,
    DC_DYN_VAL_TYPE_u64,
    DC_DYN_VAL_TYPE_f32,
    DC_DYN_VAL_TYPE_f64,
    DC_DYN_VAL_TYPE_uptr,
    DC_DYN_VAL_TYPE_char,
    DC_DYN_VAL_TYPE_string,
    DC_DYN_VAL_TYPE_voidptr,
    DC_DYN_VAL_TYPE_size,
    DC_DYN_VAL_TYPE_usize,
} DCDynValueType;

typedef struct
{
    DCDynValueType type;
    union
    {
        u8 u8_val;
        i32 i32_val;
        u32 u32_val;
        u64 u64_val;
        f32 f32_val;
        f64 f64_val;
        uptr uptr_val;
        char char_val;
        string string_val;
        void* voidptr_val;
        size size_val;
        usize usize_val;
    } value;
} DCDynValue;

typedef void (*DCDynValFreeFunc)(DCDynValue*);

typedef struct
{
    DCDynValue* elements;
    usize cap;
    usize count;

    DCDynValFreeFunc element_free_func;
} DCDynArr;

typedef struct
{
    string str;
    usize len;
    string cstr;
} DCStringView;

typedef struct
{
    voidptr key;
    DCDynValue value;
} DCHashEntry;

typedef u32 (*DCHashFunc)(voidptr);
typedef bool (*DCKeyCompFunc)(voidptr, voidptr);

typedef struct
{
    DCDynArr* container;
    usize cap;
    usize key_count;

    DCHashFunc hash_func;
    DCKeyCompFunc key_cmp_func;
    DCDynValFreeFunc element_free_func;
} DCHashTable;

#endif // DC_ALIASES_H
