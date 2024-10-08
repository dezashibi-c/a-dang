// ***************************************************************************************
//    Project: Dang Compiler -> https://github.com/dezashibi-c/dang
//    File: evaluator.c
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
// *  Description: Evaluator source file
// ***************************************************************************************

#include "evaluator.h"

static DCResult dang_eval_statements(DNode* dn)
{
    DC_RES();

    dc_da_for(dn->children) dc_try_fail(dang_eval(dn_child(dn, _idx)));

    dc_res_ret();
}

DCResult dang_eval(DNode* dn)
{
    DC_RES();

    if (!dn)
        dc_res_ret_e(-1, "got NULL node");
    else if (!dn_type_is_valid(dn->type))
        dc_res_ret_ea(-1, "got invalid node type: %s", tostr_DNType(dn->type));

    switch (dn->type)
    {
        case DN_PROGRAM:
            return dang_eval_statements(dn);

        case DN_EXPRESSION_STATEMENT:
            return dang_eval(dn_child(dn, 0));

        case DN_INTEGER_LITERAL:
            dc_res_ret_ok_dv(i64, dn_child_as(dn, 0, i64));

        default:
            break;
    };

    dc_res_ret_ea(-1, "Unimplemented or unsupported node type: %s", tostr_DNType(dn->type));
}
