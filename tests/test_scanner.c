#define CLOVE_SUITE_NAME ScannerTests
#define DCOMMON_IMPL

#include "clove-unit/clove-unit.h"

#include "dcommon/dcommon.h"
#include "scanner.h"

typedef struct
{
    TokenType type;
    const string text;
} TestExpectedResult;

static bool perform_scanner_test(const string input, usize number_of_tests,
                                 TestExpectedResult tests[])
{
    Scanner s;
    scanner_init(&s, input);

    u8 i = 0;
    dc_sforeach(tests, TestExpectedResult, expected_result,
                expected_result->type != TOK_EOF)
    {
        Token* token = scanner_next_token(&s);

        dc_halt_when(
            expected_result->type != token->type, return false,
            "Bad result on token [%d], expected type='%s' but got='%s'", i,
            tostr_TokenType(expected_result->type),
            tostr_TokenType(token->type));

        dc_halt_when(
            strcmp(expected_result->text, token->text) != 0, return false,
            "Bad result on token [%d], expected text='%s' but got='%s'", i,
            expected_result->text, token->text);

        ++i;
    }

    dc_halt_when(number_of_tests != s.tokens.count, return false,
                 "Expected %zu tokens but got=%zu", number_of_tests,
                 s.tokens.count);

    scanner_free(&s);

    return true;
}

CLOVE_TEST(BasicSigns)
{
    const string input = "=+(){},;\n";

    TestExpectedResult tests[] = {
        {.type = TOK_ASSIGN, .text = "="},
        {.type = TOK_PLUS, .text = "+"},
        {.type = TOK_LPAREN, .text = "("},
        {.type = TOK_RPAREN, .text = ")"},
        {.type = TOK_LBRACE, .text = "{"},
        {.type = TOK_RBRACE, .text = "}"},
        {.type = TOK_COMMA, .text = ","},
        {.type = TOK_SEMICOLON, .text = ";"},
        {.type = TOK_NEWLINE, .text = "\n"},
        {.type = TOK_EOF, .text = ""},
    };

    if (!perform_scanner_test(input, dc_len(tests), tests)) CLOVE_FAIL();
}