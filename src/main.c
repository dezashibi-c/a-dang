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

static void print_obj(DObj* obj)
{
    if (dobj_is_bool(*obj))
        printf("%s", dc_tostr_bool(dobj_as_bool(*obj)));

    else if (dobj_is_array(*obj))
    {
        printf("%s", "[ ");

        dc_da_for(obj->children, {
            print_obj(dobj_child(obj, _idx));

            if (_idx < obj->children.count - 1) printf("%s", ", ");
        });

        printf("%s", " ]");
    }

    else if (dobj_is_null(*obj))
        printf("%s", dc_colorize_fg(LRED, "(null)"));

    else if (dobj_is_string(*obj))
    {
        printf("%s", "\"");
        if (strlen(dc_dv_as(obj->dv, string)) > 0) dc_dv_print(&obj->dv);
        printf("%s", "\"");
    }

    else
        dc_dv_print(&obj->dv);
}

static void repl()
{
    puts("" dc_colorize_fg(LGREEN, "dang") " REPL");
    printf("Hi %s! Type '%s' to exit.\n", dc_get_username(), dc_colorize_bg(RED, DANG_REPL_EXIT));

    char line[1024];

    ResEnv de_res = dang_env_new();
    if (dc_is_err2(de_res))
    {
        dc_err_log2(de_res, "cannot initialize environment");

        return;
    }

    DEnv* de = dc_unwrap2(de_res);

    while (true)
    {
        printf("%s", dc_colorize_fg(LGREEN, "> "));

        if (!fgets(line, sizeof(line), stdin))
        {
            puts("");
            break;
        }

        if (strncmp(line, DANG_REPL_EXIT, strlen(DANG_REPL_EXIT)) == 0) break;

        DScanner s;
        dang_scanner_init(&s, line);

        DParser p;
        DCResVoid res = dang_parser_init(&p, &s);

        if (dc_is_err2(res))
        {
            dc_err_log2(res, DC_FG_LRED "DParser initialization error");
            printf("%s", DC_COLOR_RESET);

            continue;
        }

        ResNode program_res = dang_parser_parse_program(&p);
        if (dc_is_err2(program_res))
            dc_log("parser could not finish the job properly: (code %d) %s", dc_err_code2(program_res),
                   dc_err_msg2(program_res));
        else
        {
            if (dang_parser_has_error(&p)) dang_parser_log_errors(&p);

            DNode* program = dc_unwrap2(program_res);

            string result = NULL;
            DCResVoid inspection_res = dang_node_inspect(program, &result);
            if (dc_is_err2(inspection_res))
            {
                dc_err_log2(inspection_res, "Inspection error");
            }
            else
            {
                printf("Evaluated text:\n" dc_colorize_fg(LGREEN, "%s") "\n", result);

                ResObj evaluated = dang_eval(dc_unwrap2(program_res), de);
                if (dc_is_err2(evaluated))
                {
                    dc_err_log2(evaluated, DC_FG_LRED "Evaluation error");
                    printf("%s", DC_COLOR_RESET);
                }
                else
                {
                    printf("%s", "Result: " DC_FG_LGREEN);

                    print_obj(dc_unwrap2(evaluated));

                    printf("\n%s", DC_COLOR_RESET);
                }
            }
        }

        dang_parser_free(&p);
    }

    dang_env_free(de);
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
