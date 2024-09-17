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
// *  Description: Main `dcommon` library entry point
// ***************************************************************************************

#ifndef DC_MAIN_HEADER_H
#define DC_MAIN_HEADER_H

#define __DC_BYPASS_PRIVATE_PROTECTION

#include "_headers/general.h"

#include "_headers/aliases.h"

#include "_headers/macros.h"

// ***************************************************************************************
// * FUNCTION DECLARATIONS
// ***************************************************************************************

void dc_dynarr_init(DCDynArr* darr, DCDynValFreeFunc element_free_func);
DCDynArr* dc_dynarr_create(DCDynValFreeFunc element_free_func);
void ___dc_dynarr_init_with_values(DCDynArr* darr, usize count,
                                   DCDynValFreeFunc element_free_func,
                                   DCDynValue values[]);
void dc_dynarr_grow(DCDynArr* darr);
void dc_dynarr_grow_by(DCDynArr* darr, usize amount);
void dc_dynarr_grow_to(DCDynArr* darr, usize amount);
void dc_dynarr_trunc(DCDynArr* darr);
void dc_dynarr_pop(DCDynArr* darr, usize count, DCDynValue** out_popped);
void dc_dynarr_push(DCDynArr* darr, DCDynValue value);
void ___dc_dynarr_append_values(DCDynArr* darr, usize count,
                                DCDynValue values[]);
void dc_dynarr_append(DCDynArr* darr, DCDynArr* from);
DCDynValue* dc_dynarr_get(DCDynArr* darr, usize index);
DCDynValue* dc_dynarr_find(DCDynArr* darr, DCDynValue* el);
void dc_dynval_free(DCDynValue* element, void (*custom_free)(DCDynValue*));
void dc_dynarr_free(DCDynArr* darr);
bool dc_dynarr_delete(DCDynArr* darr, usize index);
void dc_dynarr_insert(DCDynArr* darr, usize index, DCDynValue value);
void ___dc_dynarr_insert_values(DCDynArr* darr, usize start_index, usize count,
                                DCDynValue values[]);
void dc_dynarr_insert_from(DCDynArr* darr, usize start_index, DCDynArr* from);

___dc_dynarr_converters_decl(u8);
___dc_dynarr_converters_decl(i32);
___dc_dynarr_converters_decl(u32);
___dc_dynarr_converters_decl(u64);
___dc_dynarr_converters_decl(f32);
___dc_dynarr_converters_decl(f64);
___dc_dynarr_converters_decl(uptr);
___dc_dynarr_converters_decl(char);
___dc_dynarr_converters_decl(size);
___dc_dynarr_converters_decl(usize);
___dc_dynarr_converters_decl(string);
___dc_dynarr_converters_decl(voidptr);

void dc_ht_init(DCHashTable* ht, usize capacity, DCHashFunc hash_func,
                DCKeyCompFunc key_cmp_func, DCDynValFreeFunc element_free_func);
DCHashTable* dc_ht_create(usize capacity, DCHashFunc hash_func,
                          DCKeyCompFunc key_cmp_func,
                          DCDynValFreeFunc element_free_func);
void dc_ht_free(DCHashTable* ht);
usize dc_ht_find_by_key(DCHashTable* ht, voidptr key, DCDynValue** out_result);
void dc_ht_set(DCHashTable* ht, voidptr key, DCDynValue value);
void ___dc_ht_set_multiple(DCHashTable* ht, usize count, DCHashEntry entries[]);
void dc_ht_merge(DCHashTable* ht, DCHashTable* from);
bool dc_ht_delete(DCHashTable* ht, voidptr key);
usize dc_ht_keys(DCHashTable* ht, voidptr** out_arr);

DCStringView dc_sv_create(string base, usize start, usize length);
string dc_sv_as_cstr(DCStringView* sv);
void dc_sv_free(DCStringView* sv);

int dc_sprintf(string* str, string fmt, ...)
    __dc_attribute((format(printf, 2, 3)));
string dc_strdup(const string in);
void dc_normalize_path_to_posix(string path);
string dc_replace_file_in_path(const string path, const string new_file);
string dc_get_home_dir_path();
string dc_get_username();

// ***************************************************************************************
// * IMPLEMENTATIONS
// ***************************************************************************************

#ifdef DCOMMON_IMPL

FILE* dc_error_logs = NULL;
DC_ERROR_MODE dc_error_mode = DC_ERR_MODE_NORMAL;

#include "_dynarr.c"
#include "_dynht.c"
#include "_string_view.c"
#include "_utils.c"

#else

extern FILE* dc_error_logs;
extern DC_ERROR_MODE dc_error_mode;

#endif

#endif // DC_MAIN_HEADER_H
