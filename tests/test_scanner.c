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

CLOVE_TEST(ScannerKnowsBasicSigns)
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

    Scanner s;
    scanner_init(&s, input);

    u8 i = 0;
    dc_sforeach(tests, TestExpectedResult, expected_result,
                expected_result->type != TOK_EOF)
    {
        Token* token = scanner_next_token(&s);

        dc_halt_when(
            expected_result->type != token->type, CLOVE_FAIL(),
            "Bad result on token [%d], expected type='%s' but got='%s'", i,
            tostr_TokenType(expected_result->type),
            tostr_TokenType(token->type));

        dc_halt_when(
            strcmp(expected_result->text, token->text) != 0, CLOVE_FAIL(),
            "Bad result on token [%d], expected text='%s' but got='%s'", i,
            expected_result->text, token->text);

        ++i;
    }

    dc_halt_when(dc_len(tests) != s.tokens.count, CLOVE_FAIL(),
                 "Expected %zu tokens but got=%zu", dc_len(tests),
                 s.tokens.count);

    CLOVE_PASS();
}