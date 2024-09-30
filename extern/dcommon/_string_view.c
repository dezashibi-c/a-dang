// ***************************************************************************************
//    Project: dcommon -> https://github.com/dezashibi-c/dcommon
//    File: _string_view.c
//    Date: 2024-09-12
//    Author: Navid Dezashibi
//    Contact: navid@dezashibi.com
//    Website: https://dezashibi.com | https://github.com/dezashibi
//    License:
//     Please refer to the LICENSE file, repository or website for more
//     information about the licensing of this work. If you have any questions
//     or concerns, please feel free to contact me at the email address provided
//     above.
// ***************************************************************************************
// *  Description: private implementation file for definition of common helper
// *               functions
// *               DO NOT LINK TO THIS DIRECTLY
// ***************************************************************************************

#ifndef __DC_BYPASS_PRIVATE_PROTECTION
#error "You cannot link to this source (_string_view.c) directly, please consider including dcommon.h"
#endif

#include "_headers/aliases.h"
#include "_headers/general.h"
#include "_headers/macros.h"

DCResultSv dc_sv_create(string base, usize start, usize length)
{
    DC_RES_sv();

    if (!base)
    {
        dc_dbg_log("got NULL base string");

        dc_res_ret_e(1, "got NULL base string");
    }

    DCStringView view;
    view.cstr = NULL;
    view.str = base + start;
    view.len = length;

    dc_res_ret_ok(view);
}

DCResultString dc_sv_as_cstr(DCStringView* sv)
{
    DC_RES_string();

    if (!sv)
    {
        dc_dbg_log("got NULL DCStringView");

        dc_res_ret_e(1, "got NULL DCStringView");
    }

    if (sv->cstr != NULL) dc_res_ret_ok(sv->cstr);

    sv->cstr = (string)malloc(sv->len + 1);
    if (sv->cstr == NULL)
    {
        dc_dbg_log("Memory allocation failed");

        dc_res_ret_e(2, "Memory allocation failed");
    }

    strncpy(sv->cstr, sv->str, sv->len);

    sv->cstr[sv->len] = '\0';

    dc_res_ret_ok(sv->cstr);
}

DCResultVoid dc_sv_free(DCStringView* sv)
{
    DC_RES_void();

    if (!sv || sv->cstr == NULL) dc_res_ret();

    free(sv->cstr);
    sv->cstr = NULL;

    dc_res_ret();
}
