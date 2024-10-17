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
#error "You cannot link to this source (_utils.c) directly, please consider including dcommon.h"
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

DCResUsize dc_sprintf(string* str, string fmt, ...)
{
    DC_RES_usize();

    if (!str)
    {
        dc_dbg_log("got NULL str");

        dc_res_ret_e(1, "got NULL str");
    }

    va_list argp;
    va_start(argp, fmt);

    char one_char[1];
    int len = vsnprintf(one_char, 1, fmt, argp);
    if (len < 1)
    {
        dc_dbg_log("An encoding error occurred. Setting the input pointer to NULL.");

        *str = NULL;
        va_end(argp);

        dc_res_ret_e(5, "An encoding error occurred. Setting the input pointer to NULL.");
    }
    va_end(argp);

    *str = malloc(len + 1);
    if (!str)
    {
        dc_dbg_log("Couldn't allocate %i chars.", len + 1);

        dc_res_ret_e(1, "Memory allocation failed");
    }

    va_start(argp, fmt);
    vsnprintf(*str, len + 1, fmt, argp);
    va_end(argp);

    dc_res_ret_ok((usize)len);
}

DCResUsize dc_sappend(string* str, const string fmt, ...)
{
    DC_RES_usize();

    if (!str)
    {
        dc_dbg_log("got NULL str");

        dc_res_ret_e(1, "got NULL str");
    }

    va_list argp;
    va_start(argp, fmt);

    // Calculate the length of the formatted string
    char one_char[1];
    int len = vsnprintf(one_char, 1, fmt, argp);
    if (len < 0)
    {
        dc_dbg_log("An encoding error occurred. Setting the input pointer to NULL.");

        *str = NULL;
        va_end(argp);

        dc_res_ret_e(5, "An encoding error occurred. Setting the input pointer to NULL.");
    }
    va_end(argp);

    // Calculate the current length of the existing string
    size_t current_len = *str ? strlen(*str) : 0;

    // Allocate memory for the new string (old length + new formatted part)
    string new_str = realloc(*str, current_len + len + 1);
    if (!new_str)
    {
        dc_dbg_log("Couldn't allocate %i chars.", len + 1);

        dc_res_ret_e(1, "Memory allocation failed");
    }

    // Update the string pointer
    *str = new_str;

    // Append the new formatted string to the existing string
    va_start(argp, fmt);
    vsnprintf(*str + current_len, len + 1, fmt, argp);
    va_end(argp);

    dc_res_ret_ok(current_len + len);
}

DCResString dc_strdup(const string in)
{
    DC_RES_string();

    if (!in)
    {
        dc_dbg_log("got NULL input str");

        dc_res_ret_e(1, "got NULL input str");
    }

    string out;

    dc_try_fail_temp(DCResUsize, dc_sprintf(&out, "%s", in));

    dc_res_ret_ok(out);
}

DCResString dc_tostr_dv(DCDynVal* dv)
{
    DC_RES_string();

    if (!dv) dc_res_ret_e(1, "got NULL dynamic value");

#define stringify(TYPE)                                                                                                        \
    case dc_dvt(TYPE):                                                                                                         \
        dc_sprintf(&result, dc_dv_fmt(dv), dc_dv_as(*dv, TYPE));                                                               \
        break

    string result = NULL;

    switch (dv->type)
    {
        stringify(u8);
        stringify(u16);
        stringify(u32);
        stringify(u64);
        stringify(i8);
        stringify(i16);
        stringify(i32);
        stringify(i64);
        stringify(f32);
        stringify(f64);
        stringify(uptr);
        stringify(char);
        stringify(string);
        stringify(voidptr);
        stringify(fileptr);
        stringify(size);
        stringify(usize);

        case dc_dvt(DCStringView):
            dc_sprintf(&result, DCPRIsv, dc_sv_fmt(dc_dv_as(*dv, DCStringView)));
            break;

        default:
            dc_sprintf(&result, "%s", "(unknown dynamic value)");
            break;
    };

    dc_res_ret_ok(result);

#undef stringify
}

DCResVoid dc_dv_print(DCDynVal* dv)
{
    DC_RES_void();

    dc_try_or_fail_with3(DCResString, res, dc_tostr_dv(dv), {});

    printf("%s", dc_res_val2(res));

    if (dc_res_val2(res)) free(dc_res_val2(res));

    dc_res_ret();
}

DCResVoid dc_dv_println(DCDynVal* dv)
{
    DC_TRY_DEF2(DCResVoid, dc_dv_print(dv));

    printf("%s", "\n");

    dc_res_ret();
}

DCResVoid dc_normalize_path_to_posix(string path)
{
    DC_RES_void();

    if (!path)
    {
        dc_dbg_log("got NULL path");

        dc_res_ret_e(1, "got NULL path");
    }

#if defined(DC_WINDOWS)
    for (string p = path; *p; ++p)
    {
        if (*p == '\\')
        {
            *p = '/';
        }
    }
#endif

    dc_res_ret();
}

DCResString dc_replace_file_in_path(string path, const string file_name)
{
    DC_RES_string();

    if (!path || !file_name)
    {
        dc_dbg_log("got NULL path or file_name");

        dc_res_ret_e(1, "got NULL path or file_name");
    }

    dc_try_fail_temp(DCResVoid, dc_normalize_path_to_posix(path));
    const string last_sep = strrchr(path, '/');

    size_t dir_length = (last_sep != NULL) ? (last_sep - path + 1) : 0;

    size_t new_path_length = dir_length + strlen(file_name) + 1;
    string new_path = (string)malloc(new_path_length);

    if (new_path == NULL)
    {
        dc_dbg_log("Memory allocation failed");

        dc_res_ret_e(1, "Memory allocation failed");
    }

    if (dir_length > 0) strncpy(new_path, path, dir_length);

    strcpy(new_path + dir_length, file_name);

    dc_res_ret_ok(new_path);
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

void __dc_handle_signal(int sig)
{
    switch (sig)
    {
        case SIGINT:
            dc_log("Caught SIGINT (Ctrl+C)");
            break;

        case SIGTERM:
            dc_log("Caught SIGTERM");
            break;

        case SIGSEGV:
            dc_log("Caught SIGSEGV (Segmentation Fault)");
            break;

        default:
            dc_log("Caught other signals: %d", sig);
            break;
    };

    if (sig != SIGSEGV)
        dc_cleanup_pool_run(DC_CLEANUP_POOL);
    else
        dc_error_logs_close();

    exit(sig);
}

void dc_cleanup_pool_init2(usize count, usize batch_capacity)
{
    if (dc_cleanup_pool.count > 0)
    {
        dc_dbg_log("cannot initialize already initialized cleanup pool, got= "
                   "%" PRIuMAX,
                   dc_cleanup_pool.count);

        return;
    }

    if (count == 0)
    {
        dc_dbg_log("cannot initialize cleanup pool to zero count of batches");

        exit(1);
    }

    if (batch_capacity == 0)
    {
        dc_dbg_log("initializing batch capacity to zero is not a good idea, "
                   "fall-back to default=%d",
                   DC_DA_INITIAL_CAP);

        batch_capacity = DC_DA_INITIAL_CAP;
    }

    dc_cleanup_pool.count = count;

    dc_cleanup_pool.pool = (DCCleanupBatch*)calloc(count, sizeof(DCCleanupBatch));
    if (!dc_cleanup_pool.pool)
    {
        dc_dbg_log("Memory allocation failed");

        exit(2);
    }

    DCResVoid res;

    for (usize i = 0; i < count; ++i)
    {
        res = dc_da_init2(&dc_cleanup_pool.pool[i], batch_capacity, 3, NULL);

        dc_dbg_action_on(dc_res_is_err2(res), exit(dc_res_err_code2(res)), "%s", dc_res_err_msg2(res));
    }

    __DC_CLEANUP_REGISTER_SIGNALS;

    dc_res_free(&res);
}

DCResVoid dc_cleanup_batch_run(DCCleanupBatch* batch)
{
    DC_RES_void();

    dc_dbg_log_if(batch->cap == 0 || batch->count == 0, "batch is not initialized or has no "
                                                        "elements registered, exiting now...");

    if (batch->cap == 0 || batch->count == 0) dc_res_ret();

    dc_dbg_log("cleaning up '%" PRIuMAX "' elements", batch->count);

    // run cleanup of each item
    dc_da_for(*batch, {
        DCCleanupJob* key_value = dc_da_get_as(*batch, _idx, voidptr);

        dc_dbg_log("cleaning index: '%" PRIuMAX "', cleanup perform: %p", _idx, (*key_value).element);

        dc_try_fail(dc_cleanup_job_run(*key_value));
    });

    // clean up the dc_cleanup itself
    dc_dbg_log("freeing cleanup batch, current capacity: '%" PRIuMAX "', current "
               "count: '%" PRIuMAX "'",
               batch->cap, batch->count);

    dc_try_fail(dc_da_free(batch));

    dc_dbg_log("cleanup batch has been freed, current capacity: '%" PRIuMAX "', current "
               "count: '%" PRIuMAX "'",
               batch->cap, batch->count);

    dc_res_ret();
}

void dc_cleanup_pool_run(i32 selection)
{
    DC_RES_void();

    switch (selection)
    {
        case DC_NO_CLEANUP:
            break;

        case DC_CLEANUP_POOL:
        {
            if (dc_cleanup_pool.count == 0) return;

            dc_dbg_log("performing pool cleanup");

            dc_for(dc_cleanup_pool.pool, DCCleanupBatch, dc_cleanup_pool.count, {
                dc_dbg_log("cleaning up the batch index [%" PRIuMAX "]", _idx);
                dc_try(dc_cleanup_batch_run(&dc_cleanup_pool.pool[_idx]));

                if (dc_res_is_err())
                {
                    dc_dbg_log("An error occurred while cleanup batch index [%" PRIuMAX "]: (code %d) %s", _idx,
                               dc_res_err_code(), dc_res_err_msg());

                    exit(dc_res_err_code());
                }
            });

            if (dc_cleanup_pool.pool)
            {
                free(dc_cleanup_pool.pool);

                dc_dbg_log("cleanup pool has been freed");
            }

            dc_error_logs_close();

            break;
        }

        default:
        {
            if ((usize)selection >= dc_cleanup_pool.count)
            {
                dc_dbg_log("Cleanup batch index out of bound: requested=%" PRId32 " got=%" PRIuMAX " elements", selection,
                           dc_cleanup_pool.count);

                exit(5);
            }

            dc_dbg_log("cleaning up the batch index [%" PRId32 "]", selection);
            dc_try(dc_cleanup_batch_run(&dc_cleanup_pool.pool[selection]));

            if (dc_res_is_err())
            {
                dc_dbg_log("An error occurred while cleanup batch index [%d]: "
                           "(code %d) %s",
                           selection, dc_res_err_code(), dc_res_err_msg());

                exit(dc_res_err_code());
            }

            break;
        }
    }
}

void dc_cleanup_push2(DCCleanupBatch* batch, voidptr element, DCCleanupFn cleanup_fn)
{
    DC_RES_void();

    if (!element || !cleanup_fn)
    {
        dc_dbg_log("got null element or cleanup_fn");

        exit(1);
    }

    DC_CLEANUP_BATCH_INIT(*batch, 10);

    DCCleanupJob* item = malloc(sizeof(DCCleanupJob));
    if (!item)
    {
        dc_dbg_log("Memory allocation failed");

        exit(2);
    }

    dc_dbg_log("cleanup push: %p", element);

    item->element = element;
    item->cleanup_fn = cleanup_fn;

    dc_try(dc_da_push(batch, dc_dva(voidptr, item)));

    if (dc_res_is_err())
    {
        dc_dbg_log("An error occurred while pushing cleanup job key_value to the "
                   "cleanup batch index: (code %d) %s",
                   dc_res_err_code(), dc_res_err_msg());

        exit(dc_res_err_code());
    }
}

DCResVoid dc_free(voidptr variable)
{
    DC_RES_void();

    if (!variable) dc_res_ret_e(1, "try to free NULL");

    free(variable);

    dc_res_ret();
}

DCResVoid dc_free_file(voidptr variable)
{
    DC_RES_void();

    if (!variable) dc_res_ret_e(1, "try to free NULL");

    fclose((fileptr)variable);

    dc_res_ret();
}

// ***************************************************************************************
// * RESULT
// ***************************************************************************************

DCResVoid dc_res_free(voidptr res_ptr)
{
    DC_RES_void();

    if (!res_ptr) dc_res_ret_e(1, "got NULL result");

    DCResVoid* result = (DCResVoid*)res_ptr;


    if (dc_res_is_err2(*result) && result->data.e.allocated) free(result->data.e.message);

    result->data.e.message = "";
    result->status = DC_RES_OK;

    dc_res_ret();
}

// ***************************************************************************************
// * Files
// ***************************************************************************************

DCResFileptr dc_file_open(const string file, const string mode)
{
    DC_RES_fileptr();

    fileptr fp = fopen(file, mode);

    if (fp == NULL)
    {
        dc_dbg_log("Cannot open file '%s': (code %d) %s", file, errno, strerror(errno));

        dc_res_ret_ea(errno, "%s", strerror(errno));
    }

    dc_res_ret_ok(fp);
}

// ***************************************************************************************
// * ERROR LOGS
// ***************************************************************************************

void dc_fprintf_datetime(fileptr stream, const string format)
{
    time_t raw_time;
    struct tm* time_info;
    char buffer[30];

    time(&raw_time);

    time_info = localtime(&raw_time);

    strftime(buffer, sizeof(buffer), format, time_info);

    fprintf(stream ? stream : stdout, "%s", buffer);
}

void dc_now(fileptr stream)
{
    dc_fprintf_datetime(stream, "%Y-%m-%d %H:%M:%S");
}

void dc_time(fileptr stream)
{
    dc_fprintf_datetime(stream, "%H:%M:%S");
}

void dc_date(fileptr stream)
{
    dc_fprintf_datetime(stream, "%Y-%m-%d");
}

void dc_error_logs_init(string filename, bool append)
{
    DC_RES_fileptr();

    string _filename;

    dc_sprintf(&_filename, "%s", (filename == NULL ? "_error_logs.log" : filename));

    const string mode = append ? "a" : "w";

    dc_try(dc_file_open(_filename, mode));
    free(_filename);

    if (dc_res_is_err())
    {
        dc_dbg_log("cannot initialize 'dc_error_logs': %s", dc_res_err_msg());

        exit(dc_res_err_code());
    }

    dc_error_logs = dc_res_val();
}

void dc_error_logs_close()
{
    if (dc_error_logs)
    {
        dc_dbg_log("Closing dc_error_logs");
        fclose(dc_error_logs);
        dc_error_logs = NULL;
        dc_dbg_log("dc_error_logs is closed");
    }
}
