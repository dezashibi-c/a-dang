#define CLOVE_SUITE_NAME parser_tests
#define DCOMMON_IMPL

#include "clove-unit/clove-unit.h"

#include "dcommon/dcommon.h"
#include "parser.h"
#include "scanner.h"

static bool parser_has_no_error(Parser* p)
{
    dc_action_on(p->errors.count != 0, parser_log_errors(p);
                 return false, "parser has %zu errors", p->errors.count);

    return true;
}

static bool node_is_valid(DNode* node, DangNodeType dnt,
                          usize number_of_expected_statements)
{
    dc_action_on(node->type != dnt, return false,
                 "Wrong node node type, expected type='%s' but got='%s'",
                 tostr_DangNodeType(dnt), tostr_DangNodeType(node->type));

    dc_action_on(node->children.count != number_of_expected_statements,
                 return false,
                 "Wrong number of statements, expected='%zu' but got='%zu'",
                 number_of_expected_statements, node->children.count);

    return true;
}

static bool program_is_valid(DNode* program,
                             usize number_of_expected_statements)
{
    return node_is_valid(program, DN_PROGRAM, number_of_expected_statements);
}

static bool test_DNodeLetStatement(DNode* stmt, string* name)
{
    dc_action_on(!dc_sv_str_eq(stmt->token->text, "let"), return false,
                 "---- Token text must be 'let', got='" DC_SV_FMT "'",
                 dc_sv_fmt_val(stmt->token->text));

    dc_action_on(
        stmt->type != DN_LET_STATEMENT, return false,
        "---- Wrong statement node type, expected type='%s' but got='%s'",
        tostr_DangNodeType(DN_LET_STATEMENT), tostr_DangNodeType(stmt->type));

    dc_action_on(stmt->children.count != 1, return false,
                 "---- Wrong number of children expected '1' but got='%zu'",
                 stmt->children.count);

    DNode* stmt_name = dc_da_get_as(&stmt->children, 0, voidptr);

    dc_action_on(stmt_name->type != DN_IDENTIFIER, return false,
                 "---- Wrong name node type, expected type='%s' but got = '%s'",
                 tostr_DangNodeType(DN_IDENTIFIER),
                 tostr_DangNodeType(stmt_name->type));

    dc_action_on(!dc_sv_str_eq(stmt_name->token->text, *name), return false,
                 " ---- Wrong text for statement's name token text, expected "
                 "text = '%s' but "
                 "got = '" DC_SV_FMT "' ",
                 *name, dc_sv_fmt_val(stmt_name->token->text));

    return true;
}

CLOVE_TEST(let_statements)
{
    const string input = "let $1 5; let $\"some long variable name\" 10\n"
                         "let foobar 838383";

    Scanner s;
    scanner_init(&s, input);

    Parser p;
    parser_init(&p, &s);

    DNode* program = parser_parse_program(&p);

    dc_action_on(!parser_has_no_error(&p), CLOVE_FAIL(), "parser has error");

    dc_action_on(!program_is_valid(program, 3), CLOVE_FAIL(),
                 "program is not valid");

    string expected_identifiers[] = {"1", "some long variable name", "foobar"};

    for (usize i = 0; i < dc_count(expected_identifiers); ++i)
    {
        DNode* stmt = dc_da_get_as(&program->children, i, voidptr);

        dc_action_on(!test_DNodeLetStatement(stmt, &expected_identifiers[i]),
                     CLOVE_FAIL(),
                     "Test failed on item #%zu check reasons above", i);
    }

    dnode_program_free(program);
    parser_free(&p);

    CLOVE_PASS();
}

CLOVE_TEST(return_statement)
{
    const string input = "return 5; return 10\n"
                         "return 838383";

    Scanner s;
    scanner_init(&s, input);

    Parser p;
    parser_init(&p, &s);

    DNode* program = parser_parse_program(&p);

    dc_action_on(!parser_has_no_error(&p), CLOVE_FAIL(), "parser has error");

    dc_action_on(!program_is_valid(program, 3), CLOVE_FAIL(),
                 "program is not valid");

    for (usize i = 0; i < program->children.count; ++i)
    {
        DNode* stmt = dc_da_get_as(&program->children, i, voidptr);

        dc_action_on(!dc_sv_str_eq(stmt->token->text, "return"), CLOVE_FAIL(),
                     "Token text must be 'return', got='" DC_SV_FMT "'",
                     dc_sv_fmt_val(stmt->token->text));

        dc_action_on(
            stmt->type != DN_RETURN_STATEMENT, CLOVE_FAIL(),
            "Wrong statement node type, expected type='%s' but got='%s'",
            tostr_DangNodeType(DN_RETURN_STATEMENT),
            tostr_DangNodeType(stmt->type));
    }

    dnode_program_free(program);
    parser_free(&p);

    CLOVE_PASS();
}

CLOVE_TEST(identifier)
{
    const string input = "foobar";

    Scanner s;
    scanner_init(&s, input);

    Parser p;
    parser_init(&p, &s);

    DNode* program = parser_parse_program(&p);

    dc_action_on(!parser_has_no_error(&p), CLOVE_FAIL(), "parser has error");

    dc_action_on(!program_is_valid(program, 1), CLOVE_FAIL(),
                 "program is not valid");

    DNode* statement1 = dn_child(program, 0);
    dc_action_on(!node_is_valid(statement1, DN_EXPRESSION_STATEMENT, 1),
                 CLOVE_FAIL(), "statement is not valid");

    DNode* expression = dn_child(statement1, 0);
    dc_action_on(!node_is_valid(expression, DN_IDENTIFIER, 1), CLOVE_FAIL(),
                 "statement is not valid");

    string value = dn_child_as(expression, 0, string);
    dc_action_on(strcmp(value, "foobar") != 0, CLOVE_FAIL(),
                 "identifier value is not '%s', got='%s'", "foobar", value);

    dc_action_on(!dc_sv_str_eq(expression->token->text, "foobar"), CLOVE_FAIL(),
                 "identifier value is not '%s', got='" DC_SV_FMT "'", "foobar",
                 dc_sv_fmt_val(expression->token->text));

    dnode_program_free(program);
    parser_free(&p);

    CLOVE_PASS();
}
