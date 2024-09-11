// ***************************************************************************************
//    Project: Dang Compiler -> https://github.com/dezashibi-c/dang
//    File: token.c
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
// *  Description: token struct and related functionalities
// ***************************************************************************************

#include "token.h"

string tostr_TokenType(TokenType enum_item)
{
    {
        switch (enum_item)
        {
            tostr_enum_scase(TOK_ILLEGAL);
            tostr_enum_scase(TOK_EOF);
            tostr_enum_scase(TOK_IDENT);
            tostr_enum_scase(TOK_INT);
            tostr_enum_scase(TOK_ASSIGN);
            tostr_enum_scase(TOK_PLUS);
            tostr_enum_scase(TOK_COMMA);
            tostr_enum_scase(TOK_SEMICOLON);
            tostr_enum_scase(TOK_NEWLINE);
            tostr_enum_scase(TOK_LPAREN);
            tostr_enum_scase(TOK_RPAREN);
            tostr_enum_scase(TOK_LBRACE);
            tostr_enum_scase(TOK_RBRACE);
            tostr_enum_scase(TOK_FUNCTION);
            tostr_enum_scase(TOK_LET);
        };

        return NULL;
    }
}

TokenType is_keyword(string text)
{
    if (strcmp(text, "fn") == 0)
        return TOK_FUNCTION;
    else if (strcmp(text, "let") == 0)
        return TOK_LET;
    else
        return TOK_IDENT;
}

Token* token_make(TokenType type)
{
    Token* token = malloc(sizeof(Token));
    token->type = type;
    token->text = "";

    return token;
}


Token* token_make_from_char(TokenType type, byte c)
{
    // Allocate memory for the token
    Token* token = (Token*)malloc(sizeof(Token));
    if (!token) return NULL; // Handle memory allocation failure

    // Allocate memory for the text field (2 bytes: 1 char + null terminator)
    token->text = (char*)malloc(2);
    if (!token->text)
    {
        free(token); // Free allocated token in case of failure
        return NULL;
    }

    // Set the character and null-terminate the string
    token->text[0] = c;
    token->text[1] = '\0';

    // Set the token type
    token->type = type;

    return token;
}

Token* token_make_from_string(TokenType type, string str)
{
    // Allocate memory for the token
    Token* token = (Token*)malloc(sizeof(Token));
    if (!token) return NULL; // Handle memory allocation failure

    // Allocate memory for the text field based on the length of the input
    // string
    size_t length = strlen(str);
    token->text = (char*)malloc(length + 1); // +1 for the null terminator
    if (!token->text)
    {
        free(token); // Free allocated token in case of failure
        return NULL;
    }

    // Copy the input string into the token's text field
    strcpy(token->text, str);

    // Set the token type
    token->type = type;

    return token;
}

Token* token_make_from_string_portion(string str, usize start, usize len)
{
    if (!str || start >= strlen(str))
    {
        return NULL; // Handle invalid input
    }

    // Allocate memory for the token
    Token* token = (Token*)malloc(sizeof(Token));
    if (!token) return NULL; // Handle memory allocation failure

    token->text = (string)malloc(len + 1);
    if (!token->text)
    {
        free(token); // Clean up in case of failure
        return NULL;
    }

    strncpy(token->text, str + start, len);

    token->text[len] = '\0';

    return token;
}
