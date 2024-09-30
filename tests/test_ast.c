#define CLOVE_SUITE_NAME ast_tests

#include "clove-unit/clove-unit.h"

#include "ast.h"
#include "dcommon/dcommon.h"

CLOVE_TEST(node_string)
{
    const string input = "let my_var another_var;-1";

    DNode* program = dc_res_val2(dn_new(DN_PROGRAM, NULL, true));

    DToken* let_tok = dc_res_val2(token_create(TOK_LET, input, 0, 3));
    DToken* my_var_tok = dc_res_val2(token_create(TOK_IDENT, input, 4, 6));
    DToken* another_var = dc_res_val2(token_create(TOK_IDENT, input, 11, 11));
    DToken* minus = dc_res_val2(token_create(TOK_IDENT, input, 23, 1));
    DToken* one = dc_res_val2(token_create(TOK_IDENT, input, 24, 1));

    DNode* statement1 = dc_res_val2(dn_new(DN_LET_STATEMENT, let_tok, true));

    DNode* ident = dc_res_val2(dn_new(DN_IDENTIFIER, my_var_tok, false));
    DNode* ident2 = dc_res_val2(dn_new(DN_IDENTIFIER, another_var, false));

    DNode* expression = dc_res_val2(dn_new(DN_PREFIX_EXPRESSION, minus, true));
    DNode* value = dc_res_val2(dn_new(DN_INTEGER_LITERAL, one, true));

    dn_child_push(expression, value);
    dn_val_push(value, i64, 1);

    dn_child_push(statement1, ident);
    dn_child_push(statement1, ident2);

    dn_child_push(program, statement1);
    dn_child_push(program, expression);

    dn_string_init(program);

    dc_action_on(strcmp(program->text, "let my_var another_var\n(-1)\n") != 0, CLOVE_FAIL(),
                 "wrong string for program, got='%s'", program->text);

    dn_program_free(program);

    token_free(let_tok);
    token_free(my_var_tok);
    token_free(another_var);
    token_free(minus);
    token_free(one);

    CLOVE_PASS();
}
