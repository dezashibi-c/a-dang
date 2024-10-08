#define CLOVE_SUITE_NAME evaluator_tests

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

    dc_try_or_fail_with(eval(program), {
        dn_program_free(program);
        dang_parser_free(&p);
    });

    dn_program_free(program);
    dang_parser_free(&p);

    dc_res_ret();
}

static bool test_evaluated_integer(DCDynVal* obj, DCDynVal* expected)
{
    if (dc_dv_is_not(*expected, voidptr))
    {
        DCResultBool res = dc_dv_eq(obj, expected);
        if (dc_res_is_err2(res))
        {
            dc_res_err_log2(res, "cannot compare dynamic values");
            return false;
        }

        return dc_res_val2(res);
    }

    return true;
}

typedef struct
{
    string input;
    DCDynVal expected;
} TestCase;

CLOVE_TEST(integer_expression)
{
    TestCase tests[] = {
        {.input = "5", .expected = dc_dv(i64, 5)},
        {.input = "10", .expected = dc_dv(i64, 10)},
        {.input = "", .expected = dc_dv(voidptr, NULL)},
    };

    dc_sforeach(tests, TestCase, strlen(_it->input) != 0)
    {
        DCResult res = test_eval(_it->input);
        if (dc_res_is_err2(res))
        {
            dc_res_err_log2(res, "evaluation failed");
            CLOVE_FAIL();
        }

        dc_action_on(!test_evaluated_integer(&dc_res_val2(res), &_it->expected), CLOVE_FAIL(), "evaluation result failed");
    }

    CLOVE_PASS();
}