// ***************************************************************************************
//    Project: dcommon -> https://github.com/dezashibi-c/dcommon
//    File: _utils.c
//    Date: 2024-09-13
//    Author: Navid Dezashibi
//    Contact: navid@dezashibi.com
//    Website: https://dezashibi.com | https://github.com/dezashibi
//    License:
//     Please refer to the LICENSE file, repository or website for more
//     information about the licensing of this work. If you have any questions
//     or concerns, please feel free to contact me at the email address provided
//     above.
//    ATTRIBUTIONS:
//     dc_sprintf and dc_strdup are based on source codes from the book
//     "21st-Century C" by Ben Klemens
// ***************************************************************************************
// *  Description: private implementation file for definition of common helper
// *               functions
// *               DO NOT LINK TO THIS DIRECTLY
// ***************************************************************************************

#ifndef __DC_BYPASS_PRIVATE_PROTECTION
#error                                                                         \
    "You cannot link to this source (_utils.c) directly, please consider including dcommon.h"
#endif

#include "_headers/aliases.h"
#include "_headers/general.h"
#include "_headers/macros.h"

int dc_sprintf(string* str, string fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);

    byte one_char[1];
    int len = vsnprintf(one_char, 1, fmt, argp);
    if (len < 1)
    {
        fprintf(
            stderr,
            "An encoding error occurred. Setting the input pointer to NULL.\n");
        *str = NULL;
        va_end(argp);
        return len;
    }
    va_end(argp);

    *str = malloc(len + 1);
    if (!str)
    {
        fprintf(stderr, "Couldn't allocate %i bytes.\n", len + 1);
        return -1;
    }

    va_start(argp, fmt);
    vsnprintf(*str, len + 1, fmt, argp);
    va_end(argp);

    return len;
}

string dc_strdup(const string in)
{
    if (!in) return NULL;

    string out;

    dc_sprintf(&out, "%s", in);

    return out;
}

void dc_normalize_path_to_posix(string path)
{
#if defined(_WIN32) || defined(_WIN64)
    for (string p = path; *p; ++p)
    {
        if (*p == '\\')
        {
            *p = '/';
        }
    }
#endif
}

string dc_replace_file_in_path(string path, const string new_file)
{
    dc_normalize_path_to_posix(path);
    const string last_sep = strrchr(path, '/');

    size_t dir_length = (last_sep != NULL) ? (last_sep - path + 1) : 0;

    size_t new_path_length = dir_length + strlen(new_file) + 1;
    string new_path = (string)malloc(new_path_length);

    if (new_path == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    if (dir_length > 0)
    {
        strncpy(new_path, path, dir_length);
    }

    strcpy(new_path + dir_length, new_file);

    return new_path;
}
