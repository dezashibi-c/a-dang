// ***************************************************************************************
//    Project: Dang Compiler -> https://github.com/dezashibi-c/dang
//    File: main.c
//    Date: 2024-09-10
//    Author: Navid Dezashibi
//    Contact: navid@dezashibi.com
//    Website: https://dezashibi.com | https://github.com/dezashibi
//    License:
//     Please refer to the LICENSE file, repository or website for more
//     information about the licensing of this work. If you have any questions
//     or concerns, please feel free to contact me at the email address provided
//     above.
// ***************************************************************************************
// *  Description:
// ***************************************************************************************

#define DCOMMON_IMPL
#include "scanner.h"

#define DANG_REPL_EXIT ":q"

static void repl()
{
    puts("" dc_colorize_fg(LGREEN, "dang") " REPL");
    printf("Hi %s! Type '%s' to exit.\n", dc_get_username(),
           dc_colorize_bg(RED, DANG_REPL_EXIT));

    char line[1024];

    // while (true)
    // {
    //     printf("%s", dc_colorize_fg(LGREEN, "> "));

    //     if (!fgets(line, sizeof(line), stdin))
    //     {
    //         puts("");
    //         break;
    //     }

    //     if (strncmp(line, DANG_REPL_EXIT, strlen(DANG_REPL_EXIT)) == 0)
    //     break;

    //     Scanner s;
    //     scanner_init(&s, line);

    //     Token* token;

    //     token = scanner_next_token(&s);

    //     u8 i = 0;
    //     while (token->type != TOK_EOF)
    //     {
    //         printf("[%d] '" DC_SV_FMT "' (%s)\n", i,
    //         dc_sv_fmt_val(token->text),
    //                tostr_DTokenType(token->type));

    //         token = scanner_next_token(&s);
    //         ++i;
    //     }
    // }
}

int main(int argc, string argv[])
{
    if (argc == 1)
    {
        repl();
    }
    else if (argc == 2)
    { // file_run(argv[1]);
    }
    else
    {
        fprintf(stderr, "Usage: dang [path]\n");
    }

    return 0;
}
