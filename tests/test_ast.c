#define CLOVE_SUITE_NAME ast_tests

#include "clove-unit/clove-unit.h"

#include "ast.h"
#include "common.h"

CLOVE_TEST(node_string)
{
    // "let my_var another_var;-1"

    DCDynArrPtr stmts = dc_unwrap2(dc_da_new(NULL));

    DCDynVal my_var = dc_dv(string, "my_var");
    DCDynVal another_var = dc_dv(string, "another_var");
    DCDynVal minus = dc_dv(string, "-");
    DCDynVal one = dc_dv(i64, 1);


    DCDynVal ident2 = dc_dv(DNodeIdentifier, dn_identifier(dc_dv_as(another_var, string)));

    DCDynVal statement1 = dc_dv(DNodeLetStatement, (dn_let(dc_dv_as(my_var, string), &ident2)));

    DCDynVal expression = dc_dv(DNodePrefixExpression, (dn_prefix(dc_dv_as(minus, string), &one)));

    dc_da_push(stmts, dc_dv(DCDynValPtr, &statement1));
    dc_da_push(stmts, dc_dv(DCDynValPtr, &expression));

    DCDynVal program_statements = dc_dva(DCDynArrPtr, stmts);

    DNodeProgram program = dn_program(dc_dv_as(program_statements, DCDynArrPtr));

    string result = NULL;
    DCResVoid inspection_res = dang_program_inspect(&program, &result);
    if (dc_is_err2(inspection_res))
    {
        dc_err_log2(inspection_res, "Inspection error");

        CLOVE_FAIL();
    }
    else
        dc_action_on(strcmp("let my_var another_var\n(-1)\n", result ? result : "") != 0, CLOVE_FAIL(),
                     "expected='let my_var another_var\n(-1)\n', got=%s", result);

    dc_dv_free(&program_statements, NULL);

    CLOVE_PASS();
}
