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

// ***************************************************************************************
// * GENERAL DECLARATIONS
// ***************************************************************************************

typedef enum
{
    DC_ERR_MODE_NORMAL,
    DC_ERR_MODE_ABORT,
} DC_ERROR_MODE;

// ***************************************************************************************
// * PRIMITIVE TYPES DECLARATIONS
// ***************************************************************************************

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

// ***************************************************************************************
// * DYNAMIC ARRAY TYPE DECLARATIONS
// ***************************************************************************************

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
    usize multiplier;

    DCDynValFreeFunc element_free_func;
} DCDynArr;

// ***************************************************************************************
// * STRING VIEW TYPE DECLARATION
// ***************************************************************************************

typedef struct
{
    string str;
    usize len;
    string cstr;
} DCStringView;

// ***************************************************************************************
// * HASH TABLE TYPE DECLARATIONS
// ***************************************************************************************

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

// ***************************************************************************************
// * MEMORY CLEANUP TYPE DECLARATIONS
// ***************************************************************************************

typedef DCDynArr DCCleanups;

typedef void (*DCCleanupFunc)(voidptr);

typedef struct
{
    voidptr element;
    DCCleanupFunc cleanup_func;
} DCCleanupEntry;

#endif // DC_ALIASES_H
