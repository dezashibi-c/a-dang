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
void dc_dynarr_init(DCDynArr* darr)
{
    darr->cap = DC_DYNARR_INITIAL_CAP;
    darr->count = 0;
    darr->elements = malloc(DC_DYNARR_INITIAL_CAP * sizeof(DCDynValue));
    if (darr->elements == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
}

// Function to initialize the dynamic array with initial values
void ___dc_dynarr_init_with_values(DCDynArr* darr, usize count,
                                   DCDynValue values[])
{
    darr->cap = count;
    darr->count = 0;
    darr->elements = malloc((count == 0 ? DC_DYNARR_INITIAL_CAP : count) *
                            sizeof(DCDynValue));
    if (darr->elements == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    for (usize i = 0; i < count; ++i)
    {
        dc_dynarr_add(darr, values[i]);
    }
}

// Function to add an element to the dynamic array
void dc_dynarr_add(DCDynArr* darr, DCDynValue value)
{
    if (darr->count >= darr->cap)
    {
        // Resize the array if needed (double the capacity)
        darr->cap *= 2;
        darr->elements =
            realloc(darr->elements, darr->cap * sizeof(DCDynValue));
        if (darr->elements == NULL)
        {
            fprintf(stderr, "Memory reallocation failed\n");
            exit(EXIT_FAILURE);
        }
    }
    // Add the new element with its type and value
    darr->elements[darr->count] = value;
    darr->count++;
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

            case DC_DYN_VAL_TYPE_byte:
                if (element->value.byte_val == el->value.byte_val)
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


void dc_dynarr_value_free(DCDynValue* element, void (*custom_free)(void*))
{
    switch (element->type)
    {
        case DC_DYN_VAL_TYPE_string:
            if (custom_free)
            {
                custom_free(element->value.string_val);
            }
            else
            {
                free(element->value.string_val);
            }
            break;

        case DC_DYN_VAL_TYPE_voidptr:
            if (custom_free)
            {
                custom_free(element->value.voidptr_val);
            }
            else
            {
                free(element->value.voidptr_val);
            }
            break;

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
        dc_dynarr_value_free(&darr->elements[i], NULL);
    }

    free(darr->elements);

    darr->elements = NULL;
    darr->cap = 0;
    darr->count = 0;
}

void dc_dynarr_delete(DCDynArr* darr, usize index, void (*custom_free)(void*))
{
    if (index >= darr->count)
    {
        return;
    }

    // Free the element at the specified index
    dc_dynarr_value_free(&darr->elements[index], custom_free);

    // Shift the elements after the deleted one to fill the gap
    for (usize i = index; i < darr->count - 1; i++)
    {
        darr->elements[i] = darr->elements[i + 1];
    }

    darr->count--;

    // Optional: shrink the capacity if too much unused space
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
}

___dc_dynval_converters_decl(u8)
{
    ___dc_dynval_converters_impl(u8);
}
___dc_dynval_converters_decl(i32)
{
    ___dc_dynval_converters_impl(i32);
}
___dc_dynval_converters_decl(u32)
{
    ___dc_dynval_converters_impl(u32);
}
___dc_dynval_converters_decl(u64)
{
    ___dc_dynval_converters_impl(u64);
}
___dc_dynval_converters_decl(f32)
{
    ___dc_dynval_converters_impl(f32);
}
___dc_dynval_converters_decl(f64)
{
    ___dc_dynval_converters_impl(f64);
}
___dc_dynval_converters_decl(uptr)
{
    ___dc_dynval_converters_impl(uptr);
}
___dc_dynval_converters_decl(byte)
{
    ___dc_dynval_converters_impl(byte);
}
___dc_dynval_converters_decl(size)
{
    ___dc_dynval_converters_impl(size);
}
___dc_dynval_converters_decl(usize)
{
    ___dc_dynval_converters_impl(usize);
}
___dc_dynval_converters_decl(string)
{
    ___dc_dynval_converters_impl(string);
}
___dc_dynval_converters_decl(voidptr)
{
    ___dc_dynval_converters_impl(voidptr);
}
