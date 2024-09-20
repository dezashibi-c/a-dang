#define CLOVE_SUITE_NAME parser_tests
#define DCOMMON_IMPL

#include "clove-unit/clove-unit.h"

#include "dcommon/dcommon.h"
#include "parser.h"
#include "scanner.h"

typedef struct
{
    DangTokenType type;
    const string text;
} TestExpectedResult;

static bool parser_has_no_error(Parser* p)
{
    dc_action_on(p->errors.count != 0, parser_log_errors(p);
                 return false, "parser has %zu errors", p->errors.count);

    return true;
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
    const string input = "let x 5; let y 10\n"
                         "let foobar 838383";

    Scanner s;
    scanner_init(&s, input);

    Parser p;
    parser_init(&p, &s);

    DNode* program = parser_parse_program(&p);

    dc_action_on(!parser_has_no_error(&p), CLOVE_FAIL(), "parser has error");

    dc_action_on(program->type != DN_PROGRAM, CLOVE_FAIL(),
                 "Wrong program node type, expected type='%s' but got='%s'",
                 tostr_DangNodeType(DN_PROGRAM),
                 tostr_DangNodeType(program->type));

    dc_action_on(program->children.count != 3, CLOVE_FAIL(),
                 "Wrong number of statements, expected='%d' but got='%zu'", 3,
                 program->children.count);

    string expected_identifiers[] = {"x", "y", "foobar"};

    for (usize i = 0; i < dc_count(expected_identifiers); ++i)
    {
        DNode* stmt = dc_da_get_as(&program->children, i, voidptr);

        dc_action_on(!test_DNodeLetStatement(stmt, &expected_identifiers[i]),
                     CLOVE_FAIL(),
                     "Test failed on item #%zu check reasons above", i);
    }

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

    dc_action_on(program->type != DN_PROGRAM, CLOVE_FAIL(),
                 "Wrong program node type, expected type='%s' but got='%s'",
                 tostr_DangNodeType(DN_PROGRAM),
                 tostr_DangNodeType(program->type));

    dc_action_on(program->children.count != 3, CLOVE_FAIL(),
                 "Wrong number of statements, expected='%d' but got='%zu'", 3,
                 program->children.count);

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

    CLOVE_PASS();
}
