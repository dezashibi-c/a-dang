// ***************************************************************************************
//    Project: dcommon -> https://github.com/dezashibi-c/dcommon
//    File: dcommon.h
//    Date: 2024-09-08
//    Author: Navid Dezashibi
//    Contact: navid@dezashibi.com
//    Website: https://dezashibi.com | https://github.com/dezashibi
//    License:
//     Please refer to the LICENSE file, repository or website for more
//     information about the licensing of this work. If you have any questions
//     or concerns, please feel free to contact me at the email address provided
//     above.
// ***************************************************************************************
// *  Description: Main `dcommon` library entry point
// ***************************************************************************************

#ifndef DC_MAIN_HEADER_H
#define DC_MAIN_HEADER_H

#include <limits.h>
#include <math.h>
#include <memory.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __DC_BYPASS_PRIVATE_HEADER_PROTECTION

#include "_headers/aliases.h"

#include "_headers/macros.h"

FILE* dc_error_logs = NULL;
DC_ERROR_MODE dc_error_mode = DC_ERR_MODE_NORMAL;

#endif // DC_MAIN_HEADER_H
