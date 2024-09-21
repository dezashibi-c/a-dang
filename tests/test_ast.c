#define CLOVE_SUITE_NAME ast_tests
#define DCOMMON_IMPL

#include "clove-unit/clove-unit.h"

#include "ast.h"
#include "dcommon/dcommon.h"

CLOVE_TEST(node_string)
{
    const string input = "let my_var another_var";

    DNode* program = dnode_create(DN_PROGRAM, NULL, true);

    Token* let_tok = token_create(TOK_LET, input, 0, 3);
    Token* my_var_tok = token_create(TOK_IDENT, input, 4, 6);
    Token* another_var = token_create(TOK_IDENT, input, 11, 11);

    DNode* statement1 = dnode_create(DN_LET_STATEMENT, let_tok, true);
    DNode* ident = dnode_create(DN_IDENTIFIER, my_var_tok, false);
    DNode* ident2 = dnode_create(DN_IDENTIFIER, another_var, false);

    dn_child_push(statement1, ident);
    dn_child_push(statement1, ident2);

    dn_child_push(program, statement1);

    dnode_string_init(program);

    dc_action_on(strcmp(program->text, "let my_var another_var\n") != 0,
                 CLOVE_FAIL(), "wrong string for program, got='%s'",
                 program->text);

    dnode_program_free(program);

    CLOVE_PASS();
}
