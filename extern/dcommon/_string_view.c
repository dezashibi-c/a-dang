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
#include "dcommon_primitives.h"

DCResSv dc_sv_create(string base, usize start, usize length)
{
    DC_RES_sv();

    if (!base)
    {
        dc_dbg_log("got NULL base string");

        dc_ret_e(1, "got NULL base string");
    }

    if (start + length > strlen(base))
    {
        dc_dbg_log("starting at index " dc_fmt(usize) " cannot get " dc_fmt(
                       usize) " characters out of a string with length=" dc_fmt(usize),
                   start, length, strlen(base));

        dc_ret_e(dc_e_code(INDEX), "starting at the given index cannot provide desired sub string with the given length");
    }

    DCStringView view;
    view.cstr = NULL;
    view.str = base + start;
    view.len = length;

    dc_ret_ok(view);
}

DCResString dc_sv_as_cstr(DCStringView* sv)
{
    DC_RES_string();

    if (!sv)
    {
        dc_dbg_log("got NULL DCStringView");

        dc_ret_e(1, "got NULL DCStringView");
    }

    if (sv->cstr != NULL) dc_ret_ok(sv->cstr);

    sv->cstr = (string)malloc(sv->len + 1);
    if (sv->cstr == NULL)
    {
        dc_dbg_log("Memory allocation failed");

        dc_ret_e(2, "Memory allocation failed");
    }

    strncpy(sv->cstr, sv->str, sv->len);

    sv->cstr[sv->len] = '\0';

    dc_ret_ok(sv->cstr);
}

DCResVoid dc_sv_free(DCStringView* sv)
{
    DC_RES_void();

    if (!sv) dc_ret_e(dc_e_code(NV), dc_e_msg(NV));

    if (sv->cstr) free(sv->cstr);

    sv->cstr = NULL;
    sv->str = NULL;
    sv->len = 0;

    dc_ret();
}
