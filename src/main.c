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
#include "parser.h"

#define DANG_REPL_EXIT ":q"

static void repl()
{
    puts("" dc_colorize_fg(LGREEN, "dang") " REPL");
    printf("Hi %s! Type '%s' to exit.\n", dc_get_username(), dc_colorize_bg(RED, DANG_REPL_EXIT));

    char line[1024];

    while (true)
    {
        printf("%s", dc_colorize_fg(LGREEN, "> "));

        if (!fgets(line, sizeof(line), stdin))
        {
            puts("");
            break;
        }

        if (strncmp(line, DANG_REPL_EXIT, strlen(DANG_REPL_EXIT)) == 0) break;

        Scanner s;
        scanner_init(&s, line);

        Parser p;
        DCResultVoid res = parser_init(&p, &s);

        if (dc_res_is_err2(res))
        {
            printf("Parser initialization error: %s\n", dc_res_err_msg2(res));

            parser_free(&p);
            continue;
        }

        ResultDNode program_res = parser_parse_program(&p);

        if (dc_res_is_err2(program_res))
            parser_log_errors(&p);
        else
        {
            dn_string_init(dc_res_val2(program_res));

            printf("Evaluated text:\n" dc_colorize_fg(LGREEN, "%s") "\n", dc_res_val2(program_res)->text);

            dn_program_free(dc_res_val2(program_res));
        }

        parser_free(&p);
    }
}

int main(int argc, string argv[])
{
    (void)argv;

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
