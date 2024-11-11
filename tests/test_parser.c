#define CLOVE_SUITE_NAME dang_parser_tests

#include "clove-unit/clove-unit.h"

#include "common.h"
#include "parser.h"
#include "scanner.h"

static DCDynArr pool;
static DCDynArr errors;

static DParser parser;

CLOVE_SUITE_SETUP()
{
    pool = (DCDynArr){0};
    errors = (DCDynArr){0};

    parser = (DParser){0};

    DCResVoid res = dang_parser_init(&parser, &pool, &errors);
    if (dc_is_err2(res))
    {
        dc_log("parser initialization error on input");

        dc_err_log2(res, "error");

        exit(dc_err_code2(res));
    }
}

CLOVE_SUITE_TEARDOWN()
{
    dc_da_free(&pool);
    dc_da_free(&errors);
}

static b1 dang_parser_has_no_error(DParser* p)
{
    dc_action_on(p->errors->count != 0, dang_parser_log_errors(p);
                 return false, "parser has " dc_fmt(usize) " errors", p->errors->count);

    return true;
}

static b1 perform_test_batch(string tests[], usize tests_count)
{
    for (usize i = 0; i < tests_count / 2; ++i)
    {
        string input = tests[i * 2];
        string expected = tests[(i * 2) + 1];

        ResDNodeProgram program_res = dang_parse(&parser, input);

        if (!dang_parser_has_no_error(&parser))
        {
            dc_log("parser error on input '%s'", input);

            return false;
        }

        DNodeProgram program = dc_unwrap2(program_res);

        string result = NULL;
        DCResVoid inspection_res = dang_program_inspect(&program, &result);
        if (dc_is_err2(inspection_res))
        {
            dc_log("inspection failed on input: '%s'", input);
            dc_err_log2(inspection_res, "Inspection error");

            return false;
        }
        else
            dc_action_on(strcmp(expected, result) != 0, return false, "expected=%s, got=%s", expected, result);
    }

    return true;
}

CLOVE_TEST(literals)
{
    string tests[] = {
        "5",
        "5\n",

        "'hello world'",
        "\"hello world\"\n",

        "\"hello world\"",
        "\"hello world\"\n",

        "foobar",
        "foobar\n",

        "$\"a long variable name\"",
        "a long variable name\n",

        "$1",
        "1\n",

        "true",
        "true\n",

        "false",
        "false\n",

        "fn () {}",
        "Fn () { }\n",

        "fn (x) {}",
        "Fn (x) { }\n",

        "fn (x y z) {}",
        "Fn (x, y, z) { }\n",

        "macro (x y z) {}",
        "MACRO (x, y, z) { }\n",

        "if x < y { x }",
        "if (x < y) { x; }\n",

        "if x < y { x } else { y }",
        "if (x < y) { x; } else { y; }\n",

        "if a > 10 { if a > 10 { a } }",
        "if (a > 10) { if (a > 10) { a; }; }\n",
    };

    if (!perform_test_batch(tests, dc_count(tests)))
    {
        dc_log("test failed");
        CLOVE_FAIL();

        return;
    }

    CLOVE_PASS();
}

CLOVE_TEST(statements)
{
    string tests[] = {
        "foobar;",
        "foobar()\n",

        "let $1 5; let $\"some long variable name\" true\nlet foobar y",
        "let 1 5\nlet some long variable name true\nlet foobar y\n",

        "return; return 5; return true\nreturn y",
        "return\nreturn 5\nreturn true\nreturn y\n",

        "fn(x, y z) {x+y-z}",
        "Fn (x, y, z) { ((x + y) - z); }\n",

        "add 1 2 3",
        "add(1, 2, 3)\n",

        "${add 1 2 3}",
        "add(1, 2, 3)\n",

        "${add 1 2 3};",
        "add(1, 2, 3)()\n",

        "${add 1 a * b, -4 $1}",
        "add(1, (a * b), (-4), 1)\n",

        "${add 1 a * b, -4 $1};",
        "add(1, (a * b), (-4), 1)()\n",

        "add ${add 1 2 3} 3 x",
        "add(add(1, 2, 3), 3, x)\n",

        "a + ${add b * c} + d",
        "((a + add((b * c))) + d)\n",

        "add a * b[2] b[1] 2 * [1 2][1], [1]", // to stop the index we need to use comma
        "add((a * (b[2])), (b[1]), (2 * ([1, 2][1])), [1])\n",

        "add a b 1 2*3, 4+5 ${add 6 7 * 8}",
        "add(a, b, 1, (2 * 3), (4 + 5), add(6, (7 * 8)))\n",

        "${add a b 1 2*3, 4+5 ${add 6 7 * 8}}",
        "add(a, b, 1, (2 * 3), (4 + 5), add(6, (7 * 8)))\n",

        "${add a b 1 2*3, 4+5 ${add 6 7 * 8}};",
        "add(a, b, 1, (2 * 3), (4 + 5), add(6, (7 * 8)))()\n",

        "add a + b + c * d / f + g",
        "add((((a + b) + ((c * d) / f)) + g))\n",

        "${add a + b + c * d / f + g}",
        "add((((a + b) + ((c * d) / f)) + g))\n",

        "${add a + b + c * d / f + g};",
        "add((((a + b) + ((c * d) / f)) + g))()\n",

        "arr2[0]",
        "(arr2[0])\n",

        "let arr3 [1 2 3];",
        "let arr3 [1, 2, 3]\n",

        "let arr2 [1 2 3]; arr2[0] + arr2[1] + arr2[2]",
        "let arr2 [1, 2, 3]\n(((arr2[0]) + (arr2[1])) + (arr2[2]))\n",

        "let arr3 [1 2 3]; let i arr3[0]; arr3[i]",
        "let arr3 [1, 2, 3]\nlet i (arr3[0])\n(arr3[i])\n",

        "quote 5",
        "QUOTE(5)\n",
    };

    if (!perform_test_batch(tests, dc_count(tests)))
    {
        dc_log("test failed");
        CLOVE_FAIL();

        return;
    }

    CLOVE_PASS();
}

CLOVE_TEST(expressions)
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

        "3 + 4\n -5 * 5",
        "(3 + 4)\n((-5) * 5)\n",

        "5 > 4 == 3 < 4",
        "((5 > 4) == (3 < 4))\n",

        "5 < 4 != 3 > 4",
        "((5 < 4) != (3 > 4))\n",

        "3 + 4 * 5 == 3 * 1 + 4 * 5",
        "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))\n",

        "fn(x, y z) {x+y-z}",
        "Fn (x, y, z) { ((x + y) - z); }\n",

        "if x < y { x }",
        "if (x < y) { x; }\n",

        "if x < y { x } else { y }",
        "if (x < y) { x; } else { y; }\n",

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

        "[1]",
        "[1]\n",

        "[1 2 'hello' 1 - 1, -1]",
        "[1, 2, \"hello\", (1 - 1), (-1)]\n",

        "my_array[1 + 1]",
        "(my_array[(1 + 1)])\n",

        "a * [1 2 3 4 5][a * c] * d",
        "((a * ([1, 2, 3, 4, 5][(a * c)])) * d)\n",

        "if a > 10 { if a > 10 { a } }",
        "if (a > 10) { if (a > 10) { a; }; }\n",

        "{}",
        "{}\n",

        "{'one': 1, 'two': 2, 'three': 3}",
        "{\"one\": 1, \"two\": 2, \"three\": 3}\n",

        "{'one': 0 + 1, 'two': 10 - 8, 'three': 15 / 5}",
        "{\"one\": (0 + 1), \"two\": (10 - 8), \"three\": (15 / 5)}\n",

        "{\n"
        " 'one': 10 - 9\n"
        " 'thr' + 'ee': 6 / 2\n"
        " 4: 4,\n"
        " true: 5,\n"
        " false: 6\n"
        "}",
        "{\"one\": (10 - 9), (\"thr\" + \"ee\"): (6 / 2), 4: 4, true: 5, false: 6}\n",

        "let a {\n"
        " 'one': 10 - 9\n"
        " 'thr' + 'ee': 6 / 2\n"
        " 4: 4,\n"
        " true: 5,\n"
        " false: 6\n"
        "}",
        "let a {\"one\": (10 - 9), (\"thr\" + \"ee\"): (6 / 2), 4: 4, true: 5, false: 6}\n",
    };

    if (!perform_test_batch(tests, dc_count(tests)))
    {
        dc_log("test failed");
        CLOVE_FAIL();

        return;
    }

    CLOVE_PASS();
}
