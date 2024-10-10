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

DObjResult dang_eval(DNode* dn, DEnv* de);

void dang_obj_init(DObject* dobj, DObjType dobjt, DCDynVal dv, bool is_returned);
DObjPResult dang_obj_new(DObjType dobjt, DCDynVal dv, bool is_returned);
DObjPResult dang_obj_new_from(DObject* dobj);
DCResultVoid dang_dobj_free(DObject* dobj);

DEnvResult dang_denv_new();
DCResultVoid dang_denv_free(DEnv* de);
DObjPResult dang_denv_get(DEnv* de, string name);
DObjPResult dang_denv_set(DEnv* de, string name, DObject* dobj);

string tostr_DObjType(DObjType dobjt);

#endif // DANG_EVAL_H
