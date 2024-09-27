// ***************************************************************************************
//    Project: Dang Compiler -> https://github.com/dezashibi-c/dang
//    File: common.h
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

#ifndef DANG_COMMON_H
#define DANG_COMMON_H

typedef enum
{
    DANG_MAIN_BATCH,
    DANG_TEMP_BATCH,

    DANG_MAX_BATCH,
} DangCleanupBatch;

void configure(bool init_pool, string log_file, bool append_logs);

#endif // DANG_COMMON_H
