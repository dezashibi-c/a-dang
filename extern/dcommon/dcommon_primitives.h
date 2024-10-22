// ***************************************************************************************
//    Project: dcommon -> https://github.com/dezashibi-c/dcommon
//    File: dcommon_primitives.h
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
// *  Description: `dcommon` primitive type declarations
// ***************************************************************************************

#ifndef DCOMMON_PRIMITIVES_H
#define DCOMMON_PRIMITIVES_H

#define __DC_BYPASS_PRIVATE_PROTECTION

#include "_headers/general.h"

// ***************************************************************************************
// * PRIMITIVE TYPES DECLARATIONS
// ***************************************************************************************

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef uintptr_t uptr;

typedef ptrdiff_t size;
typedef size_t usize;

typedef char* string;

typedef void* voidptr;
typedef FILE* fileptr;

typedef struct DCStringView DCStringView;

// ***************************************************************************************
// * RESULT TYPE DECLARATIONS
// ***************************************************************************************

/**
 * Enum for two possible result status, ok and error
 */
typedef enum
{
    DC_RES_OK,
    DC_RES_ERR,
} DCResStatus;

/**
 * Holds error data when the result is an error
 *
 * Notes:
 *
 * 1. There are reserved codes (Not mandatory but `dcommon` follows that)
 * - '1' Null Value detected
 * - '2' Memory management failed, malloc, calloc, realloc, etc.
 * - '3' Unknown type or un-matched type detected
 * - '4' Index out of bound
 * - '5' Internal errors
 * - '6' Not Found
 *
 * NOTE: You can use macros `dc_err` and `dc_e_msg`
 *
 * 2. If message is allocated the `allocated` field must be set to true.
 */
typedef struct
{
    i8 code;
    string message;
    bool allocated;
} DCError;

/**
 * `[MACRO]` defines new Result type
 *
 * NOTE: See (alias.h) for usage in definition of default result types
 */
#define DCResType(TYPE, NAME)                                                                                                  \
    typedef struct                                                                                                             \
    {                                                                                                                          \
        DCResStatus status;                                                                                                    \
        union                                                                                                                  \
        {                                                                                                                      \
            DCError e;                                                                                                         \
            TYPE v;                                                                                                            \
        } data;                                                                                                                \
    } NAME

/**
 * Minimum structure to be able to be cased and used as a Result is DCResVoid
 */
typedef struct
{
    DCResStatus status;
    union
    {
        DCError e;
    } data;
} DCResVoid;

typedef struct DCKeyValuePair DCKeyValuePair;

#undef __DC_BYPASS_PRIVATE_PROTECTION

#endif // DCOMMON_PRIMITIVES_H
