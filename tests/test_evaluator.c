#define CLOVE_SUITE_NAME dang_evaluator_tests

#include "clove-unit/clove-unit.h"

#include "dcommon/dcommon.h"
#include "evaluator.h"
#include "parser.h"
#include "scanner.h"

static bool dang_parser_has_no_error(Parser* p)
{
    dc_action_on(p->errors.count != 0, dang_parser_log_errors(p); return false, "parser has %zu errors", p->errors.count);

    return true;
}

static DCResult test_eval(string input)
{
    DC_RES();

    Scanner s;
    dang_scanner_init(&s, input);

    Parser p;
    dang_parser_init(&p, &s);

    ResultDNode program_res = dang_parser_parse_program(&p);

    DNode* program = dc_res_val2(program_res);

    if (!dang_parser_has_no_error(&p))
    {
        dn_program_free(program);
        dang_parser_free(&p);

        dc_res_ret_e(-1, "parser has error");
    }

    dc_try_or_fail_with(dang_eval(program), {
        dn_program_free(program);
        dang_parser_free(&p);
    });

    dn_program_free(program);
    dang_parser_free(&p);

    dc_res_ret();
}

static bool test_evaluated_literal(DCDynVal* obj, DCDynVal* expected)
{
    if (dc_dv_is_not(*expected, voidptr))
    {
        DCResultBool res = dc_dv_eq(obj, expected);
        if (dc_res_is_err2(res))
        {
            dc_res_err_log2(res, "cannot compare dynamic values");
            return false;
        }

        if (!dc_res_val2(res))
        {
            DCResultString obj_str, expected_str;

            obj_str = dc_tostr_dv(obj);
            expected_str = dc_tostr_dv(expected);

            dc_log("expected '%s' but got '%s'", dc_res_val2(expected_str), dc_res_val2(obj_str));

            if (dc_res_val2(obj_str)) free(dc_res_val2(obj_str));
            if (dc_res_val2(expected_str)) free(dc_res_val2(expected_str));
        }

        return dc_res_val2(res);
    }

    return false;
}

typedef struct
{
    string input;
    DCDynVal expected;
} TestCase;

CLOVE_TEST(literal_expressions)
{
    TestCase tests[] = {
        {.input = "5", .expected = dang_int(5)},

        {.input = "10", .expected = dang_int(10)},

        {.input = "true", .expected = DANG_TRUE},

        {.input = "false", .expected = DANG_FALSE},

        {.input = "", .expected = DANG_NULL},
    };

    dc_sforeach(tests, TestCase, strlen(_it->input) != 0)
    {
        DCResult res = test_eval(_it->input);
        if (dc_res_is_err2(res))
        {
            dc_res_err_log2(res, "evaluation failed");
            CLOVE_FAIL();
        }

        dc_action_on(!test_evaluated_literal(&dc_res_val2(res), &_it->expected), CLOVE_FAIL(), "evaluation result failed");
    }

    CLOVE_PASS();
}

CLOVE_TEST(integer_expressions)
{
    TestCase tests[] = {
        {.input = "5", .expected = dang_int(5)},

        {.input = "10", .expected = dang_int(10)},

        {.input = "-5", .expected = dang_int(-5)},

        {.input = "-10", .expected = dang_int(-10)},

        {.input = "--10", .expected = dang_int(10)},

        {.input = "5 + 5 + 5 + 5 - 10", .expected = dang_int(10)},

        {.input = "2 * 2 * 2 * 2 * 2", .expected = dang_int(32)},

        {.input = "-50 + 100 + -50", .expected = dang_int(0)},

        {.input = "5 * 2 + 10", .expected = dang_int(20)},

        {.input = "5 + 2 * 10", .expected = dang_int(25)},

        {.input = "20 + 2 * -10", .expected = dang_int(0)},

        {.input = "50 / 2 * 2 + 10", dang_int(60)},

        {.input = "2 * (5 + 10)", .expected = dang_int(30)},

        {.input = "3 * 3 * 3 + 10", .expected = dang_int(37)},

        {.input = "3 * (3 * 3) + 10", .expected = dang_int(37)},

        {.input = "(5 + 10 * 2 + 15 / 3) * 2 + -10", .expected = dang_int(50)},

        {.input = "", .expected = DANG_NULL},
    };

    dc_sforeach(tests, TestCase, strlen(_it->input) != 0)
    {
        DCResult res = test_eval(_it->input);
        if (dc_res_is_err2(res))
        {
            dc_res_err_log2(res, "evaluation failed");
            CLOVE_FAIL();
        }

        dc_action_on(!test_evaluated_literal(&dc_res_val2(res), &_it->expected), CLOVE_FAIL(), "evaluation result failed");
    }

    CLOVE_PASS();
}

CLOVE_TEST(bang_operator)
{
    TestCase tests[] = {
        {.input = "!false", .expected = DANG_TRUE},

        {.input = "!true", .expected = DANG_FALSE},

        {.input = "!5", .expected = DANG_FALSE},

        {.input = "!!true", .expected = DANG_TRUE},

        {.input = "!!false", .expected = DANG_FALSE},

        {.input = "!!5", .expected = DANG_TRUE},

        {.input = "", .expected = DANG_NULL},
    };

    dc_sforeach(tests, TestCase, strlen(_it->input) != 0)
    {
        DCResult res = test_eval(_it->input);
        if (dc_res_is_err2(res))
        {
            dc_res_err_log2(res, "evaluation failed");
            CLOVE_FAIL();
        }

        dc_action_on(!test_evaluated_literal(&dc_res_val2(res), &_it->expected), CLOVE_FAIL(), "evaluation result failed");
    }

    CLOVE_PASS();
}
