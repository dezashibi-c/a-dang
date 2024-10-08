#define CLOVE_SUITE_NAME dang_scanner_tests

#include "clove-unit/clove-unit.h"

#include "dcommon/dcommon.h"
#include "scanner.h"

typedef struct
{
    DTokenType type;
    const string text;
} TestExpectedResult;

static bool perform_token_test(TestExpectedResult* expected_token, DToken* actual_token, u8 token_index)
{
    dc_action_on(actual_token->text.str == NULL || expected_token->text == NULL, return false,
                 "Null string in Token comparison at index %d", token_index);

    dc_action_on(expected_token->type != actual_token->type, return false,
                 "Bad result on token [%d], expected type='%s' but got='%s'", token_index,
                 tostr_DTokenType(expected_token->type), tostr_DTokenType(actual_token->type));

    dc_action_on(!dc_sv_str_eq(actual_token->text, expected_token->text), return false,
                 "Bad result on token [%d], expected text='%s', len='%zu' but "
                 "got='" DCPRIsv "', len='%zu'",
                 token_index, expected_token->text, strlen(expected_token->text), dc_sv_fmt(actual_token->text),
                 actual_token->text.len);

    return true;
}

static bool perform_dang_scanner_test(const string input, TestExpectedResult tests[])
{
    Scanner s;
    dang_scanner_init(&s, input);

    ResultToken token;

    u8 i = 0;
    dc_sforeach(tests, TestExpectedResult, _it->type != TOK_EOF)
    {
        token = dang_scanner_next_token(&s);
        if (dc_res_is_err2(token)) return false;

        if (!perform_token_test(_it, dc_res_val2(token), i)) return false;

        ++i;
    }

    token = dang_scanner_next_token(&s);
    if (!perform_token_test(&tests[i], dc_res_val2(token), i)) return false;
    ++i;

    dc_action_on(i != s.tokens.count, return false, "Expected %d tokens but got=%zu", i, s.tokens.count);

    dang_scanner_free(&s);

    return true;
}

CLOVE_TEST(basic_signs)
{
    const string input = "=+(){},;\n";

    TestExpectedResult tests[] = {
        {.type = TOK_ASSIGN, .text = "="}, {.type = TOK_PLUS, .text = "+"},      {.type = TOK_LPAREN, .text = "("},
        {.type = TOK_RPAREN, .text = ")"}, {.type = TOK_LBRACE, .text = "{"},    {.type = TOK_RBRACE, .text = "}"},
        {.type = TOK_COMMA, .text = ","},  {.type = TOK_SEMICOLON, .text = ";"}, {.type = TOK_NEWLINE, .text = "\n"},
        {.type = TOK_EOF, .text = ""},
    };

    CLOVE_IS_TRUE(perform_dang_scanner_test(input, tests));
}

CLOVE_TEST(more_tokens)
{
    const string input = "let five = 5; let ten = 10\n"
                         "let add = fn(x, y)\n"
                         "{\n"
                         "  x + y\n"
                         "}\n"
                         "let result = (add five, ten)";

    TestExpectedResult tests[] = {
        {.type = TOK_LET, .text = "let"},     {.type = TOK_IDENT, .text = "five"},   {.type = TOK_ASSIGN, .text = "="},
        {.type = TOK_INT, .text = "5"},       {.type = TOK_SEMICOLON, .text = ";"},  {.type = TOK_LET, .text = "let"},
        {.type = TOK_IDENT, .text = "ten"},   {.type = TOK_ASSIGN, .text = "="},     {.type = TOK_INT, .text = "10"},
        {.type = TOK_NEWLINE, .text = "\n"},

        {.type = TOK_LET, .text = "let"},     {.type = TOK_IDENT, .text = "add"},    {.type = TOK_ASSIGN, .text = "="},
        {.type = TOK_FUNCTION, .text = "fn"}, {.type = TOK_LPAREN, .text = "("},     {.type = TOK_IDENT, .text = "x"},
        {.type = TOK_COMMA, .text = ","},     {.type = TOK_IDENT, .text = "y"},      {.type = TOK_RPAREN, .text = ")"},
        {.type = TOK_NEWLINE, .text = "\n"},

        {.type = TOK_LBRACE, .text = "{"},    {.type = TOK_NEWLINE, .text = "\n"},

        {.type = TOK_IDENT, .text = "x"},     {.type = TOK_PLUS, .text = "+"},       {.type = TOK_IDENT, .text = "y"},
        {.type = TOK_NEWLINE, .text = "\n"},

        {.type = TOK_RBRACE, .text = "}"},    {.type = TOK_NEWLINE, .text = "\n"},

        {.type = TOK_LET, .text = "let"},     {.type = TOK_IDENT, .text = "result"}, {.type = TOK_ASSIGN, .text = "="},
        {.type = TOK_LPAREN, .text = "("},    {.type = TOK_IDENT, .text = "add"},    {.type = TOK_IDENT, .text = "five"},
        {.type = TOK_COMMA, .text = ","},     {.type = TOK_IDENT, .text = "ten"},    {.type = TOK_RPAREN, .text = ")"},

        {.type = TOK_EOF, .text = ""},
    };

    CLOVE_IS_TRUE(perform_dang_scanner_test(input, tests));
}

CLOVE_TEST(remaining_tokens)
{
    const string input = "!-/*5\n"
                         "5 < 10 > 5\n"
                         "5 == 10 != 5";

    TestExpectedResult tests[] = {
        {.type = TOK_BANG, .text = "!"},     {.type = TOK_MINUS, .text = "-"}, {.type = TOK_SLASH, .text = "/"},
        {.type = TOK_ASTERISK, .text = "*"}, {.type = TOK_INT, .text = "5"},   {.type = TOK_NEWLINE, .text = "\n"},

        {.type = TOK_INT, .text = "5"},      {.type = TOK_LT, .text = "<"},    {.type = TOK_INT, .text = "10"},
        {.type = TOK_GT, .text = ">"},       {.type = TOK_INT, .text = "5"},   {.type = TOK_NEWLINE, .text = "\n"},

        {.type = TOK_INT, .text = "5"},      {.type = TOK_EQ, .text = "=="},   {.type = TOK_INT, .text = "10"},
        {.type = TOK_NEQ, .text = "!="},     {.type = TOK_INT, .text = "5"},

        {.type = TOK_EOF, .text = ""},
    };

    CLOVE_IS_TRUE(perform_dang_scanner_test(input, tests));
}

CLOVE_TEST(rest_of_keywords)
{
    const string input = "if (5 < 10) {\n"
                         "  return true\n"
                         "} else {\n"
                         "  return false\n"
                         "}\n"
                         "$\"hey there\" test $12";

    TestExpectedResult tests[] = {
        {.type = TOK_IF, .text = "if"},
        {.type = TOK_LPAREN, .text = "("},
        {.type = TOK_INT, .text = "5"},
        {.type = TOK_LT, .text = "<"},
        {.type = TOK_INT, .text = "10"},
        {.type = TOK_RPAREN, .text = ")"},
        {.type = TOK_LBRACE, .text = "{"},
        {.type = TOK_NEWLINE, .text = "\n"},

        {.type = TOK_RET, .text = "return"},
        {.type = TOK_TRUE, .text = "true"},
        {.type = TOK_NEWLINE, .text = "\n"},

        {.type = TOK_RBRACE, .text = "}"},
        {.type = TOK_ELSE, .text = "else"},
        {.type = TOK_LBRACE, .text = "{"},
        {.type = TOK_NEWLINE, .text = "\n"},

        {.type = TOK_RET, .text = "return"},
        {.type = TOK_FALSE, .text = "false"},
        {.type = TOK_NEWLINE, .text = "\n"},

        {.type = TOK_RBRACE, .text = "}"},
        {.type = TOK_NEWLINE, .text = "\n"},

        {.type = TOK_IDENT, .text = "hey there"},
        {.type = TOK_IDENT, .text = "test"},
        {.type = TOK_IDENT, .text = "12"},


        {.type = TOK_EOF, .text = ""},
    };

    CLOVE_IS_TRUE(perform_dang_scanner_test(input, tests));
}
