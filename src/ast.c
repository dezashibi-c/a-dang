// ***************************************************************************************
//    Project: Dang Compiler -> https://github.com/dezashibi-c/dang
//    File: ast.c
//    Date: 2024-09-14
//    Author: Navid Dezashibi
//    Contact: navid@dezashibi.com
//    Website: https://dezashibi.com | https://github.com/dezashibi
//    License:
//     Please refer to the LICENSE file, repository or website for more
//     information about the licensing of this work. If you have any questions
//     or concerns, please feel free to contact me at the email address provided
//     above.
// ***************************************************************************************
// *  Description: ast structs and related functionalities
// ***************************************************************************************

#include "ast.h"

string tostr_DangNodeType(DangNodeType dnt)
{
    switch (dnt)
    {
        dc_str_case(DN__UNKNOWN);
        dc_str_case(DN_PROGRAM);
        dc_str_case(DN__START_STATEMENT);
        dc_str_case(DN_LET_STATEMENT);
        dc_str_case(DN_RETURN_STATEMENT);
        dc_str_case(DN_EXPRESSION_STATEMENT);
        dc_str_case(DN_BLOCK_STATEMENT);
        dc_str_case(DN__END_STATEMENT);
        dc_str_case(DN__START_EXPRESSION);
        dc_str_case(DN_IDENTIFIER);
        dc_str_case(DN_PREFIX_EXPRESSION);
        dc_str_case(DN_INFIX_EXPRESSION);
        dc_str_case(DN_IF_EXPRESSION);
        dc_str_case(DN_WHILE_EXPRESSION);
        dc_str_case(DN_CALL_EXPRESSION);
        dc_str_case(DN_INDEX_EXPRESSION);
        dc_str_case(DN__END_EXPRESSION);
        dc_str_case(DN__START_LITERAL);
        dc_str_case(DN_FUNCTION_LITERAL);
        dc_str_case(DN_ARRAY_LITERAL);
        dc_str_case(DN_HASH_LITERAL);
        dc_str_case(DN_MACRO_LITERAL);
        dc_str_case(DN_BOOLEAN_LITERAL);
        dc_str_case(DN_STRING_LITERAL);
        dc_str_case(DN_INTEGER_LITERAL);
        dc_str_case(DN__END_LITERAL);
        dc_str_case(DN__MAX);
    };

    return NULL;
}

DCStringView* dnode_get_token_text(DNode* dn)
{
    dc_action_on(!dnode_type_is_valid(dn->type), exit(1),
                 "got wrong node type: %s", tostr_DangNodeType(dn->type));

    return &dn->token->text;

    // switch (dn->type)
    // {
    //     case DN_PROGRAM:
    //         break;

    //         // DN_LET_STATEMENT
    //         // DN_RETURN_STATEMENT
    //         // DN_EXPRESSION_STATEMENT
    //         // DN_BLOCK_STATEMENT
    //         // DN_IDENTIFIER
    //         // DN_PREFIX_EXPRESSION
    //         // DN_INFIX_EXPRESSION
    //         // DN_IF_EXPRESSION
    //         // DN_WHILE_EXPRESSION
    //         // DN_CALL_EXPRESSION
    //         // DN_INDEX_EXPRESSION
    //         // DN_FUNCTION_LITERAL
    //         // DN_ARRAY_LITERAL
    //         // DN_HASH_LITERAL
    //         // DN_MACRO_LITERAL
    //         // DN_BOOLEAN_LITERAL
    //         // DN_STRING_LITERAL
    //         // DN_INTEGER_LITERAL
    // }

    // return NULL; // unreachable
}
