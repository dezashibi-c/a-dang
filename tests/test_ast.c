#define CLOVE_SUITE_NAME ast_tests
#define DCOMMON_IMPL

#include "clove-unit/clove-unit.h"

#include "ast.h"
#include "dcommon/dcommon.h"

CLOVE_TEST(node_string)
{
    const string input = "let my_var another_var;-1";

    DNode* program = dnode_create(DN_PROGRAM, NULL, true);

    Token* let_tok = token_create(TOK_LET, input, 0, 3);
    Token* my_var_tok = token_create(TOK_IDENT, input, 4, 6);
    Token* another_var = token_create(TOK_IDENT, input, 11, 11);
    Token* minus = token_create(TOK_IDENT, input, 23, 1);
    Token* one = token_create(TOK_IDENT, input, 24, 1);

    DNode* statement1 = dnode_create(DN_LET_STATEMENT, let_tok, true);

    DNode* ident = dnode_create(DN_IDENTIFIER, my_var_tok, false);
    DNode* ident2 = dnode_create(DN_IDENTIFIER, another_var, false);

    DNode* expression = dnode_create(DN_PREFIX_EXPRESSION, minus, true);
    DNode* value = dnode_create(DN_INTEGER_LITERAL, one, true);

    dn_child_push(expression, value);
    dn_val_push(value, i64, 1);

    dn_child_push(statement1, ident);
    dn_child_push(statement1, ident2);

    dn_child_push(program, statement1);
    dn_child_push(program, expression);

    dnode_string_init(program);

    dc_action_on(strcmp(program->text, "let my_var another_var\n(-1)\n") != 0,
                 CLOVE_FAIL(), "wrong string for program, got='%s'",
                 program->text);

    dnode_program_free(program);

    token_free(let_tok);
    token_free(my_var_tok);
    token_free(another_var);
    token_free(minus);
    token_free(one);

    CLOVE_PASS();
}
