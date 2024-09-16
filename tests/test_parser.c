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

bool test_DNodeLetStatement(DNode* stmt, string name)
{
    dc_action_on(
        !dc_sv_str_eq(stmt->token->text, "let"), return false,
        "---- Wrong text for statement token text, expected text='let' but "
        "got = '" DC_SV_FMT "' ",
        dc_sv_fmt_val(stmt->token->text));

    dc_action_on(
        stmt->type != DN_LET_STATEMENT, return false,
        "---- Wrong statement node type, expected type='%s' but got='%s'",
        tostr_DangNodeType(DN_LET_STATEMENT), tostr_DangNodeType(stmt->type));

    DNodeLetStatement* let_stmt = (DNodeLetStatement*)stmt;

    dc_action_on(let_stmt->name.type != DN_IDENTIFIER, return false,
                 "---- Wrong name node type, expected type='%s' but got='%s'",
                 tostr_DangNodeType(DN_IDENTIFIER),
                 tostr_DangNodeType(let_stmt->name.type));

    dc_action_on(!dc_sv_str_eq(let_stmt->name.token->text, name), return false,
                 " ---- Wrong text for statement's name token text, expected "
                 "text='%s' but "
                 "got = '" DC_SV_FMT "' ",
                 name, dc_sv_fmt_val(let_stmt->token->text));

    return true;
}

CLOVE_TEST(let_statements)
{
    const string input = "let x = 5; let y = 10\n"
                         "let foobar = 838383";

    Scanner s;
    scanner_init(&s, input);

    Parser p;
    parser_init(&p, &s);

    DNodeProgram program = parser_parse_program(&p);

    dc_action_on(program.type != DN_PROGRAM, CLOVE_FAIL(),
                 "Wrong program node type, expected type='%s' but got='%s'",
                 tostr_DangNodeType(DN_PROGRAM),
                 tostr_DangNodeType(program.type));

    dc_action_on(program.statements.count != 3, CLOVE_FAIL(),
                 "Wrong number of statements, expected='%d' but got='%zu'", 3,
                 program.statements.count);

    string expected_identifiers[] = {"x", "y", "foobar"};

    for (usize i = 0; i < dc_count(expected_identifiers); ++i)
    {
        DNode* stmt = dc_dynarr_get_as(&program.statements, i, voidptr);

        dc_action_on(!test_DNodeLetStatement(stmt, expected_identifiers[i]),
                     CLOVE_FAIL(), "Test error on item #%zu", i);
    }

    CLOVE_PASS();
}