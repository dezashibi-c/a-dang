// ***************************************************************************************
//    Project: dcommon -> https://github.com/dezashibi-c/dcommon
//    File: _lit_val.c
//    Date: 2024-09-21
//    Author: Navid Dezashibi
//    Contact: navid@dezashibi.com
//    Website: https://dezashibi.com | https://github.com/dezashibi
//    License:
//     Please refer to the LICENSE file, repository or website for more
//     information about the licensing of this work. If you have any questions
//     or concerns, please feel free to contact me at the email address provided
//     above.
// ***************************************************************************************
// *  Description: private implementation file for definition of Hash Table
// *               Functionalities
// *               DO NOT LINK TO THIS DIRECTLY
// ***************************************************************************************

#ifndef __DC_BYPASS_PRIVATE_PROTECTION
#error "You cannot link to this source (_lit_val.c) directly, please consider including dcommon.h"
#endif

#include "dcommon.h"

DCResI8 dc_str_to_i8(const string str)
{
    DC_RES_i8();

    if (!str)
    {
        dc_dbg_log("str is NULL or not provided");

        dc_ret_e(1, "str is NULL or not provided");
    }

    if (strlen(str) == 0) dc_ret_ok(0);

    string end;
    errno = 0;
    long val = strtol(str, &end, 10);

    if (end == str || *end != '\0')
    {
        dc_dbg_log("str is not '\\0' terminated, or not provided");

        dc_ret_e(1, "str is not '\\0' terminated, or not provided");
    }

    if (val < INT8_MIN || val > INT8_MAX || errno == ERANGE)
    {
        dc_dbg_log("result is not within the 'i8' range: %" PRId8 ", %" PRId8, INT8_MIN, INT8_MAX);

        dc_ret_e(1, "result is not within the 'i8' range");
    }

    dc_ret_ok((i8)val);
}

DCResI16 dc_str_to_i16(const string str)
{
    DC_RES_i16();

    if (!str)
    {
        dc_dbg_log("str is NULL or not provided");

        dc_ret_e(1, "str is NULL or not provided");
    }

    if (strlen(str) == 0) dc_ret_ok(0);

    string end;
    errno = 0;
    long val = strtol(str, &end, 10);

    if (end == str || *end != '\0')
    {
        dc_dbg_log("str is not '\\0' terminated, or not provided");

        dc_ret_e(1, "str is not '\\0' terminated, or not provided");
    }

    if (val < INT16_MIN || val > INT16_MAX || errno == ERANGE)
    {
        dc_dbg_log("result is not within the 'i16' range: %" PRId16 ", %" PRId16, INT16_MIN, INT16_MAX);

        dc_ret_e(1, "result is not within the 'i16' range");
    }

    dc_ret_ok((i16)val);
}

DCResI32 dc_str_to_i32(const string str)
{
    DC_RES_i32();

    string end;
    errno = 0;
    long val = strtol(str, &end, 10);

    if (end == str || *end != '\0')
    {
        dc_dbg_log("str is not '\\0' terminated, or not provided");

        dc_ret_e(1, "str is not '\\0' terminated, or not provided");
    }

    if (val < INT32_MIN || val > INT32_MAX || errno == ERANGE)
    {
        dc_dbg_log("result is not within the 'i32' range: %" PRId32 ", %" PRId32, INT32_MIN, INT32_MAX);

        dc_ret_e(1, "result is not within the 'i32' range");
    }

    dc_ret_ok((i32)val);
}

DCResI64 dc_str_to_i64(const string str)
{
    DC_RES_i64();

    string end;
    errno = 0;
    long long val = strtoll(str, &end, 10);

    if (end == str || *end != '\0')
    {
        dc_dbg_log("str is not '\\0' terminated, or not provided");

        dc_ret_e(1, "str is not '\\0' terminated, or not provided");
    }

    if (val < INT64_MIN || val > INT64_MAX || errno == ERANGE)
    {
        dc_dbg_log("result is not within the 'i64' range: %" PRId64 ", %" PRId64, INT64_MIN, INT64_MAX);

        dc_ret_e(1, "result is not within the 'i64' range");
    }

    dc_ret_ok((i64)val);
}

DCResU8 dc_str_to_u8(const string str)
{
    DC_RES_u8();

    string end;
    errno = 0;
    unsigned long val = strtoul(str, &end, 10);

    if (end == str || *end != '\0')
    {
        dc_dbg_log("str is not '\\0' terminated, or not provided");

        dc_ret_e(1, "str is not '\\0' terminated, or not provided");
    }

    if (val > UINT8_MAX || errno == ERANGE)
    {
        dc_dbg_log("result is not within the 'u8' range: 0, %" PRIu8, UINT8_MAX);

        dc_ret_e(1, "result is not within the 'u8' range");
    }

    dc_ret_ok((u8)val);
}

DCResU16 dc_str_to_u16(const string str)
{
    DC_RES_u16();

    string end;
    errno = 0;
    unsigned long val = strtoul(str, &end, 10);

    if (end == str || *end != '\0')
    {
        dc_dbg_log("str is not '\\0' terminated, or not provided");

        dc_ret_e(1, "str is not '\\0' terminated, or not provided");
    }

    if (val > UINT16_MAX || errno == ERANGE)
    {
        dc_dbg_log("result is not within the 'u16' range: 0, %" PRIu16, UINT16_MAX);

        dc_ret_e(1, "result is not within the 'u16' range");
    }

    dc_ret_ok((u16)val);
}

DCResU32 dc_str_to_u32(const string str)
{
    DC_RES_u32();

    string end;
    errno = 0;
    unsigned long val = strtoul(str, &end, 10);

    if (end == str || *end != '\0')
    {
        dc_dbg_log("str is not '\\0' terminated, or not provided");

        dc_ret_e(1, "str is not '\\0' terminated, or not provided");
    }

    if (val > UINT32_MAX || errno == ERANGE)
    {
        dc_dbg_log("result is not within the 'u32' range: 0, %" PRIu32, UINT32_MAX);

        dc_ret_e(1, "result is not within the 'u32' range");
    }

    dc_ret_ok((u32)val);
}

DCResU64 dc_str_to_u64(const string str)
{
    DC_RES_u64();

    string end;
    errno = 0;
    unsigned long long val = strtoull(str, &end, 10);

    if (end == str || *end != '\0')
    {
        dc_dbg_log("str is not '\\0' terminated, or not provided");

        dc_ret_e(1, "str is not '\\0' terminated, or not provided");
    }

    if (val > UINT64_MAX || errno == ERANGE)
    {
        dc_dbg_log("result is not within the 'u64' range: 0, %" PRIu64, UINT64_MAX);

        dc_ret_e(1, "result is not within the 'u64' range");
    }

    dc_ret_ok((u64)val);
}

DCResF32 dc_str_to_f32(const string str)
{
    DC_RES_f32();

    string end;
    errno = 0;
    float val = strtof(str, &end);

    if (end == str || *end != '\0')
    {
        dc_dbg_log("str is not '\\0' terminated, or not provided");

        dc_ret_e(1, "str is not '\\0' terminated, or not provided");
    }

    if (errno == ERANGE)
    {
        dc_dbg_log("result is not within the 'f32' range");

        dc_ret_e(1, "result is not within the 'f32' range");
    }

    dc_ret_ok((f32)val);
}

DCResF64 dc_str_to_f64(const string str)
{
    DC_RES_f64();

    string end;
    errno = 0;
    double val = strtod(str, &end);

    if (end == str || *end != '\0')
    {
        dc_dbg_log("str is not '\\0' terminated, or not provided");

        dc_ret_e(1, "str is not '\\0' terminated, or not provided");
    }

    if (errno == ERANGE)
    {
        dc_dbg_log("result is not within the 'f64' range");

        dc_ret_e(1, "result is not within the 'f64' range");
    }

    dc_ret_ok((f64)val);
}
