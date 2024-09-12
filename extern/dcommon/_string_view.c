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
#error                                                                         \
    "You cannot link to this source (_string_view.c) directly, please consider including dcommon.h"
#endif

#include "_headers/aliases.h"
#include "_headers/general.h"
#include "_headers/macros.h"

DCStringView dc_sv_create(string base, size start, size length)
{
    DCStringView view;
    view.cstr = NULL;
    view.str = base + start;
    view.len = length;

    return view;
}

string dc_sv_as_cstr(DCStringView* sv)
{
    if (sv->cstr != NULL) return sv->cstr;

    sv->cstr = (string)malloc(sv->len + 1);
    if (sv->cstr == NULL) return NULL;

    strncpy(sv->cstr, sv->str, sv->len);

    sv->cstr[sv->len] = '\0';

    return sv->cstr;
}

void dc_sv_free(DCStringView* sv)
{
    if (sv->cstr == NULL) return;

    free(sv->cstr);
    sv->cstr = NULL;
}
