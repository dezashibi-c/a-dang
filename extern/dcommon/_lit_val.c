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
#error                                                                         \
    "You cannot link to this source (_lit_val.c) directly, please consider including dcommon.h"
#endif

#include "dcommon.h"

bool dc_str_to_i8(const char* str, i8* out)
{
    char* end;
    errno = 0;
    long val = strtol(str, &end, 10);
    if (end == str || *end != '\0' || errno == ERANGE || val < INT8_MIN ||
        val > INT8_MAX)
    {
        return false;
    }
    *out = (i8)val;
    return true;
}

bool dc_str_to_i16(const char* str, i16* out)
{
    char* end;
    errno = 0;
    long val = strtol(str, &end, 10);
    if (end == str || *end != '\0' || errno == ERANGE || val < INT16_MIN ||
        val > INT16_MAX)
    {
        return false;
    }
    *out = (i16)val;
    return true;
}

bool dc_str_to_i32(const char* str, i32* out)
{
    char* end;
    errno = 0;
    long val = strtol(str, &end, 10);
    if (end == str || *end != '\0' || errno == ERANGE || val < INT32_MIN ||
        val > INT32_MAX)
    {
        return false;
    }
    *out = (i32)val;
    return true;
}

bool dc_str_to_i64(const char* str, i64* out)
{
    char* end;
    errno = 0;
    long long val = strtoll(str, &end, 10);
    if (end == str || *end != '\0' || errno == ERANGE)
    {
        return false;
    }
    *out = (i64)val;
    return true;
}

bool dc_str_to_u8(const char* str, u8* out)
{
    char* end;
    errno = 0;
    unsigned long val = strtoul(str, &end, 10);
    if (end == str || *end != '\0' || errno == ERANGE || val > UINT8_MAX)
    {
        return false;
    }
    *out = (u8)val;
    return true;
}

bool dc_str_to_u16(const char* str, u16* out)
{
    char* end;
    errno = 0;
    unsigned long val = strtoul(str, &end, 10);
    if (end == str || *end != '\0' || errno == ERANGE || val > UINT16_MAX)
    {
        return false;
    }
    *out = (u16)val;
    return true;
}

bool dc_str_to_u32(const char* str, u32* out)
{
    char* end;
    errno = 0;
    unsigned long val = strtoul(str, &end, 10);
    if (end == str || *end != '\0' || errno == ERANGE || val > UINT32_MAX)
    {
        return false;
    }
    *out = (u32)val;
    return true;
}

bool dc_str_to_u64(const char* str, u64* out)
{
    char* end;
    errno = 0;
    unsigned long long val = strtoull(str, &end, 10);
    if (end == str || *end != '\0' || errno == ERANGE)
    {
        return false;
    }
    *out = (u64)val;
    return true;
}

bool dc_str_to_f32(const char* str, f32* out)
{
    char* end;
    errno = 0;
    float val = strtof(str, &end);
    if (end == str || *end != '\0' || errno == ERANGE)
    {
        return false;
    }
    *out = val;
    return true;
}

bool dc_str_to_f64(const char* str, f64* out)
{
    char* end;
    errno = 0;
    double val = strtod(str, &end);
    if (end == str || *end != '\0' || errno == ERANGE)
    {
        return false;
    }
    *out = val;
    return true;
}
