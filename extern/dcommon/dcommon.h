// ***************************************************************************************
//    Project: dcommon -> https://github.com/dezashibi-c/dcommon
//    File: dcommon.h
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
// *  Description: Main `dcommon` library key_value point
// ***************************************************************************************

#ifndef DC_MAIN_HEADER_H
#define DC_MAIN_HEADER_H


#include "dcommon_primitives.h"

#define __DC_BYPASS_PRIVATE_PROTECTION

#include "_headers/macros.h"

#include "_headers/aliases.h"

// ***************************************************************************************
// * FUNCTION DECLARATIONS
// ***************************************************************************************

/**
 * Converts a string to number of type i8
 *
 * @return DCResI8 (If OK i8 value can be retrieved otherwise it contains
 * proper error)
 */
DCResI8 dc_str_to_i8(const string str);

/**
 * Converts a string to number of type i16
 *
 * @return DCResI16 (If OK i16 value can be retrieved otherwise it contains
 * proper error)
 */
DCResI16 dc_str_to_i16(const string str);

/**
 * Converts a string to number of type i32
 *
 * @return DCResI32 (If OK i32 value can be retrieved otherwise it contains
 * proper error)
 */
DCResI32 dc_str_to_i32(const string str);

/**
 * Converts a string to number of type i64
 *
 * @return DCResI64 (If OK i64 value can be retrieved otherwise it contains
 * proper error)
 */
DCResI64 dc_str_to_i64(const string str);

/**
 * Converts a string to number of type u8
 *
 * @return DCResU8 (If OK u8 value can be retrieved otherwise it contains
 * proper error)
 */
DCResU8 dc_str_to_u8(const string str);

/**
 * Converts a string to number of type u16
 *
 * @return DCResU16 (If OK u16 value can be retrieved otherwise it contains
 * proper error)
 */
DCResU16 dc_str_to_u16(const string str);

/**
 * Converts a string to number of type u32
 *
 * @return DCResU32 (If OK u32 value can be retrieved otherwise it contains
 * proper error)
 */
DCResU32 dc_str_to_u32(const string str);

/**
 * Converts a string to number of type u64
 *
 * @return DCResU64 (If OK u64 value can be retrieved otherwise it contains
 * proper error)
 */
DCResU64 dc_str_to_u64(const string str);

/**
 * Converts a string to number of type f32
 *
 * @return DCResF32 (If OK f32 value can be retrieved otherwise it contains
 * proper error)
 */
DCResF32 dc_str_to_f32(const string str);

/**
 * Converts a string to number of type f64
 *
 * @return DCResF64 (If OK f64 value can be retrieved otherwise it contains
 * proper error)
 */
DCResF64 dc_str_to_f64(const string str);

// ***************************************************************************************

/**
 * Return corresponding format specifier based on the current value of the DCDynVal
 *
 * @return format specifier or ""
 */
string dc_dv_fmt(DCDynVal* dv);

/**
 * Returns string representation of the type the given dynamic value holds right now
 *
 * @return the string representation of the type, "(null dynamic value)" and "unknown or unimplemented"
 */
string dc_tostr_dvt(DCDynVal* dv);

/**
 * Checks for falsy values in a given dynamic value and return the result as
 * DCResBool
 *
 * @return DCResBool (If OK bool value can be retrieved otherwise it contains
 * proper error)
 */
DCResBool dc_dv_as_bool(DCDynVal* dv);

/**
 * Initializes a given pointer to dynamic array
 *
 * @param element_free_fn can be NULL or a proper function name which handles
 * complex cleanup for dynamic values stored in the defined dynamic array
 *
 * @return nothing or error
 */
DCResVoid dc_da_init(DCDynArr* darr, DCDynValFreeFn element_free_fn);

/**
 * Initializes a given pointer to dynamic array with custom capacity and grow
 * multiplier amounts
 *
 * @param element_free_fn can be NULL or a proper function name which handles
 * complex cleanup for dynamic values stored in the defined dynamic array
 *
 * @return nothing or error
 */
DCResVoid dc_da_init2(DCDynArr* darr, usize capacity, usize capacity_grow_multiplier, DCDynValFreeFn element_free_fn);

/**
 * Creates, allocates, initializes and returns a pointer to dynamic array
 *
 * @param element_free_fn can be NULL or a proper function name which handles
 * complex cleanup for dynamic values stored in the defined dynamic array
 *
 * @return dynamic array pointer (DCDynArr*) or error
 *
 * NOTE: Allocates memory
 */
DCResDa dc_da_new(DCDynValFreeFn element_free_fn);

/**
 * Creates, initializes and returns a pointer to dynamic array with custom
 * capacity and grow multiplier amounts
 *
 * @param element_free_fn can be NULL or a proper function name which handles
 * complex cleanup for dynamic values stored in the defined dynamic array
 *
 * @return dynamic array pointer (DCDynArr*) or error
 *
 * NOTE: Allocates memory
 */
DCResDa dc_da_new2(usize capacity, usize capacity_grow_multiplier, DCDynValFreeFn element_free_fn);

/**
 * Initializes a dynamic array with given array of dynamic value
 * literal array
 *
 * NOTE: see `dc_da_init_with_values` in macros.h, using it you can pass the
 * values without passing the count
 *
 * @return nothing or error
 */
DCResVoid __dc_da_init_with_values(DCDynArr* darr, usize count, DCDynValFreeFn element_free_fn, DCDynVal values[]);

/**
 * Grows the capacity of given dynamic array pointer by current capacity *
 * registered multiplier
 *
 * NOTE: default multiplier is 2
 *
 * @return nothing or error
 */
DCResVoid dc_da_grow(DCDynArr* darr);

/**
 * Grows the capacity of given dynamic array pointer by current capacity +
 * provided amount
 *
 * @return nothing or error
 */
DCResVoid dc_da_grow_by(DCDynArr* darr, usize amount);

/**
 * Grows the capacity of given dynamic array pointer to provided amount
 *
 * @return nothing or error
 */
DCResVoid dc_da_grow_to(DCDynArr* darr, usize amount);

/**
 * Truncates unused capacity of given dynamic array pointer
 *
 * @return nothing or error
 */
DCResVoid dc_da_trunc(DCDynArr* darr);

/**
 * Pops out the elements of a given dynamic array by amount of the given count
 *
 * @param out_popped is a pointer to a dynamic value pointer
 * @param truncate indicates whether to truncate the original array after
 * popping out values or not
 *
 * @return nothing or error
 */
DCResVoid dc_da_pop(DCDynArr* darr, usize count, DCDynVal** out_popped, bool truncate);

/**
 * Pushes given dynamic value to the given dynamic array
 *
 * @return nothing or error
 */
DCResVoid dc_da_push(DCDynArr* darr, DCDynVal value);

/**
 * Pushes multiple values to the given array with causes to check/grow the
 * capacity only once in case needed
 *
 * NOTE: see `dc_da_append_values` macro in macros.h, it helps passing values
 * without the count
 *
 * @return nothing or error
 */
DCResVoid __dc_da_append_values(DCDynArr* darr, usize count, DCDynVal values[]);

/**
 * Appends the elements from the `from` dynamic array to the given `darr`
 * dynamic array
 *
 * @return nothing or error
 */
DCResVoid dc_da_append(DCDynArr* darr, DCDynArr* from);

/**
 * Returns a pointer to an element of a dynamic array by its index
 *
 * @return a pointer to dynamic value or an error
 */
DCResDv dc_da_get(DCDynArr* darr, usize index);

/**
 * Checks whether two given pointers to dynamic values are equal or not
 *
 * @return bool or error
 */
DCResBool dc_dv_eq(DCDynVal* dv1, DCDynVal* dv2);

/**
 * Checks whether a pointer to a dynamic values is equal to
 * the given dynamic value or not
 *
 * @return bool or error
 */
DCResBool dc_dv_eq2(DCDynVal* dv1, DCDynVal dv2);

/**
 * Checks whether two given dynamic values are equal or not
 *
 * @return bool or error
 */
DCResBool dc_dv_eq3(DCDynVal dv1, DCDynVal dv2);

/**
 * Searches for given element (a pointer to a dynamic value) in an array
 *
 * @param dv_eq_fn is a function that compares custom extra types added to
 *                 dynamic value
 *
 * @return index or error
 *
 * NOTE: error code 6 means not found, other error types might happen as well
 */
DCResUsize dc_da_findp(DCDynArr* darr, DCDynVal* el, DCDvEqFn dv_eq_fn);

/**
 * Searches for given element (a literal dynamic value) in an array
 *
 * @param dv_eq_fn is a function that compares custom extra types added to
 *                 dynamic value
 *
 * @return index or error
 *
 * NOTE: error code 6 means not found, other error types might happen as well
 */
DCResUsize dc_da_find(DCDynArr* darr, DCDynVal el, DCDvEqFn dv_eq_fn);

/**
 * Frees allocated string or voidptr, does nothing for the rest of dynamic value
 * types
 *
 * NOTES:
 *
 * - For string types if they are allocated first sends them to `custom_free_fn`
 * if provided and then if the value is not `NULL` tries to `free` it and set it
 * to "".
 *
 * - For voidptr types it first send them to `custom_free_fn` then if it is
 * allocated frees it and set it to `NULL`.
 *
 * @return nothing or error
 */
DCResVoid dc_dv_free(DCDynVal* element, DCDynValFreeFn custom_free_fn);

/**
 * General function for cleanup process of a dynamic value
 *
 * NOTE: see cleanup related macros -> `dc_cleanup_push_dv`
 *
 * NOTE: If you need a custom cleanup function for your dynamic value that
 * handle some complex stuff you need to create it and use `dc_cleanup_push`
 * instead.
 *
 * @return nothing or error
 */
DCResVoid __dc_dv_free(voidptr dv);

/**
 * Frees all the dynamic values of the given dynamic array and then the array
 * itself and reset capacity back to zero
 *
 * @return nothing or error
 */
DCResVoid dc_da_free(DCDynArr* darr);

/**
 * General function for cleanup process of a dynamic array
 *
 * NOTE: see cleanup related macros -> `dc_cleanup_push_da`
 *
 * NOTE: If you need a custom cleanup function for your dynamic array that
 * handle some complex stuff you need to create it and use `dc_cleanup_push`
 * instead.
 *
 * @return nothing or error
 */
DCResVoid __dc_da_free(voidptr darr);

/**
 * Tries to delete an element by index in the given darr
 *
 * @return nothing or error
 */
DCResVoid dc_da_delete(DCDynArr* darr, usize index);

/**
 * Tries to delete an element by pointer in the given darr
 *
 * @param dv_eq_fn is a function that compares custom extra types added to
 *                 dynamic value
 *
 * NOTE: it first tries to find the item then it will delete it, so an error
 * with code 6 (not found) can happen.
 *
 * @return nothing or error (including code 6 -> not found)
 */
DCResVoid dc_da_delete_elp(DCDynArr* darr, DCDynVal* el, DCDvEqFn dv_eq_fn);

/**
 * Tries to delete an element in the given darr
 *
 * @param dv_eq_fn is a function that compares custom extra types added to
 *                 dynamic value
 *
 * NOTE: it first tries to find the item then it will delete it, so an error
 * with code 6 (not found) can happen.
 *
 * @return nothing or error (including code 6 -> not found)
 */
DCResVoid dc_da_delete_el(DCDynArr* darr, DCDynVal el, DCDvEqFn dv_eq_fn);

/**
 * Inserts given value at the given `start_index` (resize might happen)
 *
 * @return nothing or error
 */
DCResVoid dc_da_insert(DCDynArr* darr, usize index, DCDynVal value);

/**
 * Inserts multiple values at once at the given `start_index` (resize might
 * happen)
 *
 * NOTE: see `dc_da_insert_values` macro to avoid providing count
 *
 * @return nothing or error
 */
DCResVoid __dc_da_insert_values(DCDynArr* darr, usize start_index, usize count, DCDynVal values[]);

/**
 * Inserts items from a given `from` array to the original `darr` array at the
 * given `start_index`
 *
 * @return nothing or error
 */
DCResVoid dc_da_insert_from(DCDynArr* darr, usize start_index, DCDynArr* from);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than i8, when false the unmatched types will be
 * ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_i8_da_to_flat_arr(DCDynArr* arr, i8** out_arr, bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than i16, when false the unmatched types will be
 * ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_i16_da_to_flat_arr(DCDynArr* arr, i16** out_arr, bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than i32, when false the unmatched types will be
 * ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_i32_da_to_flat_arr(DCDynArr* arr, i32** out_arr, bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than i64, when false the unmatched types will be
 * ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_i64_da_to_flat_arr(DCDynArr* arr, i64** out_arr, bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than u8, when false the unmatched types will be
 * ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_u8_da_to_flat_arr(DCDynArr* arr, u8** out_arr, bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than u16, when false the unmatched types will be
 * ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_u16_da_to_flat_arr(DCDynArr* arr, u16** out_arr, bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than u32, when false the unmatched types will be
 * ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_u32_da_to_flat_arr(DCDynArr* arr, u32** out_arr, bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than u64, when false the unmatched types will be
 * ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_u64_da_to_flat_arr(DCDynArr* arr, u64** out_arr, bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than f32, when false the unmatched types will be
 * ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_f32_da_to_flat_arr(DCDynArr* arr, f32** out_arr, bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than f64, when false the unmatched types will be
 * ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_f64_da_to_flat_arr(DCDynArr* arr, f64** out_arr, bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than uptr, when false the unmatched types will
 * be ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_uptr_da_to_flat_arr(DCDynArr* arr, uptr** out_arr, bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than char, when false the unmatched types will
 * be ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_char_da_to_flat_arr(DCDynArr* arr, char** out_arr, bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than size, when false the unmatched types will
 * be ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_size_da_to_flat_arr(DCDynArr* arr, size** out_arr, bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than usize, when false the unmatched types will
 * be ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_usize_da_to_flat_arr(DCDynArr* arr, usize** out_arr, bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than string, when false the unmatched types will
 * be ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_string_da_to_flat_arr(DCDynArr* arr, string** out_arr, bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than voidptr, when false the unmatched types
 * will be ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_voidptr_da_to_flat_arr(DCDynArr* arr, voidptr** out_arr, bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than fileptr, when false the unmatched types
 * will be ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_fileptr_da_to_flat_arr(DCDynArr* arr, fileptr** out_arr, bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than DCStringView, when false the unmatched types
 * will be ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_DCStringView_da_to_flat_arr(DCDynArr* arr, DCStringView** out_arr, bool must_fail);

// ***************************************************************************************

/**
 * Initializes the given pointer to hash table with wanted capacity and other
 * information (see params)
 *
 * NOTE: capacity cannot and must not be changed after initialization
 *
 * @param hash_fn is the function that hashes the provided keys, keys are
 * voidptr so they can be anything so to say
 *
 * @param key_cmp_fn is the function that compares a provided key and keys in
 * the buckets
 *
 * @param key_value_free_fn as each hash key_value is saved as a dynamic value if they must be
 * freed using special process this is the parameter to be provided
 *
 * @return nothing or error
 */
DCResVoid dc_ht_init(DCHashTable* ht, usize capacity, DCHashFn hash_fn, DCKeyCompFn key_cmp_fn,
                     DCHtKeyValuePairFreeFn key_value_free_fn);

/**
 * Creates, allocates, initializes and returns a pointer to hash table
 *
 * @return hash table pointer (DCHashTable*) or error
 *
 * NOTE: Allocates memory
 */
DCResHt dc_ht_new(usize capacity, DCHashFn hash_fn, DCKeyCompFn key_cmp_fn, DCHtKeyValuePairFreeFn key_value_free_fn);

/**
 * Frees the given hash table and all the values
 *
 * @return nothing or error
 */
DCResVoid dc_ht_free(DCHashTable* ht);

/**
 * General free function for cleanup process see `dc_cleanup_push_ht` in macros
 *
 * @return nothing or error
 */
DCResVoid __dc_ht_free(voidptr ht);

/**
 * Searches for the key and provides the value
 *
 * @param out_result is the pointer to the dynamic value pointer in the hash
 * table
 *
 * @return index of the value in its bucket or error
 */
DCResUsize dc_ht_find_by_key(DCHashTable* ht, DCDynVal key, DCDynVal** out_result);

/**
 * Sets a value for the given key
 *
 * @param set_status indicates the action that must be taken when setting the key_value see `DCHashTableSetStatus`, in case of
 * failure error code 7 will be returned
 *
 * @return nothing or error
 */
DCResVoid dc_ht_set(DCHashTable* ht, DCDynVal key, DCDynVal value, DCHashTableSetStatus set_status);

/**
 * Inserts multiple key/values at once
 *
 * @param set_status indicates the action that must be taken when setting the key_value see `DCHashTableSetStatus`, in case of
 * failure error code 7 will be returned
 *
 * NOTE: see `dc_ht_set_multiple` macro in macro.h for easy addition without
 * providing count
 *
 * @return nothing or error
 */
DCResVoid __dc_ht_set_multiple(DCHashTable* ht, usize count, DCKeyValuePair entries[], DCHashTableSetStatus set_status);

/**
 * Merges and overwrites the key/values from the `from` hash table to the
 * original `ht` hash table
 *
 * @param set_status indicates the action that must be taken when setting the key_value see `DCHashTableSetStatus`, in case of
 * failure error code 7 will be returned
 *
 * @return nothing or error
 */
DCResVoid dc_ht_merge(DCHashTable* ht, DCHashTable* from, DCHashTableSetStatus set_status);

/**
 * Deletes the given key if key does not exists return false
 *
 * @return true if key exists, false if it doesn't or error
 */
DCResBool dc_ht_delete(DCHashTable* ht, DCDynVal key);

/**
 * Exports pointers to all the stored keys to the provided `out_arr` terminated with
 * dynamic value of null `dc_dv_nullptr()`
 *
 * @return the number of exported keys or error
 *
 * NOTE: Making changes in the array's element can tend into undefined behavior
 *
 * NOTE: Allocates memory
 */
DCResUsize dc_ht_keys(DCHashTable* ht, DCDynVal** out_arr);

// ***************************************************************************************

/**
 * Creates and return a string view literal struct
 *
 * @return a literal DCStringView or error
 */
DCResSv dc_sv_create(string base, usize start, usize length);

/**
 * Renders the string view into the cstr field or returns it if it is already
 * rendered
 *
 * @return string or error
 */
DCResString dc_sv_as_cstr(DCStringView* sv);

/**
 * Frees the string view's cstr field if it is initiated
 *
 * @return nothing or error
 */
DCResVoid dc_sv_free(DCStringView* sv);

// ***************************************************************************************

/**
 * Allocates and fill out the given string `str` with formats and data
 *
 * @return size of allocated string or error
 *
 * NOTE: Allocates memory
 */
DCResUsize dc_sprintf(string* str, string fmt, ...) __dc_attribute((format(printf, 2, 3)));

/**
 * Appends the given string `str` with given formats and data
 *
 * @return size of allocated string or error
 *
 * NOTE: Allocates memory when str is empty or reallocates it
 */
DCResUsize dc_sappend(string* str, const string fmt, ...) __dc_attribute((format(printf, 2, 3)));

/**
 * Duplicates given string and returns it
 *
 * @return string or error
 *
 * NOTE: Allocates memory
 */
DCResString dc_strdup(const string in);

/**
 * Converts the current value of the dynamic value to string
 *
 * @return string or error
 */
DCResString dc_tostr_dv(DCDynVal* dv);

/**
 * Prints the current value of the dynamic value (no new line)
 *
 * @return nothing or error
 */
DCResVoid dc_dv_print(DCDynVal* dv);

/**
 * Prints the current value of the dynamic value (with new line)
 *
 * @return nothing or error
 */
DCResVoid dc_dv_println(DCDynVal* dv);

/**
 * Replaces all the '\' with '/' in the original provided path
 *
 * @return nothing or error
 */
DCResVoid dc_normalize_path_to_posix(string path);

/**
 * Replaces the filename in the path with the given filename and returns a new
 * string as the result
 *
 * @returns string or error
 *
 * NOTE: Allocates memory
 */
DCResString dc_replace_file_in_path(const string path, const string file_name);

/**
 * Returns address of user's home directory
 *
 * @return string (might be NULL)
 */
string dc_get_home_dir_path();

/**
 * Returns current username
 *
 * @return string (might be NULL)
 */
string dc_get_username();

/**
 * Returns string representation of the operating system
 *
 * @returns string ("unknown" in case of failure)
 */
string dc_get_os();

/**
 * Returns string representation of the system architecture
 *
 * @returns string ("unknown" in case of failure)
 */
string dc_get_arch();

/**
 * Function for handling signals and trigger the cleanup in case of
 * pre-initializing
 *
 * NOTE: see Cleanup section in README.md
 */
void __dc_handle_signal(int sig);

/**
 * Initializes the global cleanup pool
 *
 * You can have 1 Batch with bigger capacity if you want to cleanup only once
 *
 * Or you can have multiple batches in the pool and cleanup them in different
 * parts of the program
 *
 * This also registers signal catcher functions `__dc_handle_signal` for proper
 * cleanups where applies
 */
void dc_cleanup_pool_init2(usize count, usize batch_capacity);

/**
 * Performs the cleanup process for any given cleanup batch
 *
 * NOTE: This function is meant to be used internally by the
 * `dc_cleanup_pool_run`, but you can indeed define your own
 * `DCCleanupBatch` variable and use `dc_cleanup_push2` pushing to it and
 * cleaning it up using this function and that's totally ok if you need to.
 *
 * @return nothing or error
 */
DCResVoid dc_cleanup_batch_run(DCCleanupBatch* batch);

/**
 * Performs the whole pool or selected cleanup or nothing
 *
 * @param selection choose DC_CLEANUP_POOL or -1, DC_NO_CLEANUP or -2 or other
 * valid batch index starting from 0
 *
 * @return nothing or exits with error code
 */
void dc_cleanup_pool_run(i32 selection);

/**
 * Pushes an allocated address and corresponding cleanup function to be
 * stored in the given cleanup batch
 *
 * @return nothing or exits with error code
 */
void dc_cleanup_push2(DCCleanupBatch* batch, voidptr element, DCCleanupFn cleanup_fn);

/**
 * Wrapper for `free` in case to be able to be used in cleanup process
 *
 * @returns nothing or error
 *
 * NOTE: Non-Null allocated pointer must be sent otherwise undefined behavior
 * might happen
 */
DCResVoid dc_free(voidptr variable);

/**
 * Frees the error message of result variables if they have any allocated error
 * message string
 *
 * @return nothing or error
 *
 * NOTE: Only proper result type based pointers must be sent otherwise undefined
 * behavior might happen
 *
 * NOTE: Checkout aliases.h if you want to create your own compatible type
 */
DCResVoid dc_res_free(voidptr res_ptr);

// ***************************************************************************************
// * Files
// ***************************************************************************************

/**
 * Tries to open a file and return proper fileptr or error code with error
 * message
 */
DCResFileptr dc_file_open(const string file, const string mode);

// ***************************************************************************************
// * ERROR LOGS
// ***************************************************************************************

/**
 * Writes date and time with given format to given stream or stdout
 */
void dc_fprintf_datetime(fileptr stream, const string format);

/**
 * Writes date and time with the default format of "%Y-%m-%d %H:%M:%S" to the
 * stream
 */
void dc_now(fileptr stream);

/**
 * Writes time with the default format of "%H:%M:%S" to the stream
 */
void dc_time(fileptr stream);

/**
 * Writes date with the default format of "%Y-%m-%d" to the stream
 */
void dc_date(fileptr stream);

/**
 * It will open the give filename or `_error_logs.log` if file name is NULL and
 * initialize global `dc_error_logs` with it, so all the logs (normal or debugs)
 * will be written to the file instead of stderr
 *
 * @return nothing or exit with proper error code
 */
void dc_error_logs_init(string filename, bool append);

/**
 * Closes `dc_error_logs` if it's already opened
 *
 * NOTE: Only cleanup pool (DC_CLEANUP_POOL mode) automatically does this
 * otherwise you must do this manually
 */
void dc_error_logs_close();

// ***************************************************************************************
// * IMPLEMENTATIONS
// ***************************************************************************************

#ifdef DCOMMON_IMPL

/**
 * File pointer for error logging, in case of null stderr will be used
 */
FILE* dc_error_logs = NULL;

/**
 * Global cleanup pool
 */
DCCleanupPool dc_cleanup_pool = {0};

#include "_dynarr.c"
#include "_dynht.c"
#include "_lit_val.c"
#include "_string_view.c"
#include "_utils.c"

#else

/**
 * File pointer for error logging, in case of null stderr will be used
 */
extern FILE* dc_error_logs;

/**
 * Global cleanup pool
 */
extern DCCleanupPool dc_cleanup_pool;

#endif

#endif // DC_MAIN_HEADER_H
