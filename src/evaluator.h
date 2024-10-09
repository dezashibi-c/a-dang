// ***************************************************************************************
//    Project: Dang Compiler -> https://github.com/dezashibi-c/dang
//    File: evaluator.h
//    Date: 2024-10-08
//    Author: Navid Dezashibi
//    Contact: navid@dezashibi.com
//    Website: https://dezashibi.com | https://github.com/dezashibi
//    License:
//     Please refer to the LICENSE file, repository or website for more
//     information about the licensing of this work. If you have any questions
//     or concerns, please feel free to contact me at the email address provided
//     above.
// ***************************************************************************************
// *  Description: Evaluator header file
// ***************************************************************************************

#ifndef DANG_EVAL_H
#define DANG_EVAL_H

#include "ast.h"
#include "object.h"

DObjResult dang_eval(DNode* dn);
string tostr_DObjType(DObjType dobjt);

#endif // DANG_EVAL_H
