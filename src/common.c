// ***************************************************************************************
//    Project: Dang Compiler -> https://github.com/dezashibi-c/dang
//    File: common.c
//    Date: 2024-09-26
//    Author: Navid Dezashibi
//    Contact: navid@dezashibi.com
//    Website: https://dezashibi.com | https://github.com/dezashibi
//    License:
//     Please refer to the LICENSE file, repository or website for more
//     information about the licensing of this work. If you have any questions
//     or concerns, please feel free to contact me at the email address provided
//     above.
// ***************************************************************************************
// *  Description: This is for initializing dcommon lib
// ***************************************************************************************

#define DCOMMON_IMPL

#include "types.h"

#include "common.h"

void configure(bool init_pool, string log_file, bool append_logs)
{
    if (log_file) dc_error_logs_init(log_file, append_logs);

    if (init_pool) dc_cleanup_pool_init2(DANG_MAX_BATCH, 10);
}
