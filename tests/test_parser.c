#define CLOVE_SUITE_NAME dang_parser_tests

#include "clove-unit/clove-unit.h"

#include "parser.h"
#include "scanner.h"
#include "types.h"

static bool dang_parser_has_no_error(DParser* p)
{
    dc_action_on(p->errors.count != 0, dang_parser_log_errors(p);
                 return false, "parser has " dc_fmt(usize) " errors", p->errors.count);

    return true;
}

static bool perform_test_batch(string tests[], usize tests_count)
{
    bool success = true;

    for (usize i = 0; i < tests_count / 2; ++i)
    {
        string input = tests[i * 2];
        string expected = tests[(i * 2) + 1];

        DScanner s;
        dang_scanner_init(&s, input);

        DParser p;
        DCResVoid res = dang_parser_init(&p, &s);

        if (dc_res_is_err2(res))
        {
            dc_log("parser initialization error on input '%s'", input);

            dc_res_err_log2(res, "error");

            success = false;

            continue;
        }

        ResNode program_res = dang_parser_parse_program(&p);

        DNode* program = dc_res_val2(program_res);

        if (!dang_parser_has_no_error(&p))
        {
            dc_log("parser error on input '%s'", input);

            dn_program_free(program);
            dang_parser_free(&p);

            success = false;

            continue;
        }

        string result = NULL;
        DCResVoid inspection_res = dang_node_inspect(program, &result);
        if (dc_res_is_err2(inspection_res))
        {
            dc_log("inspection failed on input: '%s'", input);
            dc_res_err_log2(inspection_res, "Inspection error");
            success = false;
        }
        else
            dc_action_on(strcmp(expected, result) != 0, return false, "expected=%s, got=%s", expected, result);

        dn_program_free(program);
        dang_parser_free(&p);
    }

    return success;
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
    };

    if (!perform_test_batch(tests, dc_count(tests)))
    {
        dc_log("test failed");
        CLOVE_FAIL();

        return;
    }

    CLOVE_PASS();
}
