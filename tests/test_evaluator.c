#define CLOVE_SUITE_NAME dang_evaluator_tests

#include "clove-unit/clove-unit.h"

#include "evaluator.h"
#include "parser.h"
#include "scanner.h"

typedef struct
{
    string input;
    DCDynVal expected;
} TestCase;

#define DC_STOPPER_TestCase ((TestCase){"", dc_dv_nullptr()})
#define DC_IS_STOPPER_TestCase(EL) (strlen((EL).input) == 0)

static b1 test_evaluated_literal(DCDynValPtr obj, DCDynValPtr expected)
{
    if (obj->type != expected->type)
    {
        dc_log("object types are not match: expected=%s, got=%s", dv_type_tostr(expected), dv_type_tostr(obj));
        return false;
    }

    DCResBool res = dc_dv_eq(obj, expected);
    if (dc_is_err2(res))
    {
        dc_err_log2(res, "cannot compare dynamic values");
        dc_result_free(&res);
        return false;
    }

    if (!dc_unwrap2(res))
    {
        DCResString obj_str, expected_str;

        obj_str = dc_tostr_dv(obj);
        expected_str = dc_tostr_dv(expected);

        dc_log("expected '%s' but got '%s'", dc_unwrap2(expected_str), dc_unwrap2(obj_str));

        if (dc_unwrap2(obj_str)) free(dc_unwrap2(obj_str));
        if (dc_unwrap2(expected_str)) free(dc_unwrap2(expected_str));
    }

    return dc_unwrap2(res);
}

static b1 test_evaluated_array_literal(DCDynArrPtr obj, DCDynArrPtr expected)
{
    dc_da_for(array_tesT_loop, *expected, {
        if (!test_evaluated_literal(&dc_da_get2(*obj, _idx), _it)) return false;
    });

    return true;
}

static b1 perform_evaluation_tests(TestCase tests[])
{
    dc_foreach(main_test_loop, tests, TestCase, {
        DEvaluator de;
        DCResVoid init_res = dang_evaluator_init(&de);
        if (dc_is_err2(init_res))
        {
            dc_log("Evaluator initialization error on input");

            dc_err_log2(init_res, "error");

            return false;
        }

        ResEvaluated res = dang_eval(&de, _it->input, false);
        if (dc_is_err2(res))
        {
            dc_log("test '" dc_fmt(usize) "' failed", _idx);
            dc_err_log2(res, "evaluation failed");
            dang_parser_log_errors(&de.parser);

            return false;
        }

        DCDynVal result = dc_unwrap2(res).result;

        if (_it->expected.type == dc_dvt(DCDynArrPtr))
            dc_action_on(!test_evaluated_array_literal(dc_dv_as(result, DCDynArrPtr), dc_dv_as(_it->expected, DCDynArrPtr)),
                         dang_evaluator_free(&de);
                         return false, "array test '" dc_fmt(usize) "' failed: wrong evaluation result", _idx);

        else
            dc_action_on(!test_evaluated_literal(&result, &_it->expected), dang_evaluator_free(&de);
                         return false, "literal test '" dc_fmt(usize) "' failed: wrong evaluation result", _idx);

        dang_evaluator_free(&de);
    });


    return true;
}

CLOVE_TEST(integer_expressions)
{
    TestCase tests[] = {
        {.input = "5", .expected = do_int(5)},

        {.input = "10", .expected = do_int(10)},

        {.input = "-5", .expected = do_int(-5)},

        {.input = "-10", .expected = do_int(-10)},

        {.input = "--10", .expected = do_int(10)},

        {.input = "5 + 5 + 5 + 5 - 10", .expected = do_int(10)},

        {.input = "2 * 2 * 2 * 2 * 2", .expected = do_int(32)},

        {.input = "-50 + 100 + -50", .expected = do_int(0)},

        {.input = "5 * 2 + 10", .expected = do_int(20)},

        {.input = "5 + 2 * 10", .expected = do_int(25)},

        {.input = "20 + 2 * -10", .expected = do_int(0)},

        {.input = "50 / 2 * 2 + 10", do_int(60)},

        {.input = "2 * (5 + 10)", .expected = do_int(30)},

        {.input = "3 * 3 * 3 + 10", .expected = do_int(37)},

        {.input = "3 * (3 * 3) + 10", .expected = do_int(37)},

        {.input = "(5 + 10 * 2 + 15 / 3) * 2 + -10", .expected = do_int(50)},

        {.input = "", .expected = dc_dv_nullptr()},
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
        {.input = "'Hello World!'", .expected = do_string("Hello World!")},

        {.input = "'Hello' + ' ' + 'World!'", .expected = do_string("Hello World!")},

        {.input = "'Hello' + ' ' + 5 + '!'", .expected = do_string("Hello 5!")},

        {.input = "5 + ' ' + 'programmers!'", .expected = do_string("5 programmers!")},

        {.input = "'hello' == 'hello'", .expected = dc_dv_bool(true)},

        {.input = "", .expected = dc_dv_nullptr()},
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
        {.input = "true", .expected = dc_dv_bool(true)},

        {.input = "false", .expected = dc_dv_bool(false)},

        {.input = "!false", .expected = dc_dv_bool(true)},

        {.input = "!true", .expected = dc_dv_bool(false)},

        {.input = "!5", .expected = dc_dv_bool(false)},

        {.input = "!!true", .expected = dc_dv_bool(true)},

        {.input = "!!false", .expected = dc_dv_bool(false)},

        {.input = "!!5", .expected = dc_dv_bool(true)},

        {.input = "1 < 2", .expected = dc_dv_bool(true)},

        {.input = "1 > 2", .expected = dc_dv_bool(false)},

        {.input = "1 > 1", .expected = dc_dv_bool(false)},

        {.input = "1 < 1", .expected = dc_dv_bool(false)},

        {.input = "1 == 1", .expected = dc_dv_bool(true)},

        {.input = "1 != 1", .expected = dc_dv_bool(false)},

        {.input = "1 == 2", .expected = dc_dv_bool(false)},

        {.input = "1 != 2", .expected = dc_dv_bool(true)},

        {.input = "true == true", .expected = dc_dv_bool(true)},

        {.input = "false == false", .expected = dc_dv_bool(true)},

        {.input = "true == false", .expected = dc_dv_bool(false)},

        {.input = "true != false", .expected = dc_dv_bool(true)},

        {.input = "false != true", .expected = dc_dv_bool(true)},

        {.input = "(1 < 2) == true", .expected = dc_dv_bool(true)},

        {.input = "(1 < 2) == false", .expected = dc_dv_bool(false)},

        {.input = "(1 > 2) == true", .expected = dc_dv_bool(false)},

        {.input = "(1 > 2) == false", .expected = dc_dv_bool(true)},

        {.input = "", .expected = dc_dv_nullptr()},
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
    DCDynArr expected_result_arr = {0};
    dc_da_init2(&expected_result_arr, 3, 3, NULL);

    dc_da_push(&expected_result_arr, do_int(1));
    dc_da_push(&expected_result_arr, do_int(4));
    dc_da_push(&expected_result_arr, do_int(6));

    TestCase tests[] = {
        {.input = "[1 2 * 2 3 + 3]", .expected = dc_dv(DCDynArrPtr, &expected_result_arr)},

        {.input = "let a [1 2 * 2 3 + 3]; a", .expected = dc_dv(DCDynArrPtr, &expected_result_arr)},

        {.input = "", .expected = dc_dv_nullptr()},
    };

    if (perform_evaluation_tests(tests))
    {
        dc_da_free(&expected_result_arr);
        CLOVE_PASS();
    }
    else
    {
        dc_log("test has failed");
        CLOVE_FAIL();
    }
}

CLOVE_TEST(array_index_expressions)
{
    TestCase tests[] = {
        {.input = "[1 2 3][0]", .expected = do_int(1)},

        {.input = "[1 2 3][1]", .expected = do_int(2)},

        {.input = "[1 2 3][2]", .expected = do_int(3)},

        {.input = "let i 0; [1][i]", .expected = do_int(1)},

        {.input = "[1 2 3][1 + 1]", .expected = do_int(3)},

        {.input = "let arr [1 2 3]; arr[2]", .expected = do_int(3)},

        {.input = "let arr2 [1 2 3]; arr2[0] + arr2[1] + arr2[2]", .expected = do_int(6)},

        {.input = "let arr3 [1 2 3]; let i2 arr3[0]; arr3[i2]", .expected = do_int(2)},

        {.input = "[1 2 3][99]", .expected = dc_dv_nullptr()},

        {.input = "[1 2 3][3]", .expected = dc_dv_nullptr()},

        {.input = "[1 2 3][-1]", .expected = dc_dv_nullptr()},

        {.input = "", .expected = dc_dv_nullptr()},
    };

    if (perform_evaluation_tests(tests))
        CLOVE_PASS();
    else
    {
        dc_log("test has failed");
        CLOVE_FAIL();
    }
}

CLOVE_TEST(hash_index_expression)
{
#define FIXED_INPUT                                                                                                            \
    "let two 'two'\n"                                                                                                          \
    "{\n"                                                                                                                      \
    " 'one': 10 - 9,\n"                                                                                                        \
    " 'thr' + 'ee': 6 / 2,\n"                                                                                                  \
    " 4: 4,\n"                                                                                                                 \
    " true: 5,\n"                                                                                                              \
    " false: 6\n"                                                                                                              \
    "}"

#define FIXED_INPUT2                                                                                                           \
    "let two 'two'\n"                                                                                                          \
    "let a {\n"                                                                                                                \
    " 'one': 10 - 9,\n"                                                                                                        \
    " 'thr' + 'ee': 6 / 2,\n"                                                                                                  \
    " 4: 4,\n"                                                                                                                 \
    " true: 5,\n"                                                                                                              \
    " false: 6\n"                                                                                                              \
    "};"

    TestCase tests[] = {
        {.input = FIXED_INPUT "['one']", .expected = do_int(1)},

        {.input = FIXED_INPUT "['three']", .expected = do_int(3)},

        {.input = FIXED_INPUT "[4]", .expected = do_int(4)},

        {.input = FIXED_INPUT "[true]", .expected = do_int(5)},

        {.input = FIXED_INPUT "[false]", .expected = do_int(6)},


        {.input = FIXED_INPUT2 "a['one']", .expected = do_int(1)},

        {.input = FIXED_INPUT2 "a['three']", .expected = do_int(3)},

        {.input = FIXED_INPUT2 "a[4]", .expected = do_int(4)},

        {.input = FIXED_INPUT2 "a[true]", .expected = do_int(5)},

        {.input = FIXED_INPUT2 "a[false]", .expected = do_int(6)},

        {.input = "{'foo': 5}['foo']", .expected = do_int(5)},

        {.input = "{'foo': 5}['bar']", .expected = dc_dv_nullptr()},

        {.input = "let key 'foo'; {'foo': 5}[key]", .expected = do_int(5)},

        {.input = "{}['foo']", .expected = dc_dv_nullptr()},

        {.input = "{5: 5}[5]", .expected = do_int(5)},

        {.input = "{true: 5}[true]", .expected = do_int(5)},

        {.input = "{false: 5}[false]", .expected = do_int(5)},


        {.input = "", .expected = dc_dv_nullptr()},
    };

    if (perform_evaluation_tests(tests))
        CLOVE_PASS();
    else
    {
        dc_log("test has failed");
        CLOVE_FAIL();
    }

#undef FIXED_INPUT
#undef FIXED_INPUT2
}

CLOVE_TEST(if_else_expressions)
{
    TestCase tests[] = {
        {.input = "if true { 10 }", .expected = do_int(10)},

        {.input = "if false { 10 }", .expected = dc_dv_nullptr()},

        {.input = "if 1 { 10 }", .expected = do_int(10)},

        {.input = "if (true) { 10 }", .expected = do_int(10)},

        {.input = "if 1 < 2 { 10 }", .expected = do_int(10)},

        {.input = "if 1 > 2 { 10 }", .expected = dc_dv_nullptr()},

        {.input = "if 1 > 2 {\n 10 \n\n\n} else {\n\n 20 \n}\n", .expected = do_int(20)},

        {.input = "if 1 < 2 { 10 } else { 20 }", .expected = do_int(10)},

        {.input = "", .expected = dc_dv_nullptr()},
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
        {.input = "return 10", .expected = do_int(10)},

        {.input = "return", .expected = dc_dv_nullptr()},

        {.input = "return 10\n 9", .expected = do_int(10)},

        {.input = "return 2 * 5\n 9", .expected = do_int(10)},

        {.input = "9\n return 2 * 5\n 9", .expected = do_int(10)},

        {.input = "if 10 > 1 {\n if 10 > 1 {\n return 10 \n } \n return 1 \n}", .expected = do_int(10)},

        {.input = "", .expected = dc_dv_nullptr()},
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
        {.input = "let a; a", .expected = dc_dv_nullptr()},

        {.input = "let b 5 * 5; b", .expected = do_int(25)},

        {.input = "let c 5; let d c; d", .expected = do_int(5)},

        {.input = "let a2 5; let b2 a2; let c2 a2 + b2 + 5; c2", .expected = do_int(15)},

        {.input = "", .expected = dc_dv_nullptr()},
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
        {.input = "let my_fn fn() {}; my_fn;", .expected = dc_dv_nullptr()},

        {.input = "let identity fn(x) { x }; identity 5", .expected = do_int(5)},

        {.input = "let identity2 fn(x) { return x }; identity2 5", .expected = do_int(5)},

        {.input = "let double fn(x) { x * 2 }; double 5", .expected = do_int(10)},

        {.input = "let add fn(x, y) { x + y }; add 5 5", .expected = do_int(10)},

        {.input = "let add2 fn(x, y) { x + y }; add2 5 + 5 ${add2 5 5}", .expected = do_int(20)},

        {.input = "", .expected = dc_dv_nullptr()},
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
        {.input = "len ''", .expected = do_int(0)},

        {.input = "len 'four'", .expected = do_int(4)},

        {.input = "len 'hello world'", .expected = do_int(11)},

        {.input = "len, [1 2 3 4 5]", .expected = do_int(5)},

        {.input = "", .expected = dc_dv_nullptr()},
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
         .expected = do_int(4)},

        {.input = "", .expected = dc_dv_nullptr()},
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

        "{fn(x) { x }: 'Monkey'}", // function cannot be used as key

        "{'name': 'Monkey'}[fn(x) { x }]", // function cannot be used as key

        NULL,
    };


    dc_foreach(error_checking_loop, error_tests, string, {
        DEvaluator de;
        DCResVoid init_res = dang_evaluator_init(&de);
        if (dc_is_err2(init_res))
        {
            dc_log("Evaluator initialization error on input");

            dc_err_log2(init_res, "error");

            CLOVE_FAIL();
        }

        ResEvaluated res = dang_eval(&de, *_it, false);
        if (dc_is_ok2(res))
        {
            dc_log("test #" dc_fmt(usize) " error, expected input '%s' to have error result but evaluated to ok result", _idx,
                   *_it);

            CLOVE_FAIL();
        }
    });

    CLOVE_PASS();
}
