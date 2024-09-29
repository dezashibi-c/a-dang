#define CLOVE_SUITE_NAME parser_tests

#include "clove-unit/clove-unit.h"

#define DC_DEBUG

#include "dcommon/dcommon.h"
#include "parser.h"
#include "scanner.h"

static bool parser_has_no_error(Parser* p)
{
    dc_action_on(p->errors.count != 0, parser_log_errors(p);
                 return false, "parser has %zu errors", p->errors.count);

    return true;
}

static bool node_is_valid(DNode* node, DNType dnt,
                          usize number_of_expected_statements)
{
    dc_action_on(node == NULL, return false, "received NULL node");

    dc_action_on(node->type != dnt, return false,
                 "Wrong node node type, expected type='%s' but got='%s'",
                 tostr_DNType(dnt), tostr_DNType(node->type));

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
                 "---- Token text must be 'let', got='" DCPRIsv "'",
                 dc_sv_fmt(stmt->token->text));

    dc_action_on(
        stmt->type != DN_LET_STATEMENT, return false,
        "---- Wrong statement node type, expected type='%s' but got='%s'",
        tostr_DNType(DN_LET_STATEMENT), tostr_DNType(stmt->type));

    dc_action_on(stmt->children.count != 1, return false,
                 "---- Wrong number of children expected '1' but got='%zu'",
                 stmt->children.count);

    DNode* stmt_name = dc_da_get_as(stmt->children, 0, voidptr);

    dc_action_on(stmt_name->type != DN_IDENTIFIER, return false,
                 "---- Wrong name node type, expected type='%s' but got = '%s'",
                 tostr_DNType(DN_IDENTIFIER), tostr_DNType(stmt_name->type));

    dc_action_on(!dc_sv_str_eq(stmt_name->token->text, *name), return false,
                 " ---- Wrong text for statement's name token text, expected "
                 "text = '%s' but "
                 "got = '" DCPRIsv "' ",
                 *name, dc_sv_fmt(stmt_name->token->text));

    return true;
}

static bool test_BooleanLiteral(DNode* integer_literal_node, string expected,
                                bool expected_val)
{
    dc_action_on(integer_literal_node->children.count != 1, return false,
                 "Wrong number of children, expected='1' but got='%zu'",
                 integer_literal_node->children.count);

    bool value = dn_child_as(integer_literal_node, 0, u8);
    dc_action_on(value != expected_val, return false,
                 "expected boolean literal of '%s', got='%s'",
                 dc_tostr_bool(expected_val), dc_tostr_bool(value));

    dc_action_on(!dc_sv_str_eq(integer_literal_node->token->text, expected),
                 return false, "boolean literal is not '%s', got='" DCPRIsv "'",
                 expected, dc_sv_fmt(integer_literal_node->token->text));

    return true;
}

static bool test_IntegerLiteral(DNode* integer_literal_node, string expected,
                                i64 expected_val)
{
    dc_action_on(integer_literal_node->children.count != 1, return false,
                 "Wrong number of children, expected='1' but got='%zu'",
                 integer_literal_node->children.count);

    i64 value = dn_child_as(integer_literal_node, 0, i64);
    dc_action_on(value != expected_val, return false,
                 "expected integer value of '%" PRId64 "', got='%" PRId64 "'",
                 expected_val, value);

    dc_action_on(!dc_sv_str_eq(integer_literal_node->token->text, expected),
                 return false,
                 "identifier value is not '%s', got='" DCPRIsv "'", expected,
                 dc_sv_fmt(integer_literal_node->token->text));

    return true;
}

static bool test_Literal(DNode* literal_node, string expected, i64 expected_val)
{
    dc_action_on(!dn_group_is_literal(literal_node->type), return false,
                 "node is not literal, got='%s'",
                 tostr_DNType(literal_node->type));

    switch (literal_node->type)
    {
        case DN_INTEGER_LITERAL:
            return test_IntegerLiteral(literal_node, expected, expected_val);

        case DN_BOOLEAN_LITERAL:
            return test_BooleanLiteral(literal_node, expected,
                                       (bool)expected_val);

        default:
            dc_log("test for '%s' is not implemented yet",
                   tostr_DNType(literal_node->type));
            break;
    };

    return false;
}

CLOVE_TEST(let_statements)
{
    const string input = "let $1 5; let $\"some long variable name\" 10\n"
                         "let foobar 838383";

    Scanner s;
    scanner_init(&s, input);


    Parser p;
    parser_init(&p, &s);

    ResultDNode program_res = parser_parse_program(&p);

    dc_action_on(!parser_has_no_error(&p), CLOVE_FAIL(), "parser has error");

    DNode* program = dc_res_val2(program_res);

    dc_action_on(!program_is_valid(program, 3), CLOVE_FAIL(),
                 "program is not valid");

    string expected_identifiers[] = {"1", "some long variable name", "foobar"};

    for (usize i = 0; i < dc_count(expected_identifiers); ++i)
    {
        DNode* stmt = dc_da_get_as(program->children, i, voidptr);

        dc_action_on(!test_DNodeLetStatement(stmt, &expected_identifiers[i]),
                     CLOVE_FAIL(),
                     "Test failed on item #%zu check reasons above", i);
    }

    dn_program_free(program);
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

    ResultDNode program_res = parser_parse_program(&p);

    dc_action_on(!parser_has_no_error(&p), CLOVE_FAIL(), "parser has error");

    DNode* program = dc_res_val2(program_res);

    dc_action_on(!program_is_valid(program, 3), CLOVE_FAIL(),
                 "program is not valid");

    for (usize i = 0; i < program->children.count; ++i)
    {
        DNode* stmt = dc_da_get_as(program->children, i, voidptr);

        dc_action_on(!dc_sv_str_eq(stmt->token->text, "return"), CLOVE_FAIL(),
                     "Token text must be 'return', got='" DCPRIsv "'",
                     dc_sv_fmt(stmt->token->text));

        dc_action_on(
            stmt->type != DN_RETURN_STATEMENT, CLOVE_FAIL(),
            "Wrong statement node type, expected type='%s' but got='%s'",
            tostr_DNType(DN_RETURN_STATEMENT), tostr_DNType(stmt->type));
    }

    dn_program_free(program);
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

    ResultDNode program_res = parser_parse_program(&p);

    dc_action_on(!parser_has_no_error(&p), CLOVE_FAIL(), "parser has error");

    DNode* program = dc_res_val2(program_res);

    dc_action_on(!program_is_valid(program, 1), CLOVE_FAIL(),
                 "program is not valid");

    DNode* statement1 = dn_child(program, 0);
    dc_action_on(!node_is_valid(statement1, DN_EXPRESSION_STATEMENT, 1),
                 CLOVE_FAIL(), "statement is not valid");

    DNode* expression = dn_child(statement1, 0);
    dc_action_on(!node_is_valid(expression, DN_IDENTIFIER, 0), CLOVE_FAIL(),
                 "expression is not valid");

    dc_action_on(!dc_sv_str_eq(expression->token->text, "foobar"), CLOVE_FAIL(),
                 "identifier value is not '%s', got='" DCPRIsv "'", "foobar",
                 dc_sv_fmt(expression->token->text));

    dn_program_free(program);
    parser_free(&p);

    CLOVE_PASS();
}

CLOVE_TEST(boolean_literal)
{
    const string input = "true";

    Scanner s;
    scanner_init(&s, input);

    Parser p;
    parser_init(&p, &s);

    ResultDNode program_res = parser_parse_program(&p);

    dc_action_on(!parser_has_no_error(&p), CLOVE_FAIL(), "parser has error");

    DNode* program = dc_res_val2(program_res);

    dc_action_on(!program_is_valid(program, 1), CLOVE_FAIL(),
                 "program is not valid");

    DNode* statement1 = dn_child(program, 0);
    dc_action_on(!node_is_valid(statement1, DN_EXPRESSION_STATEMENT, 1),
                 CLOVE_FAIL(), "statement is not valid");

    DNode* expression = dn_child(statement1, 0);
    dc_action_on(!test_BooleanLiteral(expression, "true", true), CLOVE_FAIL(),
                 "Wrong boolean literal node");

    dn_program_free(program);
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

    ResultDNode program_res = parser_parse_program(&p);

    dc_action_on(!parser_has_no_error(&p), CLOVE_FAIL(), "parser has error");

    DNode* program = dc_res_val2(program_res);

    dc_action_on(!program_is_valid(program, 1), CLOVE_FAIL(),
                 "program is not valid");

    DNode* statement1 = dn_child(program, 0);
    dc_action_on(!node_is_valid(statement1, DN_EXPRESSION_STATEMENT, 1),
                 CLOVE_FAIL(), "statement is not valid");

    DNode* expression = dn_child(statement1, 0);
    dc_action_on(!test_IntegerLiteral(expression, "5", 5), CLOVE_FAIL(),
                 "Wrong integer value node");

    dn_program_free(program);
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
    ExpressionTest tests[] = {
        {"!5", "!", "5", 5, "", 0},
        {"-15", "-", "15", 15, "", 0},
        {"!false", "!", "false", false, "", 0},
        {"!true", "!", "true", true, "", 0},
    };

    for (usize i = 0; i < dc_count(tests); ++i)
    {
        Scanner s;
        scanner_init(&s, tests[i].input);

        Parser p;
        parser_init(&p, &s);

        ResultDNode program_res = parser_parse_program(&p);

        dc_action_on(!parser_has_no_error(&p), CLOVE_FAIL(),
                     "parser has error");

        DNode* program = dc_res_val2(program_res);

        dc_action_on(!program_is_valid(program, 1), CLOVE_FAIL(),
                     "program is not valid");

        DNode* statement1 = dn_child(program, 0);
        dc_action_on(!node_is_valid(statement1, DN_EXPRESSION_STATEMENT, 1),
                     CLOVE_FAIL(), "statement is not valid");

        DNode* prefix = dn_child(statement1, 0);
        dc_action_on(!node_is_valid(prefix, DN_PREFIX_EXPRESSION, 1),
                     CLOVE_FAIL(), "prefix expression is not valid");

        dc_action_on(!dc_sv_str_eq(prefix->token->text, tests[i].operator),
                     CLOVE_FAIL(), "operator is not '%s', got='" DCPRIsv "'",
                     tests[i].operator, dc_sv_fmt(prefix->token->text));

        DNode* value = dn_child(prefix, 0);
        dc_action_on(!test_Literal(value, tests[i].lval_str, tests[i].lval),
                     CLOVE_FAIL(), "Wrong literal value node");

        dn_program_free(program);
        parser_free(&p);
    }

    CLOVE_PASS();
}

CLOVE_TEST(infix_expressions)
{
    ExpressionTest tests[] = {
        {"5 + 5", "+", "5", 5, "5", 5},
        {"5 - 5", "-", "5", 5, "5", 5},
        {"5 * 5", "*", "5", 5, "5", 5},
        {"5 / 5", "/", "5", 5, "5", 5},
        {"5 > 5", ">", "5", 5, "5", 5},
        {"5 < 5", "<", "5", 5, "5", 5},
        {"5 == 5", "==", "5", 5, "5", 5},
        {"5 != 5", "!=", "5", 5, "5", 5},
        {"true == true", "==", "true", true, "true", true},
        {"true != false", "!=", "true", true, "false", false},
        {"false == false", "==", "false", false, "false", false},
    };

    for (usize i = 0; i < dc_count(tests); ++i)
    {
        Scanner s;
        scanner_init(&s, tests[i].input);

        Parser p;
        parser_init(&p, &s);

        ResultDNode program_res = parser_parse_program(&p);

        dc_action_on(!parser_has_no_error(&p), CLOVE_FAIL(),
                     "parser has error");

        DNode* program = dc_res_val2(program_res);

        dc_action_on(!program_is_valid(program, 1), CLOVE_FAIL(),
                     "program is not valid");

        DNode* statement1 = dn_child(program, 0);
        dc_action_on(!node_is_valid(statement1, DN_EXPRESSION_STATEMENT, 1),
                     CLOVE_FAIL(), "statement is not valid");

        DNode* infix = dn_child(statement1, 0);
        dc_action_on(!node_is_valid(infix, DN_INFIX_EXPRESSION, 2),
                     CLOVE_FAIL(), "infix expression is not valid");

        dc_action_on(!dc_sv_str_eq(infix->token->text, tests[i].operator),
                     CLOVE_FAIL(), "operator is not '%s', got='" DCPRIsv "'",
                     tests[i].operator, dc_sv_fmt(infix->token->text));

        DNode* lval = dn_child(infix, 0);
        dc_action_on(!test_Literal(lval, tests[i].lval_str, tests[i].lval),
                     CLOVE_FAIL(), "Wrong integer value node for left value");

        DNode* rval = dn_child(infix, 1);
        dc_action_on(!test_Literal(rval, tests[i].rval_str, tests[i].rval),
                     CLOVE_FAIL(), "Wrong integer value node for right value");

        dn_program_free(program);
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

        "true",
        "true\n",

        "false",
        "false\n",

        "3 > 5 == false",
        "((3 > 5) == false)\n",

        "3 < 5 == true",
        "((3 < 5) == true)\n",

        "1 + (2 + 3) + 4",
        "((1 + (2 + 3)) + 4)\n",

        "(5 + 5) * 2",
        "((5 + 5) * 2)\n",

        "2 / (5 + 5)",
        "(2 / (5 + 5))\n",

        "-(5 + 5)",
        "(-(5 + 5))\n",

        "!(true == true)",
        "(!(true == true))\n",
    };

    for (usize i = 0; i < dc_count(tests) / 2; ++i)
    {
        string input = tests[i * 2];
        string expected = tests[(i * 2) + 1];

        Scanner s;
        scanner_init(&s, input);

        Parser p;
        parser_init(&p, &s);

        ResultDNode program_res = parser_parse_program(&p);

        dc_action_on(!parser_has_no_error(&p), CLOVE_FAIL(),
                     "parser has error");

        DNode* program = dc_res_val2(program_res);

        dn_string_init(program);

        dc_action_on(strcmp(expected, program->text) != 0, CLOVE_FAIL(),
                     "expected=%s, got=%s", expected, program->text);

        dn_program_free(program);
        parser_free(&p);
    }

    CLOVE_PASS();
}

CLOVE_TEST(if_statement)
{
    const string input = "if (x < y) { x }";

    Scanner s;
    scanner_init(&s, input);

    Parser p;
    parser_init(&p, &s);

    ResultDNode program_res = parser_parse_program(&p);

    dc_action_on(!parser_has_no_error(&p), CLOVE_FAIL(), "parser has error");

    DNode* program = dc_res_val2(program_res);

    dc_action_on(!program_is_valid(program, 1), CLOVE_FAIL(),
                 "program is not valid");

    DNode* statement1 = dn_child(program, 0);
    dc_action_on(!node_is_valid(statement1, DN_EXPRESSION_STATEMENT, 1),
                 CLOVE_FAIL(), "statement is not valid");

    DNode* if_expr = dn_child(statement1, 0);
    dc_action_on(!node_is_valid(if_expr, DN_IF_EXPRESSION, 2), CLOVE_FAIL(),
                 "expression is not if expression");

    // TEST IF CONDITION
    DNode* if_condition = dn_child(if_expr, 0);
    dc_action_on(!node_is_valid(if_condition, DN_INFIX_EXPRESSION, 2),
                 CLOVE_FAIL(), "if_condition expression is not valid");

    dc_action_on(!dc_sv_str_eq(if_condition->token->text, "<"), CLOVE_FAIL(),
                 "operator is not '%s', got='" DCPRIsv "'", "<",
                 dc_sv_fmt(if_condition->token->text));

    DNode* lval = dn_child(if_condition, 0);
    dc_action_on(!node_is_valid(lval, DN_IDENTIFIER, 0), CLOVE_FAIL(),
                 "lval is not valid");

    dc_action_on(!dc_sv_str_eq(lval->token->text, "x"), CLOVE_FAIL(),
                 "identifier value is not '%s', got='" DCPRIsv "'", "x",
                 dc_sv_fmt(lval->token->text));

    DNode* rval = dn_child(if_condition, 1);
    dc_action_on(!node_is_valid(rval, DN_IDENTIFIER, 0), CLOVE_FAIL(),
                 "rval is not valid");

    dc_action_on(!dc_sv_str_eq(rval->token->text, "y"), CLOVE_FAIL(),
                 "identifier value is not '%s', got='" DCPRIsv "'", "y",
                 dc_sv_fmt(rval->token->text));


    // TEST IF CONSEQUENCE
    DNode* if_consequence = dn_child(if_expr, 1);
    dc_action_on(!node_is_valid(if_consequence, DN_BLOCK_STATEMENT, 1),
                 CLOVE_FAIL(), "if_consequence is not valid");

    dn_string_init(if_consequence);

    dc_action_on(strcmp(if_consequence->text, "{ x; }") != 0, CLOVE_FAIL(),
                 "Wrong consequence");

    dn_program_free(program);
    parser_free(&p);

    CLOVE_PASS();
}

CLOVE_TEST(if_else_statement)
{
    const string input = "if x < y { x } else { y }";

    Scanner s;
    scanner_init(&s, input);

    Parser p;
    parser_init(&p, &s);

    ResultDNode program_res = parser_parse_program(&p);

    dc_action_on(!parser_has_no_error(&p), CLOVE_FAIL(), "parser has error");

    DNode* program = dc_res_val2(program_res);

    dc_action_on(!program_is_valid(program, 1), CLOVE_FAIL(),
                 "program is not valid");

    DNode* statement1 = dn_child(program, 0);
    dc_action_on(!node_is_valid(statement1, DN_EXPRESSION_STATEMENT, 1),
                 CLOVE_FAIL(), "statement is not valid");

    dn_string_init(program);

    const string expected = "if (x < y) { x; } else { y; }\n";

    dc_action_on(strcmp(program->text, expected) != 0, CLOVE_FAIL(),
                 "expected='%s', got='%s'", expected, program->text);

    dn_program_free(program);
    parser_free(&p);

    CLOVE_PASS();
}
