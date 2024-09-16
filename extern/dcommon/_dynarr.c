// ***************************************************************************************
//    Project: dcommon -> https://github.com/dezashibi-c/dcommon
//    File: _dynarr.c
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
// *  Description: private implementation file for definition of common helper
// *               functions
// *               DO NOT LINK TO THIS DIRECTLY
// ***************************************************************************************

#ifndef __DC_BYPASS_PRIVATE_PROTECTION
#error                                                                         \
    "You cannot link to this source (_dynarr.c) directly, please consider including dcommon.h"
#endif

#include "_headers/aliases.h"
#include "_headers/general.h"
#include "_headers/macros.h"

// Function to initialize the dynamic array
void dc_dynarr_init(DCDynArr* darr, DCDynValFreeFunc element_free_func)
{
    darr->cap = DC_DYNARR_INITIAL_CAP;
    darr->count = 0;
    darr->element_free_func = element_free_func;
    darr->elements = malloc(DC_DYNARR_INITIAL_CAP * sizeof(DCDynValue));
    if (darr->elements == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
}

DCDynArr* dc_dynarr_create(DCDynValFreeFunc element_free_func)
{
    DCDynArr* darr = malloc(sizeof(DCDynArr));
    if (darr == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    dc_dynarr_init(darr, element_free_func);

    return darr;
}

// Function to initialize the dynamic array with initial values
void ___dc_dynarr_init_with_values(DCDynArr* darr, usize count,
                                   DCDynValFreeFunc element_free_func,
                                   DCDynValue values[])
{
    darr->cap = count;
    darr->count = 0;
    darr->element_free_func = element_free_func;
    darr->elements = malloc((count == 0 ? DC_DYNARR_INITIAL_CAP : count) *
                            sizeof(DCDynValue));
    if (darr->elements == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    for (usize i = 0; i < count; ++i)
    {
        dc_dynarr_push(darr, values[i]);
    }
}

void dc_dynarr_grow(DCDynArr* darr)
{
    // Resize the array if needed (double the capacity by default)
    DCDynValue* resized =
        realloc(darr->elements,
                darr->cap * DC_DYNARR_CAP_MULTIPLIER * sizeof(DCDynValue));
    if (resized == NULL)
    {
        fprintf(stderr, "Memory reallocation failed\n");

        exit(EXIT_FAILURE);
    }

    darr->cap *= DC_DYNARR_CAP_MULTIPLIER;
    darr->elements = resized;
}

// Function to add an element to the dynamic array
void dc_dynarr_push(DCDynArr* darr, DCDynValue value)
{
    if (darr->count >= darr->cap) dc_dynarr_grow(darr);

    // Add the new element with its type and value
    darr->elements[darr->count] = value;
    darr->count++;
}

DCDynValue* dc_dynarr_get(DCDynArr* darr, usize index)
{
    if (index >= darr->count) return NULL;

    return &darr->elements[index];
}

// Function to find an element in the dynamic array
DCDynValue* dc_dynarr_find(DCDynArr* darr, DCDynValue* el)
{
    for (usize i = 0; i < darr->count; i++)
    {
        DCDynValue* element = &darr->elements[i];
        if (element->type != el->type)
        {
            continue; // Skip if the type doesn't match
        }

        // check the actual value based on the type
        switch (el->type)
        {
            case DC_DYN_VAL_TYPE_u8:
                if (element->value.u8_val == el->value.u8_val) return element;
                break;

            case DC_DYN_VAL_TYPE_i32:
                if (element->value.i32_val == el->value.i32_val) return element;
                break;

            case DC_DYN_VAL_TYPE_u32:
                if (element->value.u32_val == el->value.u32_val) return element;
                break;

            case DC_DYN_VAL_TYPE_u64:
                if (element->value.u64_val == el->value.u64_val) return element;
                break;

            case DC_DYN_VAL_TYPE_f32:
                if (element->value.f32_val == el->value.f32_val) return element;
                break;

            case DC_DYN_VAL_TYPE_f64:
                if (element->value.f64_val == el->value.f64_val) return element;
                break;

            case DC_DYN_VAL_TYPE_uptr:
                if (element->value.uptr_val == el->value.uptr_val)
                    return element;
                break;

            case DC_DYN_VAL_TYPE_char:
                if (element->value.char_val == el->value.char_val)
                    return element;
                break;

            case DC_DYN_VAL_TYPE_string:
                if (strcmp(element->value.string_val, el->value.string_val) ==
                    0)
                    return element;
                break;

            case DC_DYN_VAL_TYPE_voidptr:
                if (element->value.voidptr_val == el->value.voidptr_val)
                    return element;
                break;

            default:
                break;
        }
    }
    return NULL; // Return NULL if not found
}

void dc_dynval_free(DCDynValue* element, void (*custom_free)(DCDynValue*))
{
    switch (element->type)
    {
        case DC_DYN_VAL_TYPE_string:
        {
            if (custom_free) custom_free(element);

            if (dc_dynval_get(*element, string) != NULL)
                free(dc_dynval_get(*element, string));
            break;
        }

        case DC_DYN_VAL_TYPE_voidptr:
        {
            if (custom_free) custom_free(element);

            if (dc_dynval_get(*element, voidptr) != NULL)
                free(dc_dynval_get(*element, voidptr));
            break;
        }

        // Do nothing for literal types (integer, float, etc.)
        default:
            break;
    }
}


// Function to free the dynamic array
void dc_dynarr_free(DCDynArr* darr)
{
    for (usize i = 0; i < darr->count; ++i)
    {
        dc_dynval_free(&darr->elements[i], darr->element_free_func);
    }

    free(darr->elements);

    darr->elements = NULL;
    darr->cap = 0;
    darr->count = 0;
}

bool dc_dynarr_delete(DCDynArr* darr, usize index)
{
    if (index >= darr->count) return false;

    // Free the element at the specified index
    dc_dynval_free(&darr->elements[index], darr->element_free_func);

    // Shift the elements after the deleted one to fill the gap
    memmove(&darr->elements[index], &darr->elements[index + 1],
            (darr->count - index - 1) * sizeof(DCDynValue));

    darr->count--;

    // shrink the capacity if too much unused space
    if (darr->count < darr->cap / 4 && darr->cap > DC_DYNARR_INITIAL_CAP)
    {
        darr->cap /= 2;
        darr->elements =
            realloc(darr->elements, darr->cap * sizeof(DCDynValue));
        if (darr->elements == NULL)
        {
            fprintf(stderr, "Memory reallocation failed\n");
            exit(EXIT_FAILURE);
        }
    }

    return true;
}

void dc_dynarr_insert(DCDynArr* darr, usize index, DCDynValue value)
{
    if (darr->count >= darr->cap) dc_dynarr_grow(darr);

    memmove(&darr->elements[index + 1], &darr->elements[index],
            (darr->count - index) * sizeof(DCDynValue));

    // Add the new value at the desired index
    darr->elements[index] = value;
    darr->count++;
}

___dc_dynarr_converters_decl(u8)
{
    ___dc_dynarr_converters_impl(u8);
}
___dc_dynarr_converters_decl(i32)
{
    ___dc_dynarr_converters_impl(i32);
}
___dc_dynarr_converters_decl(u32)
{
    ___dc_dynarr_converters_impl(u32);
}
___dc_dynarr_converters_decl(u64)
{
    ___dc_dynarr_converters_impl(u64);
}
___dc_dynarr_converters_decl(f32)
{
    ___dc_dynarr_converters_impl(f32);
}
___dc_dynarr_converters_decl(f64)
{
    ___dc_dynarr_converters_impl(f64);
}
___dc_dynarr_converters_decl(uptr)
{
    ___dc_dynarr_converters_impl(uptr);
}
___dc_dynarr_converters_decl(char)
{
    ___dc_dynarr_converters_impl(char);
}
___dc_dynarr_converters_decl(size)
{
    ___dc_dynarr_converters_impl(size);
}
___dc_dynarr_converters_decl(usize)
{
    ___dc_dynarr_converters_impl(usize);
}
___dc_dynarr_converters_decl(string)
{
    ___dc_dynarr_converters_impl(string);
}
___dc_dynarr_converters_decl(voidptr)
{
    ___dc_dynarr_converters_impl(voidptr);
}
