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
#include "evaluator.h"
#include "parser.h"

#define DANG_REPL_EXIT ":q"

static DC_DV_FREE_FN_DECL(_program_free)
{
    DC_RES_void();

    if (_value && _value->type == dc_dvt(voidptr))
    {
        DNode* program = (DNode*)dc_dv_as(*_value, voidptr);

        dn_free(program);
    }

    dc_res_ret();
}

static void repl()
{
    puts("" dc_colorize_fg(LGREEN, "dang") " REPL");
    printf("Hi %s! Type '%s' to exit.\n", dc_get_username(), dc_colorize_bg(RED, DANG_REPL_EXIT));

    char line[1024];

    DEnvResult de_res = dang_denv_new();
    if (dc_res_is_err2(de_res))
    {
        dc_res_err_log2(de_res, "cannot initialize environment");

        return;
    }

    DEnv* de = dc_res_val2(de_res);
    DCResultDa programs_res = dc_da_new(_program_free);
    if (dc_res_is_err2(programs_res))
    {
        dc_res_err_log2(programs_res, "cannot initialize programs array");

        dang_denv_free(de);

        return;
    }

    DCDynArr* programs = dc_res_val2(programs_res);

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
        dang_scanner_init(&s, line);

        Parser p;
        DCResultVoid res = dang_parser_init(&p, &s);

        if (dc_res_is_err2(res))
        {
            dc_res_err_log2(res, DC_FG_LRED "Parser initialization error");
            printf("%s", DC_COLOR_RESET);

            continue;
        }

        ResultDNode program_res = dang_parser_parse_program(&p);

        dc_da_push(programs, dc_dva(voidptr, dc_res_val2(program_res)));

        if (dc_res_is_err2(program_res))
            dc_log("parser could not finish the job properly: (code %d) %s", dc_res_err_code2(program_res),
                   dc_res_err_msg2(program_res));
        else
        {
            if (dang_parser_has_error(&p)) dang_parser_log_errors(&p);

            dn_string_init(dc_res_val2(program_res));

            if (dc_res_val2(program_res)->text)
            {
                printf("Evaluated text:\n" dc_colorize_fg(LGREEN, "%s") "\n", dc_res_val2(program_res)->text);

                DObjResult evaluated = dang_eval(dc_res_val2(program_res), de);
                if (dc_res_is_err2(evaluated))
                {
                    dc_res_err_log2(evaluated, DC_FG_LRED "Evaluation error");
                    printf("%s", DC_COLOR_RESET);
                }
                else
                {
                    printf("%s", "Result: " DC_FG_LGREEN);

                    if (dobj_is_bool(dc_res_val2(evaluated)))
                        printf("%s\n", dc_tostr_bool(dobj_as_bool(dc_res_val2(evaluated))));
                    else if (dobj_is_null(dc_res_val2(evaluated)))
                        printf("%s", dc_colorize_fg(LRED, "(null)\n"));
                    else
                        dc_dv_println(&dc_res_val2(evaluated).dv);

                    printf("%s", DC_COLOR_RESET);
                }
            }
        }

        dang_parser_free(&p);
    }

    dc_da_free(programs);

    dang_denv_free(de);
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
