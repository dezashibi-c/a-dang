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

void dc_da_init(DCDynArr* darr, DCDynValFreeFunc element_free_func);
DCDynArr* dc_da_create(DCDynValFreeFunc element_free_func);
void __dc_da_init_with_values(DCDynArr* darr, usize count,
                              DCDynValFreeFunc element_free_func,
                              DCDynValue values[]);
void dc_da_grow(DCDynArr* darr);
void dc_da_grow_by(DCDynArr* darr, usize amount);
void dc_da_grow_to(DCDynArr* darr, usize amount);
void dc_da_trunc(DCDynArr* darr);
void dc_da_pop(DCDynArr* darr, usize count, DCDynValue** out_popped,
               bool truncate);
void dc_da_push(DCDynArr* darr, DCDynValue value);
void __dc_da_append_values(DCDynArr* darr, usize count, DCDynValue values[]);
void dc_da_append(DCDynArr* darr, DCDynArr* from);
DCDynValue* dc_da_get(DCDynArr* darr, usize index);
DCDynValue* dc_da_find(DCDynArr* darr, DCDynValue* el);
void dc_dv_free(DCDynValue* element, void (*custom_free)(DCDynValue*));
void dc_dv_free__(voidptr item);
void dc_da_free(DCDynArr* darr);
void dc_da_free__(voidptr darr);
bool dc_da_delete(DCDynArr* darr, usize index);
void dc_da_insert(DCDynArr* darr, usize index, DCDynValue value);
void __dc_da_insert_values(DCDynArr* darr, usize start_index, usize count,
                           DCDynValue values[]);
void dc_da_insert_from(DCDynArr* darr, usize start_index, DCDynArr* from);

__dc_da_converters_decl(u8);
__dc_da_converters_decl(i32);
__dc_da_converters_decl(u32);
__dc_da_converters_decl(u64);
__dc_da_converters_decl(f32);
__dc_da_converters_decl(f64);
__dc_da_converters_decl(uptr);
__dc_da_converters_decl(char);
__dc_da_converters_decl(size);
__dc_da_converters_decl(usize);
__dc_da_converters_decl(string);
__dc_da_converters_decl(voidptr);

void dc_ht_init(DCHashTable* ht, usize capacity, DCHashFunc hash_func,
                DCKeyCompFunc key_cmp_func, DCDynValFreeFunc element_free_func);
DCHashTable* dc_ht_create(usize capacity, DCHashFunc hash_func,
                          DCKeyCompFunc key_cmp_func,
                          DCDynValFreeFunc element_free_func);
void dc_ht_free(DCHashTable* ht);
void dc_ht_free__(voidptr ht);
usize dc_ht_find_by_key(DCHashTable* ht, voidptr key, DCDynValue** out_result);
void dc_ht_set(DCHashTable* ht, voidptr key, DCDynValue value);
void __dc_ht_set_multiple(DCHashTable* ht, usize count, DCHashEntry entries[]);
void dc_ht_merge(DCHashTable* ht, DCHashTable* from);
bool dc_ht_delete(DCHashTable* ht, voidptr key);
usize dc_ht_keys(DCHashTable* ht, voidptr** out_arr);

DCStringView dc_sv_create(string base, usize start, usize length);
string dc_sv_as_cstr(DCStringView* sv);
void dc_sv_free(DCStringView* sv);

int dc_sprintf(string* str, string fmt, ...)
    __dc_attribute((format(printf, 2, 3)));
int dc_sappend(string* str, const string fmt, ...)
    __dc_attribute((format(printf, 2, 3)));
string dc_strdup(const string in);
void dc_normalize_path_to_posix(string path);
string dc_replace_file_in_path(const string path, const string new_file);
string dc_get_home_dir_path();
string dc_get_username();
string dc_get_os();
string dc_get_arch();

void __dc_handle_signal(int sig);
void __dc_perform_cleanup(DCCleanups* cleanups_arr);
void dc_perform_cleanup(void);
void __dc_cleanups_custom_push(DCCleanups* cleanup_arr, voidptr element,
                               DCCleanupFunc cleanup_func);

// ***************************************************************************************
// * IMPLEMENTATIONS
// ***************************************************************************************

#ifdef DCOMMON_IMPL

FILE* dc_error_logs = NULL;
DC_ERROR_MODE dc_error_mode = DC_ERR_MODE_NORMAL;
DCCleanups dc_cleanups = {0};

#include "_dynarr.c"
#include "_dynht.c"
#include "_string_view.c"
#include "_utils.c"

#else

extern FILE* dc_error_logs;
extern DC_ERROR_MODE dc_error_mode;
extern DCCleanups dc_cleanups;

#endif

#endif // DC_MAIN_HEADER_H
