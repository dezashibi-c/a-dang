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

void dc_dynarr_init(DCDynArr* darr);
void ___dc_dynarr_init_with_values(DCDynArr* darr, usize count,
                                   DCDynValue values[]);
void dc_dynarr_add(DCDynArr* darr, DCDynValue value);
DCDynValue* dc_dynarr_find(DCDynArr* darr, DCDynValue* el);
void dc_dynarr_value_free(DCDynValue* element,
                          void (*custom_free)(DCDynValue*));
void dc_dynarr_free(DCDynArr* darr, void (*custom_free)(DCDynValue*));
void dc_dynarr_delete(DCDynArr* darr, usize index,
                      void (*custom_free)(DCDynValue*));

___dc_dynval_converters_decl(u8);
___dc_dynval_converters_decl(i32);
___dc_dynval_converters_decl(u32);
___dc_dynval_converters_decl(u64);
___dc_dynval_converters_decl(f32);
___dc_dynval_converters_decl(f64);
___dc_dynval_converters_decl(uptr);
___dc_dynval_converters_decl(byte);
___dc_dynval_converters_decl(size);
___dc_dynval_converters_decl(usize);
___dc_dynval_converters_decl(string);
___dc_dynval_converters_decl(voidptr);

DCStringView dc_sv_create(string base, usize start, usize length);
string dc_sv_as_cstr(DCStringView* sv);
void dc_sv_free(DCStringView* sv);

int dc_sprintf(string* str, string fmt, ...) ATTRIB((format(printf, 2, 3)));
string dc_strdup(const string in);
void dc_normalize_path_to_posix(string path);
string dc_replace_file_in_path(const string path, const string new_file);

// ***************************************************************************************
// * IMPLEMENTATIONS
// ***************************************************************************************

#ifdef DCOMMON_IMPL

FILE* dc_error_logs = NULL;
DC_ERROR_MODE dc_error_mode = DC_ERR_MODE_NORMAL;

#include "_dynarr.c"
#include "_string_view.c"
#include "_utils.c"

#else

extern FILE* dc_error_logs;
extern DC_ERROR_MODE dc_error_mode;

#endif

#endif // DC_MAIN_HEADER_H
