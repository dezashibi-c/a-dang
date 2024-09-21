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
    dc_action_on(node == NULL, return false, "received NULL node");

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

static bool test_integer_value(DNode* expression, string expected,
                               i64 expected_val)
{
    dc_action_on(!node_is_valid(expression, DN_INTEGER_LITERAL, 1),
                 return false, "expression is not valid");

    i64 value = dn_child_as(expression, 0, i64);
    dc_action_on(value != expected_val, return false,
                 "expected integer value of '%" PRId64 "', got='%" PRId64 "'",
                 expected_val, value);

    dc_action_on(!dc_sv_str_eq(expression->token->text, expected), return false,
                 "identifier value is not '%s', got='" DC_SV_FMT "'", expected,
                 dc_sv_fmt_val(expression->token->text));

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
    dc_action_on(!node_is_valid(expression, DN_IDENTIFIER, 0), CLOVE_FAIL(),
                 "expression is not valid");

    dc_action_on(!dc_sv_str_eq(expression->token->text, "foobar"), CLOVE_FAIL(),
                 "identifier value is not '%s', got='" DC_SV_FMT "'", "foobar",
                 dc_sv_fmt_val(expression->token->text));

    dnode_program_free(program);
    parser_free(&p);

    CLOVE_PASS();
}

CLOVE_TEST(integer_literal)
{
    const string input = "5";

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
    dc_action_on(!test_integer_value(expression, "5", 5), CLOVE_FAIL(),
                 "Wrong integer value node");

    dnode_program_free(program);
    parser_free(&p);

    CLOVE_PASS();
}

typedef struct
{
    string input;
    string operator;
    string lval_str;
    i64 lval;
    string rval_str;
    i64 rval;
} ExpressionTest;

CLOVE_TEST(prefix_expressions)
{
    ExpressionTest tests[] = {{"!5", "!", "5", 5, "", 0},
                              {"-15", "-", "15", 15, "", 0}};

    for (usize i = 0; i < dc_count(tests); ++i)
    {
        Scanner s;
        scanner_init(&s, tests[i].input);

        Parser p;
        parser_init(&p, &s);

        DNode* program = parser_parse_program(&p);

        dc_action_on(!parser_has_no_error(&p), CLOVE_FAIL(),
                     "parser has error");

        dc_action_on(!program_is_valid(program, 1), CLOVE_FAIL(),
                     "program is not valid");

        DNode* statement1 = dn_child(program, 0);
        dc_action_on(!node_is_valid(statement1, DN_EXPRESSION_STATEMENT, 1),
                     CLOVE_FAIL(), "statement is not valid");

        DNode* prefix = dn_child(statement1, 0);
        dc_action_on(!node_is_valid(prefix, DN_PREFIX_EXPRESSION, 1),
                     CLOVE_FAIL(), "prefix expression is not valid");

        dc_action_on(!dc_sv_str_eq(prefix->token->text, tests[i].operator),
                     CLOVE_FAIL(), "operator is not '%s', got='" DC_SV_FMT "'",
                     tests[i].operator, dc_sv_fmt_val(prefix->token->text));

        DNode* value = dn_child(prefix, 0);
        dc_action_on(
            !test_integer_value(value, tests[i].lval_str, tests[i].lval),
            CLOVE_FAIL(), "Wrong integer value node");

        dnode_program_free(program);
        parser_free(&p);
    }

    CLOVE_PASS();
}

CLOVE_TEST(infix_expressions)
{
    ExpressionTest tests[] = {
        {"5 + 5", "+", "5", 5, "5", 5},   {"5 - 5", "-", "5", 5, "5", 5},
        {"5 * 5", "*", "5", 5, "5", 5},   {"5 / 5", "/", "5", 5, "5", 5},
        {"5 > 5", ">", "5", 5, "5", 5},   {"5 < 5", "<", "5", 5, "5", 5},
        {"5 == 5", "==", "5", 5, "5", 5}, {"5 != 5", "!=", "5", 5, "5", 5},
    };

    for (usize i = 0; i < dc_count(tests); ++i)
    {
        Scanner s;
        scanner_init(&s, tests[i].input);

        Parser p;
        parser_init(&p, &s);

        DNode* program = parser_parse_program(&p);

        dc_action_on(!parser_has_no_error(&p), CLOVE_FAIL(),
                     "parser has error");

        dc_action_on(!program_is_valid(program, 1), CLOVE_FAIL(),
                     "program is not valid");

        DNode* statement1 = dn_child(program, 0);
        dc_action_on(!node_is_valid(statement1, DN_EXPRESSION_STATEMENT, 1),
                     CLOVE_FAIL(), "statement is not valid");

        DNode* infix = dn_child(statement1, 0);
        dc_action_on(!node_is_valid(infix, DN_INFIX_EXPRESSION, 2),
                     CLOVE_FAIL(), "infix expression is not valid");

        dc_action_on(!dc_sv_str_eq(infix->token->text, tests[i].operator),
                     CLOVE_FAIL(), "operator is not '%s', got='" DC_SV_FMT "'",
                     tests[i].operator, dc_sv_fmt_val(infix->token->text));

        DNode* lval = dn_child(infix, 0);
        dc_action_on(
            !test_integer_value(lval, tests[i].lval_str, tests[i].lval),
            CLOVE_FAIL(), "Wrong integer value node for left value");

        DNode* rval = dn_child(infix, 1);
        dc_action_on(
            !test_integer_value(rval, tests[i].rval_str, tests[i].rval),
            CLOVE_FAIL(), "Wrong integer value node for right value");

        dnode_program_free(program);
        parser_free(&p);
    }

    CLOVE_PASS();
}

CLOVE_TEST(operator_precedence)
{
    string tests[] = {
        "-a * b",
        "((-a) * b)\n",

        "!-a",
        "(!(-a))\n",

        "a + b + c",
        "((a + b) + c)\n",

        "a + b - c",
        "((a + b) - c)\n",

        "a * b * c",
        "((a * b) * c)\n",

        "a * b / c",
        "((a * b) / c)\n",

        "a + b / c",
        "(a + (b / c))\n",

        "a + b * c + d / e - f",
        "(((a + (b * c)) + (d / e)) - f)\n",

        "3 + 4; -5 * 5",
        "(3 + 4)\n((-5) * 5)\n",

        "5 > 4 == 3 < 4",
        "((5 > 4) == (3 < 4))\n",

        "5 < 4 != 3 > 4",
        "((5 < 4) != (3 > 4))\n",

        "3 + 4 * 5 == 3 * 1 + 4 * 5",
        "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))\n",
    };

    for (usize i = 0; i < dc_count(tests) / 2; ++i)
    {
        string input = tests[i * 2];
        string expected = tests[(i * 2) + 1];

        Scanner s;
        scanner_init(&s, input);

        Parser p;
        parser_init(&p, &s);

        DNode* program = parser_parse_program(&p);

        dc_action_on(!parser_has_no_error(&p), CLOVE_FAIL(),
                     "parser has error");

        dnode_string_init(program);

        dc_action_on(strcmp(expected, program->text) != 0, CLOVE_FAIL(),
                     "expected=%s, got=%s", expected, program->text);

        dnode_program_free(program);
        parser_free(&p);
    }

    CLOVE_PASS();
}
