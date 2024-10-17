#define CLOVE_SUITE_NAME ast_tests

#include "clove-unit/clove-unit.h"

#include "ast.h"
#include "types.h"

CLOVE_TEST(node_string)
{
    // "let my_var another_var;-1"

    DNode* program = dc_res_val2(dn_new(DN_PROGRAM, dc_dv_nullptr(), true));

    DCDynVal my_var = dc_dv(string, "my_var");
    DCDynVal another_var = dc_dv(string, "another_var");
    DCDynVal minus = dc_dv(string, "-");
    DCDynVal one = dc_dv(i64, 1);

    DNode* statement1 = dc_res_val2(dn_new(DN_LET_STATEMENT, dc_dv_nullptr(), true));

    DNode* ident = dc_res_val2(dn_new(DN_IDENTIFIER, my_var, false));
    DNode* ident2 = dc_res_val2(dn_new(DN_IDENTIFIER, another_var, false));

    DNode* expression = dc_res_val2(dn_new(DN_PREFIX_EXPRESSION, minus, true));
    DNode* value = dc_res_val2(dn_new(DN_INTEGER_LITERAL, one, true));

    dn_child_push(expression, value);

    dn_child_push(statement1, ident);
    dn_child_push(statement1, ident2);

    dn_child_push(program, statement1);
    dn_child_push(program, expression);

    string result = NULL;
    DCResVoid inspection_res = dang_node_inspect(program, &result);
    if (dc_res_is_err2(inspection_res))
    {
        dc_res_err_log2(inspection_res, "Inspection error");

        CLOVE_FAIL();
    }
    else
        dc_action_on(strcmp("let my_var another_var\n(-1)\n", result) != 0, CLOVE_FAIL(),
                     "expected='let my_var another_var\n(-1)\n', got=%s", result);

    dn_program_free(program);

    CLOVE_PASS();
}
