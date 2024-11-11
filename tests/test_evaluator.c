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

        else if (result.type == DO_QUOTE)
        {
            DCResString str_res = do_tostr(&result);
            dc_action_on(dc_is_err2(str_res), dang_evaluator_free(&de);
                         return false, "quote test '" dc_fmt(usize) "' failed: string conversion failed", _idx);

            bool comparison_result = strcmp(dc_unwrap2(str_res), dc_dv_as(_it->expected, string)) == 0;
            if (!comparison_result)
            {
                dc_log("quote/unquote test '" dc_fmt(usize) "' failed: expected '%s' bot got '%s'", _idx,
                       dc_dv_as(_it->expected, string), dc_unwrap2(str_res));
            }
            free(dc_unwrap2(str_res));
        }

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

CLOVE_TEST(quote)
{
    TestCase tests[] = {
        {.input = "quote 5", .expected = do_string("QUOTE(5)")},

        {.input = "quote 5 + 8", .expected = do_string("QUOTE((5 + 8))")},

        {.input = "quote foobar", .expected = do_string("QUOTE(foobar)")},

        {.input = "quote foobar + baz", .expected = do_string("QUOTE((foobar + baz))")},


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

static DECL_DNODE_MODIFIER_FN(modifier_fn)
{
    DC_RES();

    (void)de;
    (void)env;

    if (dn->type != DO_INTEGER) dc_ret_ok(*dn);

    i64 value = do_as_int(*dn);
    if (value != 1) dc_ret_ok(*dn);

    dc_ret_ok(do_int(2));
}

CLOVE_TEST(modify_program_node)
{
    DCDynVal one = do_int(1);
    DCDynVal two = do_int(2);

    DCDynArr statements;
    dc_da_init(&statements, NULL);

    dc_da_push(&statements, one);
    dc_da_push(&statements, two);

    DCDynVal program = dc_dv(DNodeProgram, dn_program(&statements));

    DCRes modified = dn_modify(NULL, &program, NULL, modifier_fn);
    if (dc_is_err2(modified))
    {
        dc_err_log2(modified, "program modify test failed");
        CLOVE_FAIL();
    }

    // verifications
    DCDynVal modified_node = dc_unwrap2(modified);
    CLOVE_IS_TRUE(modified_node.type == dc_dvt(DNodeProgram));
    DNodeProgram modified_program = dc_dv_as(modified_node, DNodeProgram);

    // tests
    CLOVE_IS_TRUE(modified_program.statements->elements[0].value.i64_val == 2);
    CLOVE_IS_TRUE(modified_program.statements->elements[1].value.i64_val == 2);

    dc_da_free(&statements);

    CLOVE_PASS();
}

CLOVE_TEST(modify_infix_node)
{
    DCDynVal one = do_int(1);
    DCDynVal two = do_int(2);

    DCDynVal infix = dc_dv(DNodeInfixExpression, dn_infix("+", &one, &two));

    DCRes modified_res = dn_modify(NULL, &infix, NULL, modifier_fn);
    if (dc_is_err2(modified_res))
    {
        dc_err_log2(modified_res, "infix modify test failed");
        CLOVE_FAIL();
    }

    // verifications
    DCDynVal modified = dc_unwrap2(modified_res);
    CLOVE_IS_TRUE(modified.type == dc_dvt(DNodeInfixExpression));
    DNodeInfixExpression modified_node = dc_dv_as(modified, DNodeInfixExpression);

    // tests
    CLOVE_IS_TRUE(modified_node.left->value.i64_val == 2);
    CLOVE_IS_TRUE(modified_node.right->value.i64_val == 2);

    CLOVE_PASS();
}

CLOVE_TEST(modify_prefix_node)
{
    DCDynVal one = do_int(1);

    DCDynVal prefix = dc_dv(DNodePrefixExpression, dn_prefix("-", &one));

    DCRes modified_res = dn_modify(NULL, &prefix, NULL, modifier_fn);
    if (dc_is_err2(modified_res))
    {
        dc_err_log2(modified_res, "prefix modify test failed");
        CLOVE_FAIL();
    }

    // verifications
    DCDynVal modified = dc_unwrap2(modified_res);
    CLOVE_IS_TRUE(modified.type == dc_dvt(DNodePrefixExpression));
    DNodePrefixExpression modified_node = dc_dv_as(modified, DNodePrefixExpression);

    // tests
    CLOVE_IS_TRUE(modified_node.operand->value.i64_val == 2);

    CLOVE_PASS();
}

CLOVE_TEST(modify_index_node)
{
    DCDynVal one = do_int(1);

    DCDynVal index = dc_dv(DNodeIndexExpression, dn_index(&one, &one));

    DCRes modified_res = dn_modify(NULL, &index, NULL, modifier_fn);
    if (dc_is_err2(modified_res))
    {
        dc_err_log2(modified_res, "index modify test failed");
        CLOVE_FAIL();
    }

    // verifications
    DCDynVal modified = dc_unwrap2(modified_res);
    CLOVE_IS_TRUE(modified.type == dc_dvt(DNodeIndexExpression));
    DNodeIndexExpression modified_node = dc_dv_as(modified, DNodeIndexExpression);

    // tests
    CLOVE_IS_TRUE(modified_node.operand->value.i64_val == 2);
    CLOVE_IS_TRUE(modified_node.index->value.i64_val == 2);

    CLOVE_PASS();
}

CLOVE_TEST(modify_return_node)
{
    DCDynVal one = do_int(1);

    DCDynVal return_node = dc_dv(DNodeReturnStatement, dn_return(&one));

    DCRes modified_res = dn_modify(NULL, &return_node, NULL, modifier_fn);
    if (dc_is_err2(modified_res))
    {
        dc_err_log2(modified_res, "return modify test failed");
        CLOVE_FAIL();
    }

    // verifications
    DCDynVal modified = dc_unwrap2(modified_res);
    CLOVE_IS_TRUE(modified.type == dc_dvt(DNodeReturnStatement));
    DNodeReturnStatement modified_node = dc_dv_as(modified, DNodeReturnStatement);

    // tests
    CLOVE_IS_TRUE(modified_node.ret_val->value.i64_val == 2);

    CLOVE_PASS();
}

CLOVE_TEST(modify_if_expression_node)
{
    DCDynVal one = do_int(1);

    DCDynArr cons;
    dc_da_init(&cons, NULL);

    DCDynArr alt;
    dc_da_init(&alt, NULL);

    dc_da_push(&cons, one);
    dc_da_push(&cons, one);

    dc_da_push(&alt, one);
    dc_da_push(&alt, one);


    DCDynVal if_expression_node = dc_dv(DNodeIfExpression, dn_if(&one, &cons, &alt));

    DCRes modified_res = dn_modify(NULL, &if_expression_node, NULL, modifier_fn);
    if (dc_is_err2(modified_res))
    {
        dc_err_log2(modified_res, "if expression modify test failed");
        CLOVE_FAIL();
    }

    // verifications
    DCDynVal modified = dc_unwrap2(modified_res);
    CLOVE_IS_TRUE(modified.type == dc_dvt(DNodeIfExpression));
    DNodeIfExpression modified_node = dc_dv_as(modified, DNodeIfExpression);

    // tests
    CLOVE_IS_TRUE(modified_node.condition->value.i64_val == 2);

    CLOVE_IS_TRUE(modified_node.consequence->elements[0].value.i64_val == 2);
    CLOVE_IS_TRUE(modified_node.consequence->elements[1].value.i64_val == 2);

    CLOVE_IS_TRUE(modified_node.alternative->elements[0].value.i64_val == 2);
    CLOVE_IS_TRUE(modified_node.alternative->elements[1].value.i64_val == 2);

    dc_da_free(&cons);
    dc_da_free(&alt);

    CLOVE_PASS();
}

CLOVE_TEST(quote_unquote)
{
    TestCase tests[] = {
        {.input = "quote ${unquote 4}", .expected = do_string("QUOTE(4)")},

        {.input = "quote ${unquote 4 + 4}", .expected = do_string("QUOTE(8)")},

        {.input = "quote 8 + ${unquote 4 + 4}", .expected = do_string("QUOTE((8 + 8))")},

        {.input = "quote ${unquote 4 + 4} + 8", .expected = do_string("QUOTE((8 + 8))")},

        {.input = "let foobar 8; quote foobar", .expected = do_string("QUOTE(foobar)")},

        {.input = "let foobar 8; quote ${unquote 4}", .expected = do_string("QUOTE(4)")},

        {.input = "quote ${unquote true}", .expected = do_string("QUOTE(true)")},

        {.input = "quote ${unquote true == false}", .expected = do_string("QUOTE(false)")},

        {.input = "quote ${unquote ${quote 4 + 4}}", .expected = do_string("QUOTE((4 + 4))")},

        {.input = "let quoted_infix ${quote 4 + 4}; quote ${unquote 4 + 4} + ${unquote quoted_infix}",
         .expected = do_string("QUOTE((8 + (4 + 4)))")},


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

CLOVE_TEST(macro_definition)
{
    string input = "let number 1; let function fn (x y) { x + y }; let my_macro macro (x y) { x + y }";

    DEvaluator de;
    DCResVoid init_res = dang_evaluator_init(&de);
    if (dc_is_err2(init_res))
    {
        dc_log("Evaluator initialization error on input");

        dc_err_log2(init_res, "error");

        CLOVE_FAIL();
    }

    ResDNodeProgram res = dang_define_macros(&de, input);
    if (dc_is_err2(res))
    {
        dc_err_log2(res, "macro definition failed");

        dang_parser_log_errors(&de.parser);

        CLOVE_FAIL();
    }

    DCRes env_res = dang_env_get(&de.main_env, "number");
    CLOVE_IS_TRUE(dc_is_err2(env_res));
    CLOVE_IS_TRUE(dc_err_code2(env_res) == dc_e_code(NF));

    env_res = dang_env_get(&de.main_env, "function");
    CLOVE_IS_TRUE(dc_is_err2(env_res));
    CLOVE_IS_TRUE(dc_err_code2(env_res) == dc_e_code(NF));

    env_res = dang_env_get(&de.macro_env, "my_macro");
    CLOVE_IS_TRUE(dc_is_ok2(env_res));
    CLOVE_IS_TRUE(dc_unwrap2(env_res).type == dc_dvt(DNodeMacro));

    DNodeMacro macro = dc_dv_as(dc_unwrap2(env_res), DNodeMacro);

    CLOVE_IS_TRUE(macro.parameters->count == 2);

    CLOVE_IS_TRUE(dc_da_get2(*macro.parameters, 0).type == dc_dvt(DNodeIdentifier));
    CLOVE_IS_TRUE(strcmp(dc_dv_as(dc_da_get2(*macro.parameters, 0), DNodeIdentifier).value, "x") == 0);

    CLOVE_IS_TRUE(dc_da_get2(*macro.parameters, 1).type == dc_dvt(DNodeIdentifier));
    CLOVE_IS_TRUE(strcmp(dc_dv_as(dc_da_get2(*macro.parameters, 1), DNodeIdentifier).value, "y") == 0);

    dang_evaluator_free(&de);

    CLOVE_PASS();
}

CLOVE_TEST(macro_expansion)
{
    dc_foreach2(
        test_loop, string,
        {
            if (_idx % 2 != 0) continue;

            string input = *_it;
            string expected = *(_it + 1);

            DEvaluator de;
            DCResVoid init_res = dang_evaluator_init(&de);
            if (dc_is_err2(init_res))
            {
                dc_log("Evaluator initialization error on input");

                dc_err_log2(init_res, "error");

                CLOVE_FAIL();
            }

            ResDNodeProgram res = dang_define_macros(&de, input);
            if (dc_is_err2(res))
            {
                dc_err_log2(res, "macro definition failed");

                dang_parser_log_errors(&de.parser);

                CLOVE_FAIL();
            }

            DNodeProgram program = dc_unwrap2(res);

            DCResVoid res2 = dang_expand_macros(&de, program.statements);
            if (dc_is_err2(res2))
            {
                dc_err_log2(res2, "macro expansion failed");

                dang_parser_log_errors(&de.parser);

                CLOVE_FAIL();
            }

            string inspect = NULL;
            DCResVoid inspect_res = dang_program_inspect(&program, &inspect);
            if (dc_is_err2(inspect_res))
            {
                dc_err_log2(inspect_res, "result inspection failed");

                dang_parser_log_errors(&de.parser);

                CLOVE_FAIL();
            }

            if (strncmp(inspect, expected, strlen(expected)) != 0)
            {
                dc_log("macro expansion failed on input %s, expected=%s got=%s", input, expected, inspect);

                if (inspect)
                {
                    free(inspect);
                    inspect = NULL;
                }

                CLOVE_FAIL();
            }

            if (inspect) free(inspect);

            dang_evaluator_free(&de);
        },

        "let infix_expr macro () { quote 1 + 2 }; infix_expr;",

        "(1 + 2)\n",

        "let reverse macro (a b) { quote ${unquote b} - ${unquote a} }; reverse 2 + 2 10 - 5",

        "((10 - 5) - (2 + 2))\n",

        "let unless macro (condition consequence alternative) { quote if !${unquote condition} { unquote consequence } else { "
        "unquote alternative }}; unless 10 > 5 ${print 'not greater'} ${print 'greater'}",
        "if (!(10 > 5)) { print(\"not greater\"); } else { print(\"greater\"); }",

        NULL, NULL);

    CLOVE_PASS();
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

        "quote 1 2 3",

        "unquote 1 2 3",

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
