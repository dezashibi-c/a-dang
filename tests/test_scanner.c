#define CLOVE_SUITE_NAME scanner_tests
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
    dc_sforeach(tests, TestExpectedResult, _it->type != TOK_EOF)
    {
        Token* token = scanner_next_token(&s);

        dc_action_on(
            _it->type != token->type, return false,
            "Bad result on token [%d], expected type='%s' but got='%s'", i,
            tostr_TokenType(_it->type), tostr_TokenType(token->type));

        dc_action_on(
            strcmp(_it->text, token->text) != 0, return false,
            "Bad result on token [%d], expected text='%s' but got='%s'", i,
            _it->text, token->text);

        ++i;
    }

    dc_action_on(number_of_tests != s.tokens.count, return false,
                 "Expected %zu tokens but got=%zu", number_of_tests,
                 s.tokens.count);

    scanner_free(&s);

    return true;
}

CLOVE_TEST(basic_signs)
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

    CLOVE_IS_TRUE(perform_scanner_test(input, dc_len(tests), tests));
}

CLOVE_TEST(more_tokens)
{
    const string input = "let five = 5;\n"
                         "let ten = 10;\n"
                         "let add = fn(x, y)\n"
                         "{\n"
                         "x + y;\n"
                         "};\n"
                         "let result = add(five, ten);";

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

    CLOVE_IS_TRUE(perform_scanner_test(input, dc_len(tests), tests));
}