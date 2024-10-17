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

ResObj dang_eval(DNode* dn, DEnv* de);

DCResVoid dang_obj_init(DObj* dobj, DObjType dobjt, DCDynVal dv, DEnv* de, bool is_returned, bool has_children);
ResObj dang_obj_new(DObjType dobjt, DCDynVal dv, DEnv* de, bool is_returned, bool has_children);
ResObj dang_obj_copy(DObj* dobj);
DCResVoid dang_obj_free(DObj* dobj);
DC_DV_FREE_FN_DECL(dobj_child_free);

DEnvResult dang_env_new();
DEnvResult dang_env_new_enclosed(DEnv* outer);
DCResVoid dang_env_free(DEnv* de);
ResObj dang_env_get(DEnv* de, string name);
ResObj dang_env_set(DEnv* de, string name, DObj* dobj, bool update_only);

string tostr_DObjType(DObjType dobjt);

#endif // DANG_EVAL_H
