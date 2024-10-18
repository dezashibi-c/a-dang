#define CLOVE_SUITE_NAME dang_evaluator_tests

#include "clove-unit/clove-unit.h"

#include "evaluator.h"
#include "parser.h"
#include "scanner.h"
#include "types.h"

typedef struct
{
    string input;
    DObj expected;
} TestCase;

#define DC_STOPPER_TestCase ((TestCase){"", dobj(DOBJ_NULL, voidptr, NULL)})
#define DC_IS_STOPPER_TestCase(EL) (strlen((EL).input) == 0)


static bool dang_parser_has_no_error(DParser* p)
{
    dc_action_on(p->errors.count != 0, dang_parser_log_errors(p);
                 return false, "parser has " dc_fmt(usize) " errors", p->errors.count);

    return true;
}

static ResObj test_eval(string input, DEnv* de)
{
    DC_RES2(ResObj);

    DScanner s;
    dang_scanner_init(&s, input);

    DParser p;
    dang_parser_init(&p, &s);

    ResNode program_res = dang_parser_parse_program(&p);
    dang_parser_free(&p);

    DNode* program = dc_res_val2(program_res);

    if (!dang_parser_has_no_error(&p))
    {
        dn_program_free(program);
        dang_parser_free(&p);

        dc_res_ret_ea(-1, "parser has error on input '%s'", input);
    }

    dc_try_or_fail_with(dang_eval(program, de), {
        dn_program_free(program);
        dang_parser_free(&p);
    });

    dc_res_ret();
}

static bool test_evaluated_literal(DObj* obj, DObj* expected)
{
    if (obj->type != expected->type)
    {
        dc_log("object types are not match: expected=%s, got=%s", tostr_DObjType(expected->type), tostr_DObjType(obj->type));
        return false;
    }

    DCResBool res = dc_dv_eq(&obj->dv, &expected->dv);
    if (dc_res_is_err2(res))
    {
        dc_res_err_log2(res, "cannot compare dynamic values");
        return false;
    }

    if (!dc_res_val2(res))
    {
        DCResString obj_str, expected_str;

        obj_str = dc_tostr_dv(&obj->dv);
        expected_str = dc_tostr_dv(&expected->dv);

        dc_log("expected '%s' but got '%s'", dc_res_val2(expected_str), dc_res_val2(obj_str));

        if (dc_res_val2(obj_str)) free(dc_res_val2(obj_str));
        if (dc_res_val2(expected_str)) free(dc_res_val2(expected_str));
    }

    return dc_res_val2(res);
}

static bool test_evaluated_array_literal(DObj* obj, DObj* expected)
{
    dc_da_for(expected->children, {
        if (!test_evaluated_literal(dobj_child(obj, _idx), dobj_child(expected, _idx))) return false;
    });

    return true;
}

static bool perform_evaluation_tests(TestCase tests[])
{
    dc_foreach(tests, TestCase, {
        ResEnv de_res = dang_env_new();

        if (dc_res_is_err2(de_res))
        {
            dc_log("test '" dc_fmt(usize) "' failed", _idx);
            dc_res_err_log2(de_res, "initializing environment error");
            return false;
        }

        DEnv* de = dc_res_val2(de_res);
        ResObj res = test_eval(_it->input, de);
        if (dc_res_is_err2(res))
        {
            dc_log("test '" dc_fmt(usize) "' failed", _idx);
            dc_res_err_log2(res, "evaluation failed");
            dang_env_free(de);

            return false;
        }

        if (_it->expected.type == DOBJ_ARRAY)
            dc_action_on(!test_evaluated_array_literal(dc_res_val2(res), &_it->expected), return false,
                         "array test '" dc_fmt(usize) "' failed: wrong evaluation result", _idx);
        else
            dc_action_on(!test_evaluated_literal(dc_res_val2(res), &_it->expected), return false,
                         "literal test '" dc_fmt(usize) "' failed: wrong evaluation result", _idx);

        dang_env_free(de);
    });


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

        {.input = "", .expected = dobj(DOBJ_NULL, voidptr, NULL)},
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

        {.input = "'hello' == 'hello'", .expected = dobj_true},

        {.input = "", .expected = dobj(DOBJ_NULL, voidptr, NULL)},
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
        {.input = "true", .expected = dobj_true},

        {.input = "false", .expected = dobj_false},

        {.input = "!false", .expected = dobj_true},

        {.input = "!true", .expected = dobj_false},

        {.input = "!5", .expected = dobj_false},

        {.input = "!!true", .expected = dobj_true},

        {.input = "!!false", .expected = dobj_false},

        {.input = "!!5", .expected = dobj_true},

        {.input = "1 < 2", .expected = dobj_true},

        {.input = "1 > 2", .expected = dobj_false},

        {.input = "1 > 1", .expected = dobj_false},

        {.input = "1 < 1", .expected = dobj_false},

        {.input = "1 == 1", .expected = dobj_true},

        {.input = "1 != 1", .expected = dobj_false},

        {.input = "1 == 2", .expected = dobj_false},

        {.input = "1 != 2", .expected = dobj_true},

        {.input = "true == true", .expected = dobj_true},

        {.input = "false == false", .expected = dobj_true},

        {.input = "true == false", .expected = dobj_false},

        {.input = "true != false", .expected = dobj_true},

        {.input = "false != true", .expected = dobj_true},

        {.input = "(1 < 2) == true", .expected = dobj_true},

        {.input = "(1 < 2) == false", .expected = dobj_false},

        {.input = "(1 > 2) == true", .expected = dobj_false},

        {.input = "(1 > 2) == false", .expected = dobj_true},

        {.input = "", .expected = dobj(DOBJ_NULL, voidptr, NULL)},
    };

    if (perform_evaluation_tests(tests))
        CLOVE_PASS();
    else
    {
        dc_log("test has failed");
        CLOVE_FAIL();
    }
}

CLOVE_TEST(array_literal)
{
    DObj expected_result = {0};
    dang_obj_init(&expected_result, DOBJ_ARRAY, dc_dv_nullptr(), NULL, false, true);

    DObj o1 = dobj_int(1);
    DObj o2 = dobj_int(4);
    DObj o3 = dobj_int(6);

    dc_da_push(&expected_result.children, dc_dv(voidptr, &o1));
    dc_da_push(&expected_result.children, dc_dv(voidptr, &o2));
    dc_da_push(&expected_result.children, dc_dv(voidptr, &o3));

    TestCase tests[] = {
        {.input = "[1 2 * 2 3 + 3]", .expected = expected_result},

        {.input = "let a [1 2 * 2 3 + 3]; a", .expected = expected_result},

        {.input = "", .expected = dobj(DOBJ_NULL, voidptr, NULL)},
    };

    if (perform_evaluation_tests(tests))
        CLOVE_PASS();
    else
    {
        dc_log("test has failed");
        CLOVE_FAIL();
    }
}

CLOVE_TEST(array_index_expressions)
{
    TestCase tests[] = {
        {.input = "[1 2 3][0]", .expected = dobj_int(1)},

        {.input = "[1 2 3][1]", .expected = dobj_int(2)},

        {.input = "[1 2 3][2]", .expected = dobj_int(3)},

        {.input = "let i 0; [1][i]", .expected = dobj_int(1)},

        {.input = "[1 2 3][1 + 1]", .expected = dobj_int(3)},

        {.input = "let arr [1 2 3]; arr[2]", .expected = dobj_int(3)},

        {.input = "let arr [1 2 3]; arr[0] + arr[1] + arr[2]", .expected = dobj_int(6)},

        {.input = "let arr [1 2 3]; let i arr[0]; arr[i]", .expected = dobj_int(2)},

        {.input = "[1 2 3][99]", .expected = dobj(DOBJ_NULL, voidptr, NULL)},

        {.input = "[1 2 3][3]", .expected = dobj(DOBJ_NULL, voidptr, NULL)},

        {.input = "[1 2 3][-1]", .expected = dobj(DOBJ_NULL, voidptr, NULL)},

        {.input = "", .expected = dobj(DOBJ_NULL, voidptr, NULL)},
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

        {.input = "if false { 10 }", .expected = dobj(DOBJ_NULL, voidptr, NULL)},

        {.input = "if 1 { 10 }", .expected = dobj_int(10)},

        {.input = "if (true) { 10 }", .expected = dobj_int(10)},

        {.input = "if 1 < 2 { 10 }", .expected = dobj_int(10)},

        {.input = "if 1 > 2 { 10 }", .expected = dobj(DOBJ_NULL, voidptr, NULL)},

        {.input = "if 1 > 2 {\n 10 \n\n\n} else {\n\n 20 \n}\n", .expected = dobj_int(20)},

        {.input = "if 1 < 2 { 10 } else { 20 }", .expected = dobj_int(10)},

        {.input = "", .expected = dobj(DOBJ_NULL, voidptr, NULL)},
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

        {.input = "return", .expected = dobj(DOBJ_NULL, voidptr, NULL)},

        {.input = "return 10\n 9", .expected = dobj_int(10)},

        {.input = "return 2 * 5\n 9", .expected = dobj_int(10)},

        {.input = "9\n return 2 * 5\n 9", .expected = dobj_int(10)},

        {.input = "if 10 > 1 {\n if 10 > 1 {\n return 10 \n } \n return 1 \n}", .expected = dobj_int(10)},

        {.input = "", .expected = dobj(DOBJ_NULL, voidptr, NULL)},
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
        {.input = "let a; a", .expected = dobj(DOBJ_NULL, voidptr, NULL)},

        {.input = "let a 5 * 5; a", .expected = dobj_int(25)},

        {.input = "let a 5; let b a; b", .expected = dobj_int(5)},

        {.input = "let a 5; let b a; let c a + b + 5; c", .expected = dobj_int(15)},

        {.input = "", .expected = dobj(DOBJ_NULL, voidptr, NULL)},
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
        {.input = "let my_fn fn() {}; my_fn;", .expected = dobj(DOBJ_NULL, voidptr, NULL)},

        {.input = "let identity fn(x) { x }; identity 5", .expected = dobj_int(5)},

        {.input = "let identity fn(x) { return x }; identity 5", .expected = dobj_int(5)},

        {.input = "let double fn(x) { x * 2 }; double 5", .expected = dobj_int(10)},

        {.input = "let add fn(x, y) { x + y }; add 5 5", .expected = dobj_int(10)},

        {.input = "let add fn(x, y) { x + y }; add 5 + 5 ${add 5 5}", .expected = dobj_int(20)},

        {.input = "", .expected = dobj(DOBJ_NULL, voidptr, NULL)},
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

        {.input = "", .expected = dobj(DOBJ_NULL, voidptr, NULL)},
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

        {.input = "", .expected = dobj(DOBJ_NULL, voidptr, NULL)},
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


    dc_foreach(error_tests, string, {
        ResEnv de_res = dang_env_new();

        if (dc_res_is_err2(de_res))
        {
            dc_res_err_log2(de_res, "initializing environment error");
            CLOVE_FAIL();
        }

        DEnv* de = dc_res_val2(de_res);
        ResObj res = test_eval(*_it, de);
        if (dc_res_is_ok2(res))
        {
            dc_log("test #" dc_fmt(usize) " error, expected input '%s' to have error result but evaluated to ok result", _idx,
                   *_it);

            dang_env_free(de);

            CLOVE_FAIL();
        }
        dang_env_free(de);
    });


    CLOVE_PASS();
}
