#define CLOVE_SUITE_NAME dang_evaluator_tests

#include "clove-unit/clove-unit.h"

#include "dcommon/dcommon.h"
#include "evaluator.h"
#include "parser.h"
#include "scanner.h"

static bool dang_parser_has_no_error(DParser* p)
{
    dc_action_on(p->errors.count != 0, dang_parser_log_errors(p); return false, "parser has %zu errors", p->errors.count);

    return true;
}

static DObjResult test_eval(string input)
{
    DC_RES2(DObjResult);

    DScanner s;
    dang_scanner_init(&s, input);

    DParser p;
    dang_parser_init(&p, &s);

    ResultDNode program_res = dang_parser_parse_program(&p);

    DNode* program = dc_res_val2(program_res);

    if (!dang_parser_has_no_error(&p))
    {
        dn_program_free(program);
        dang_parser_free(&p);

        dc_res_ret_ea(-1, "parser has error on input '%s'", input);
    }

    dc_try_or_fail_with3(DEnvResult, de, dang_env_new(), {
        dn_program_free(program);
        dang_parser_free(&p);
    });

    dc_try_or_fail_with(dang_eval(program, dc_res_val2(de)), {
        dn_program_free(program);
        dang_parser_free(&p);
    });

    dn_program_free(program);
    dang_parser_free(&p);
    dang_env_free(dc_res_val2(de));

    dc_res_ret();
}

static bool test_evaluated_literal(DObject* obj, DObject* expected)
{
    if (obj->type != expected->type)
    {
        dc_log("object types are not match: expected=%s, got=%s", tostr_DObjType(expected->type), tostr_DObjType(obj->type));
        return false;
    }

    DCResultBool res = dc_dv_eq(&obj->dv, &expected->dv);
    if (dc_res_is_err2(res))
    {
        dc_res_err_log2(res, "cannot compare dynamic values");
        return false;
    }

    if (!dc_res_val2(res))
    {
        DCResultString obj_str, expected_str;

        obj_str = dc_tostr_dv(&obj->dv);
        expected_str = dc_tostr_dv(&expected->dv);

        dc_log("expected '%s' but got '%s'", dc_res_val2(expected_str), dc_res_val2(obj_str));

        if (dc_res_val2(obj_str)) free(dc_res_val2(obj_str));
        if (dc_res_val2(expected_str)) free(dc_res_val2(expected_str));
    }

    return dc_res_val2(res);
}

typedef struct
{
    string input;
    DObject expected;
} TestCase;

static bool perform_evaluation_tests(TestCase tests[])
{
    dc_sforeach(tests, TestCase, strlen(_it->input) != 0)
    {
        DObjResult res = test_eval(_it->input);
        if (dc_res_is_err2(res))
        {
            dc_res_err_log2(res, "evaluation failed");
            return false;
        }

        dc_action_on(!test_evaluated_literal(&dc_res_val2(res), &_it->expected), return false, "wrong evaluation result");
    }

    return true;
}

CLOVE_TEST(integer_expressions)
{
    TestCase tests[] = {
        {.input = "5", .expected = dobj_int(5)},

        {.input = "10", .expected = dobj_int(10)},

        {.input = "-5", .expected = dobj_int(-5)},

        {.input = "-10", .expected = dobj_int(-10)},

        {.input = "--10", .expected = dobj_int(10)},

        {.input = "5 + 5 + 5 + 5 - 10", .expected = dobj_int(10)},

        {.input = "2 * 2 * 2 * 2 * 2", .expected = dobj_int(32)},

        {.input = "-50 + 100 + -50", .expected = dobj_int(0)},

        {.input = "5 * 2 + 10", .expected = dobj_int(20)},

        {.input = "5 + 2 * 10", .expected = dobj_int(25)},

        {.input = "20 + 2 * -10", .expected = dobj_int(0)},

        {.input = "50 / 2 * 2 + 10", dobj_int(60)},

        {.input = "2 * (5 + 10)", .expected = dobj_int(30)},

        {.input = "3 * 3 * 3 + 10", .expected = dobj_int(37)},

        {.input = "3 * (3 * 3) + 10", .expected = dobj_int(37)},

        {.input = "(5 + 10 * 2 + 15 / 3) * 2 + -10", .expected = dobj_int(50)},

        {.input = "", .expected = dobj_null()},
    };

    if (perform_evaluation_tests(tests))
        CLOVE_PASS();
    else
    {
        dc_log("test has failed");
        CLOVE_FAIL();
    }
}

CLOVE_TEST(string_literal)
{
    TestCase tests[] = {
        {.input = "'Hello World!'", .expected = dobj_string("Hello World!")},

        {.input = "'Hello' + ' ' + 'World!'", .expected = dobj_string("Hello World!")},

        {.input = "'Hello' + ' ' + 5 + '!'", .expected = dobj_string("Hello 5!")},

        {.input = "5 + ' ' + 'programmers!'", .expected = dobj_string("5 programmers!")},

        {.input = "'hello' == 'hello'", .expected = dobj_true()},

        {.input = "", .expected = dobj_null()},
    };

    if (perform_evaluation_tests(tests))
        CLOVE_PASS();
    else
    {
        dc_log("test has failed");
        CLOVE_FAIL();
    }
}

CLOVE_TEST(boolean_expressions)
{
    TestCase tests[] = {
        {.input = "true", .expected = dobj_true()},

        {.input = "false", .expected = dobj_false()},

        {.input = "!false", .expected = dobj_true()},

        {.input = "!true", .expected = dobj_false()},

        {.input = "!5", .expected = dobj_false()},

        {.input = "!!true", .expected = dobj_true()},

        {.input = "!!false", .expected = dobj_false()},

        {.input = "!!5", .expected = dobj_true()},

        {.input = "1 < 2", .expected = dobj_true()},

        {.input = "1 > 2", .expected = dobj_false()},

        {.input = "1 > 1", .expected = dobj_false()},

        {.input = "1 < 1", .expected = dobj_false()},

        {.input = "1 == 1", .expected = dobj_true()},

        {.input = "1 != 1", .expected = dobj_false()},

        {.input = "1 == 2", .expected = dobj_false()},

        {.input = "1 != 2", .expected = dobj_true()},

        {.input = "true == true", .expected = dobj_true()},

        {.input = "false == false", .expected = dobj_true()},

        {.input = "true == false", .expected = dobj_false()},

        {.input = "true != false", .expected = dobj_true()},

        {.input = "false != true", .expected = dobj_true()},

        {.input = "(1 < 2) == true", .expected = dobj_true()},

        {.input = "(1 < 2) == false", .expected = dobj_false()},

        {.input = "(1 > 2) == true", .expected = dobj_false()},

        {.input = "(1 > 2) == false", .expected = dobj_true()},

        {.input = "", .expected = dobj_null()},
    };

    if (perform_evaluation_tests(tests))
        CLOVE_PASS();
    else
    {
        dc_log("test has failed");
        CLOVE_FAIL();
    }
}

CLOVE_TEST(if_else_expressions)
{
    TestCase tests[] = {
        {.input = "if true { 10 }", .expected = dobj_int(10)},

        {.input = "if false { 10 }", .expected = dobj_null()},

        {.input = "if 1 { 10 }", .expected = dobj_int(10)},

        {.input = "if (true) { 10 }", .expected = dobj_int(10)},

        {.input = "if 1 < 2 { 10 }", .expected = dobj_int(10)},

        {.input = "if 1 > 2 { 10 }", .expected = dobj_null()},

        {.input = "if 1 > 2 {\n 10 \n\n\n} else {\n\n 20 \n}\n", .expected = dobj_int(20)},

        {.input = "if 1 < 2 { 10 } else { 20 }", .expected = dobj_int(10)},

        {.input = "", .expected = dobj_null()},
    };

    if (perform_evaluation_tests(tests))
        CLOVE_PASS();
    else
    {
        dc_log("test has failed");
        CLOVE_FAIL();
    }
}

CLOVE_TEST(return_statement)
{
    TestCase tests[] = {
        {.input = "return 10", .expected = dobj_int(10)},

        {.input = "return", .expected = dobj_null()},

        {.input = "return 10\n 9", .expected = dobj_int(10)},

        {.input = "return 2 * 5\n 9", .expected = dobj_int(10)},

        {.input = "9\n return 2 * 5\n 9", .expected = dobj_int(10)},

        {.input = "if 10 > 1 {\n if 10 > 1 {\n return 10 \n } \n return 1 \n}", .expected = dobj_int(10)},

        {.input = "", .expected = dobj_null()},
    };

    if (perform_evaluation_tests(tests))
        CLOVE_PASS();
    else
    {
        dc_log("test has failed");
        CLOVE_FAIL();
    }
}

CLOVE_TEST(let_statement)
{
    TestCase tests[] = {
        {.input = "let a; a", .expected = dobj_null()},

        {.input = "let a 5 * 5; a", .expected = dobj_int(25)},

        {.input = "let a 5; let b a; b", .expected = dobj_int(5)},

        {.input = "let a 5; let b a; let c a + b + 5; c", .expected = dobj_int(15)},

        {.input = "", .expected = dobj_null()},
    };

    if (perform_evaluation_tests(tests))
        CLOVE_PASS();
    else
    {
        dc_log("test has failed");
        CLOVE_FAIL();
    }
}

CLOVE_TEST(functions)
{
    TestCase tests[] = {
        {.input = "let my_fn fn() {}; my_fn;", .expected = dobj_null()},

        {.input = "let identity fn(x) { x }; identity 5", .expected = dobj_int(5)},

        {.input = "let identity fn(x) { return x }; identity 5", .expected = dobj_int(5)},

        {.input = "let double fn(x) { x * 2 }; double 5", .expected = dobj_int(10)},

        {.input = "let add fn(x, y) { x + y }; add 5 5", .expected = dobj_int(10)},

        {.input = "let add fn(x, y) { x + y }; add 5 + 5 ${add 5 5}", .expected = dobj_int(20)},

        {.input = "", .expected = dobj_null()},
    };

    if (perform_evaluation_tests(tests))
        CLOVE_PASS();
    else
    {
        dc_log("test has failed");
        CLOVE_FAIL();
    }
}

CLOVE_TEST(builtin_functions)
{
    TestCase tests[] = {
        {.input = "len ''", .expected = dobj_int(0)},

        {.input = "len 'four'", .expected = dobj_int(4)},

        {.input = "len 'hello world'", .expected = dobj_int(11)},

        {.input = "", .expected = dobj_null()},
    };

    if (perform_evaluation_tests(tests))
        CLOVE_PASS();
    else
    {
        dc_log("test has failed");
        CLOVE_FAIL();
    }
}

CLOVE_TEST(closures)
{
    TestCase tests[] = {
        {.input = "\n"
                  "let new_adder fn(x) {\n"
                  " fn(y) { x + y }\n"
                  "}\n"
                  "let add_two ${new_adder 2}\n"
                  "add_two 2",
         .expected = dobj_int(4)},

        {.input = "", .expected = dobj_null()},
    };

    if (perform_evaluation_tests(tests))
        CLOVE_PASS();
    else
    {
        dc_log("test has failed");
        CLOVE_FAIL();
    }
}

CLOVE_TEST(error_handling)
{
    string error_tests[] = {
        "5 + true",

        "5 + true\n 5",

        "-true",

        "true + false",

        "5\n true + false\n 5",

        "if 10 > 1 { true + false }",

        "if 10 > 1 { \n if 10 > 1 { \n return true + false \n } \n\n return 1 \n}",

        "foobar", // does not exist

        "let a; let a", // already is defined

        "'Hello' - 'World'",

        "len 1",

        "len;",

        "len 'one' 'two'",

        NULL,
    };

    dc_foreach(error_tests, string)
    {
        DObjResult res = test_eval(*_it);
        if (dc_res_is_ok2(res))
        {
            dc_log("expected input '%s' to have error result but evaluated to ok result", *_it);

            CLOVE_FAIL();
        }
    }

    CLOVE_PASS();
}
