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

#include "dcommon.h"

#if defined(DC_WINDOWS)
#include <Lmcons.h>
#include <windows.h>
#else
#include <pwd.h>
#include <unistd.h>
#endif

// ***************************************************************************************
// * STRINGS AND PATHS
// ***************************************************************************************

int dc_sprintf(string* str, string fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);

    char one_char[1];
    int len = vsnprintf(one_char, 1, fmt, argp);
    if (len < 1)
    {
        dc_log(
            "An encoding error occurred. Setting the input pointer to NULL.\n");
        *str = NULL;
        va_end(argp);
        return len;
    }
    va_end(argp);

    *str = malloc(len + 1);
    if (!str)
    {
        dc_log("Couldn't allocate %i chars.\n", len + 1);
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
#if defined(DC_WINDOWS)
    for (string p = path; *p; ++p)
    {
        if (*p == '\\')
        {
            *p = '/';
        }
    }
#else
    (void)path;
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
        dc_log("Memory allocation failed\n");
        return NULL;
    }

    if (dir_length > 0)
    {
        strncpy(new_path, path, dir_length);
    }

    strcpy(new_path + dir_length, new_file);

    return new_path;
}

string dc_get_home_dir_path()
{
#if defined(DC_WINDOWS)
    static char homeDir[MAX_PATH];
    if (GetEnvironmentVariable("USERPROFILE", homeDir, MAX_PATH))
    {
        return homeDir;
    }
    else
    {
        return NULL;
    }
#else
    return getenv("HOME");
#endif
}

string dc_get_username()
{
#if defined(DC_WINDOWS)
    static char username[UNLEN + 1];
    DWORD username_len = UNLEN + 1;
    if (GetUserName(username, &username_len))
    {
        return username;
    }
    else
    {
        return NULL;
    }
#else
    return getenv("USER");
#endif
}

string dc_get_os()
{
    string os = "unknown";

#ifdef DC_WINDOWS
    os = "windows";
#elif defined(DC_LINUX)
    os = "linux";
#elif defined(DC_ANDROID)
    os = "android";
#elif defined(DC_MACOS)
    os = "macos";
#elif defined(DC_UNIX)
    os = "unix";
#endif

    return os;
}

string dc_get_arch()
{
    string arch = "unknown";

#ifdef DC_WINDOWS_X64
    arch = "x64";
#elif defined(DC_WINDOWS_ARM64)
    arch = "arm64";
#elif defined(DC_WINDOWS_ARM)
    arch = "arm";
#elif defined(DC_WINDOWS_X86)
    arch = "x86";
#elif defined(DC_LINUX_X64)
    arch = "x64";
#elif defined(DC_LINUX_ARM64)
    arch = "arm64";
#elif defined(DC_LINUX_ARM)
    arch = "arm";
#elif defined(DC_LINUX_X86)
    arch = "x86";
#elif defined(DC_ANDROID_X64)
    arch = "x64";
#elif defined(DC_ANDROID_ARM64)
    arch = "arm64";
#elif defined(DC_ANDROID_ARM)
    arch = "arm";
#elif defined(DC_ANDROID_X86)
    arch = "x86";
#elif defined(DC_MACOS_X64)
    arch = "x64";
#elif defined(DC_MACOS_X86)
    arch = "x86";
#elif defined(DC_UNIX_X64)
    arch = "x64";
#elif defined(DC_UNIX_ARM64)
    arch = "arm64";
#elif defined(DC_UNIX_ARM)
    arch = "arm";
#elif defined(DC_UNIX_X86)
    arch = "x86";
#endif

    return arch;
}

// ***************************************************************************************
// * CLEANUP
// ***************************************************************************************

void ___dc_handle_signal(int sig)
{
    dc_dbg_log("caught signal %d.", sig);
    exit(sig);
}

void ___dc_perform_cleanup(DCCleanups* cleanups_arr)
{
    dc_dbg_log_if(cleanups_arr->cap == 0 || cleanups_arr->count == 0,
                  "cleanups_arr is not initialized or has no "
                  "elements registered, exiting now...");

    if (cleanups_arr->cap == 0 || cleanups_arr->count == 0) return;

    dc_dbg_log("cleaning up %zu elements", cleanups_arr->count);

    // run cleanup of each item
    dc_da_for(*cleanups_arr)
    {
        DCCleanupEntry* entry = dc_da_get_as(cleanups_arr, _idx, voidptr);

        dc_dbg_log("cleaning index: %zu, cleanup perform: %p", _idx,
                   (*entry).element);

        dc_cleanup_do(*entry);
    }

    // clean up the dc_cleanup itself
    dc_da_free(cleanups_arr);
}

void ___dc_perform_global_cleanup(void)
{
    dc_dbg_log("performing global cleanups");
    ___dc_perform_cleanup(&dc_cleanups);
}

void ____dc_cleanups_custom_push(DCCleanups* cleanup_arr, voidptr element,
                                 DCCleanupFunc cleanup_func)
{
    if (!element || !cleanup_func)
    {
        dc_log("got null element or cleanup_func");

        exit(1);
    }

    ___dc_cleanups_arr_init(*cleanup_arr, 10);

    DCCleanupEntry* item = malloc(sizeof(DCCleanupEntry));
    if (!item)
    {
        dc_log("Memory allocation failed");
        exit(1);
    }

    dc_dbg_log("cleanup push: %p", element);

    item->element = element;
    item->cleanup_func = cleanup_func;

    dc_da_push(cleanup_arr, dc_dv(voidptr, item));
}
