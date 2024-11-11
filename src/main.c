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

#include "evaluator.h"

#define DANG_REPL_EXIT ":q"

static void repl()
{
    puts("" dc_colorize_fg(LGREEN, "dang") " REPL");
    printf("Hi %s! Type '%s' to exit.\n", dc_get_username(), dc_colorize_bg(RED, DANG_REPL_EXIT));

    char line[1024];

    DEvaluator de = {0};
    DCResVoid de_res = dang_evaluator_init(&de);
    if (dc_is_err2(de_res))
    {
        dc_err_log2(de_res, "cannot initialize evaluator");

        dc_result_free(&de_res);

        return;
    }

    while (true)
    {
        printf("%s", dc_colorize_fg(LGREEN, "> "));

        if (!fgets(line, sizeof(line), stdin))
        {
            puts("");
            break;
        }

        if (strncmp(line, DANG_REPL_EXIT, strlen(DANG_REPL_EXIT)) == 0) break;

        ResEvaluated evaluation_res = dang_eval(&de, line, true);
        if (dc_is_err2(evaluation_res))
        {
            dc_log("evaluator could not finish the job properly: (code %d) %s", dc_err_code2(evaluation_res),
                   dc_err_msg2(evaluation_res));

            dang_parser_log_errors(&de.parser);

            dc_result_free(&evaluation_res);
        }
        else
        {
            Evaluated evaluated = dc_unwrap2(evaluation_res);
            printf("Evaluated text:\n" dc_colorize_fg(LGREEN, "%s") "\n", evaluated.inspect);

            printf("%s", "Result: " DC_FG_LGREEN);

            do_print(&evaluated.result);

            printf("\n%s", DC_COLOR_RESET);
        }
    }

    dang_evaluator_free(&de);
}

int main(int argc, string argv[])
{
    (void)argv;

    if (argc == 1)
    {
        repl();
    }
    else if (argc == 2)
    {
        // file_run(argv[1]);
    }
    else
    {
        fprintf(stderr, "Usage: dang [path]\n");
    }

    return 0;
}
