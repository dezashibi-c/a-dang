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
#error "You cannot use this header (aliases.h) directly, please consider including dcommon.h"
#endif

#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>

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
} DCResultStatus;

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
 * 2. If message is allocated the `allocated` field must be set to true.
 */
typedef struct
{
    i8 code;
    string message;
    bool allocated;
} DCError;


/**
 * Macro to define new Result type
 *
 * NOTE: See (alias.h) for usage in definition of default result types
 */
#define DCResultType(TYPE, NAME)                                                                                               \
    typedef struct                                                                                                             \
    {                                                                                                                          \
        DCResultStatus status;                                                                                                 \
        union                                                                                                                  \
        {                                                                                                                      \
            DCError e;                                                                                                         \
            TYPE v;                                                                                                            \
        } data;                                                                                                                \
    } NAME

/**
 * Minimum structure to be able to be cased and used as a Result is DCResultVoid
 */
typedef struct
{
    DCResultStatus status;
    union
    {
        DCError e;
    } data;
} DCResultVoid;

// ***************************************************************************************
// * DEFAULT PRIMITIVE RESULT TYPE DECLARATIONS
// ***************************************************************************************
DCResultType(i8, DCResultI8);
DCResultType(i16, DCResultI16);
DCResultType(i32, DCResultI32);
DCResultType(i64, DCResultI64);
DCResultType(u8, DCResultU8);
DCResultType(u16, DCResultU16);
DCResultType(u32, DCResultU32);
DCResultType(u64, DCResultU64);
DCResultType(f32, DCResultF32);
DCResultType(f64, DCResultF64);
DCResultType(uptr, DCResultUptr);
DCResultType(size, DCResultSize);
DCResultType(usize, DCResultUsize);
DCResultType(string, DCResultString);
DCResultType(voidptr, DCResultVoidptr);
DCResultType(fileptr, DCResultFileptr);

DCResultType(bool, DCResultBool);

// ***************************************************************************************
// * DYNAMIC ARRAY TYPE DECLARATIONS
// ***************************************************************************************

/**
 * All the types that DCDynVal can accept
 */
typedef enum
{
    DC_DYN_VAL_TYPE_i8,
    DC_DYN_VAL_TYPE_i16,
    DC_DYN_VAL_TYPE_i32,
    DC_DYN_VAL_TYPE_i64,

    DC_DYN_VAL_TYPE_u8,
    DC_DYN_VAL_TYPE_u16,
    DC_DYN_VAL_TYPE_u32,
    DC_DYN_VAL_TYPE_u64,

    DC_DYN_VAL_TYPE_f32,
    DC_DYN_VAL_TYPE_f64,

    DC_DYN_VAL_TYPE_uptr,
    DC_DYN_VAL_TYPE_char,
    DC_DYN_VAL_TYPE_size,
    DC_DYN_VAL_TYPE_usize,
    DC_DYN_VAL_TYPE_string,
    DC_DYN_VAL_TYPE_voidptr,
    DC_DYN_VAL_TYPE_fileptr,
} DCDynValType;

/**
 * Dynamic value type with ability to keep track of holding allocated string or
 * voidptr for further cleanup
 */
typedef struct
{
    DCDynValType type;
    bool allocated;
    union
    {
        i8 i8_val;
        i16 i16_val;
        i32 i32_val;
        i64 i64_val;

        u8 u8_val;
        u16 u16_val;
        u32 u32_val;
        u64 u64_val;

        f32 f32_val;
        f64 f64_val;

        uptr uptr_val;
        char char_val;
        string string_val;
        voidptr voidptr_val;
        fileptr fileptr_val;

        size size_val;
        usize usize_val;
    } value;
} DCDynVal;

/**
 * Custom function type for cleaning up a dynamic value
 *
 * As an example a Dynamic value might contain a voidptr to another struct that
 * holds allocated fields and they need to be de-allocated before the actual
 * voidptr value to be freed
 *
 * NOTE: Also the voidptr might be marked as allocated=false but still it's
 * corresponding value that it is pointing to might need to have a clean up
 * process (see dc_dv_free)
 */
typedef DCResultVoid (*DCDynValFreeFn)(DCDynVal*);

/**
 * Dynamic array with ability to keep any number of dynamic values
 *
 * Initial capacity and multiplication on grow is also customizable
 *
 * Dynamic arrays or Darr for short can be grown, truncated, popped, etc.
 */
typedef struct
{
    DCDynVal* elements;
    usize cap;
    usize count;
    usize multiplier;

    DCDynValFreeFn element_free_fn;
} DCDynArr;

// ***************************************************************************************
// * STRING VIEW TYPE DECLARATION
// ***************************************************************************************

/**
 * It is used to address a portion of a string without memory allocation.
 *
 * NOTE: cstr when initiated will be allocated to the exact copy of the string
 * piece SV pointing to
 */
typedef struct
{
    string str;
    usize len;
    string cstr;
} DCStringView;

// ***************************************************************************************
// * HASH TABLE TYPE DECLARATIONS
// ***************************************************************************************

/**
 * Each entry of a hash table can have a dynamic value
 *
 * NOTE: The key is voidptr and must be fixed or properly handled throughout
 * hash function process
 */
typedef struct
{
    voidptr key;
    DCDynVal value;
} DCHashEntry;

/**
 * Function pointer type as an acceptable hash function for an Hash Table
 */
typedef DCResultU32 (*DCHashFn)(voidptr);

/**
 * Key comparison function type for an Hash Table
 */
typedef DCResultBool (*DCKeyCompFn)(voidptr, voidptr);

/**
 * A Hash Table with track of capacity and number of registered keys
 *
 * Container is a fixed (one time allocated) array of dynamic arrays which will
 * help in case any collision happen with different keys
 */
typedef struct
{
    DCDynArr* container;
    usize cap;
    usize key_count;

    DCHashFn hash_fn;
    DCKeyCompFn key_cmp_fn;
    DCDynValFreeFn element_free_fn;
} DCHashTable;

// ***************************************************************************************
// * MEMORY CLEANUP TYPE DECLARATIONS
// ***************************************************************************************

/**
 * It is an alias for regular dynamic array that holds voidptr to multiple
 * DCCleanupJob (a batch of cleanup jobs)
 */
typedef DCDynArr DCCleanupBatch;

/**
 * Is a pool of cleanups that can be triggered at any phase of program life
 * cycle. (a pool of cleanup batches)
 *
 * As an example you might separate your program into multiple phases and
 * you want to have separate batch for each phase, you can use this method.
 *
 * NOTE: The default batch is batch index 0
 */
typedef struct
{
    DCCleanupBatch* pool;
    usize count;
} DCCleanupPool;

/**
 * Function pointer type for acceptable functions in cleanup process
 *
 * NOTE: If a dynamic value has a complex cleanup process you need to create
 * proper function with this signature and register it with the created function
 */
typedef DCResultVoid (*DCCleanupFn)(voidptr);

/**
 * Is a memory allocated object that needs to be managed
 */
typedef struct
{
    voidptr element;
    DCCleanupFn cleanup_fn;
} DCCleanupJob;

// ***************************************************************************************
// * DCOMMON CUSTOM TYPES RESULT TYPE DECLARATIONS
// ***************************************************************************************

DCResultType(DCDynVal, DCResult);
DCResultType(DCStringView, DCResultSv);
DCResultType(DCDynArr*, DCResultDa);
DCResultType(DCHashTable*, DCResultHt);
DCResultType(DCDynVal*, DCResultDv);

#endif // DC_ALIASES_H
