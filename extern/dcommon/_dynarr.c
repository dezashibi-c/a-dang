// ***************************************************************************************
//    Project: dcommon -> https://github.com/dezashibi-c/dcommon
//    File: _da.c
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
    "You cannot link to this source (_da.c) directly, please consider including dcommon.h"
#endif

#include "_headers/aliases.h"
#include "_headers/general.h"
#include "_headers/macros.h"

DCResultBool dc_dv_as_bool(DCDynVal* dv)
{
    DC_RES_bool();

    // NULL DCDynVal is going to turn into false value
    if (!dv) dc_res_ret_ok(false);

#define type_to_bool(TYPE)                                                     \
    case dc_dvt(TYPE):                                                         \
        dc_res_ret_ok(dc_as_bool(TYPE, dv->value.TYPE##_val));

    switch (dv->type)
    {
        type_to_bool(u8);
        type_to_bool(u16);
        type_to_bool(u32);
        type_to_bool(u64);
        type_to_bool(i8);
        type_to_bool(i16);
        type_to_bool(i32);
        type_to_bool(i64);
        type_to_bool(f32);
        type_to_bool(f64);
        type_to_bool(uptr);
        type_to_bool(char);
        type_to_bool(string);
        type_to_bool(voidptr);
        type_to_bool(fileptr);
        type_to_bool(size);
        type_to_bool(usize);

        default:
            break;
    };

    dc_dbg_log("Exiting Function on an unknown type");
    dc_res_ret_e(3, "unknown dynamic value type");
#undef type_to_bool
}

DCResultVoid dc_da_init(DCDynArr* darr, DCDynValFreeFn element_free_fn)
{
    DC_RES_void();

    if (!darr)
    {
        dc_dbg_log("got NULL DCDynArr");

        dc_res_ret_e(1, "got NULL DCDynArr");
    }

    darr->cap = DC_DA_INITIAL_CAP;
    darr->count = 0;
    darr->multiplier = DC_DA_CAP_MULTIPLIER;

    darr->element_free_fn = element_free_fn;

    darr->elements = malloc(DC_DA_INITIAL_CAP * sizeof(DCDynVal));
    if (darr->elements == NULL)
    {
        dc_dbg_log("Memory allocation failed");

        dc_res_ret_e(2, "Memory allocation failed");
    }

    dc_res_ret();
}

DCResultVoid dc_da_init2(DCDynArr* darr, usize capacity,
                         usize capacity_grow_multiplier,
                         DCDynValFreeFn element_free_fn)
{
    DC_RES_void();

    if (!darr)
    {
        dc_dbg_log("got NULL DCDynArr");

        dc_res_ret_e(1, "got NULL DCDynArr");
    }

    darr->cap = capacity;
    darr->count = 0;
    darr->multiplier = capacity_grow_multiplier;

    darr->element_free_fn = element_free_fn;

    darr->elements = malloc(capacity * sizeof(DCDynVal));

    if (darr->elements == NULL)
    {
        dc_dbg_log("Memory allocation failed");

        dc_res_ret_e(2, "Memory allocation failed");
    }

    dc_res_ret();
}


DCResultDa dc_da_new(DCDynValFreeFn element_free_fn)
{
    DC_RES_da();

    DCDynArr* darr = malloc(sizeof(DCDynArr));
    if (darr == NULL)
    {
        dc_dbg_log("Memory allocation failed");

        dc_res_ret_e(2, "Memory allocation failed");
    }

    dc_try_fail_temp(DCResultVoid, dc_da_init(darr, element_free_fn));

    dc_res_ret_ok(darr);
}

DCResultDa dc_da_new2(usize capacity, usize capacity_grow_multiplier,
                      DCDynValFreeFn element_free_fn)
{
    DC_RES_da();

    DCDynArr* darr = malloc(sizeof(DCDynArr));
    if (darr == NULL)
    {
        dc_dbg_log("Memory allocation failed");

        dc_res_ret_e(2, "Memory allocation failed");
    }

    dc_try_fail_temp(
        DCResultVoid,
        dc_da_init2(darr, capacity, capacity_grow_multiplier, element_free_fn));

    dc_res_ret_ok(darr);
}

DCResultVoid __dc_da_init_with_values(DCDynArr* darr, usize count,
                                      DCDynValFreeFn element_free_fn,
                                      DCDynVal values[])
{
    DC_RES_void();

    if (!darr)
    {
        dc_dbg_log("got NULL DCDynArr");

        dc_res_ret_e(1, "got NULL DCDynArr");
    }

    darr->cap = count;
    darr->count = 0;
    darr->multiplier = DC_DA_CAP_MULTIPLIER;

    darr->element_free_fn = element_free_fn;

    darr->elements =
        malloc((count == 0 ? DC_DA_INITIAL_CAP : count) * sizeof(DCDynVal));
    if (darr->elements == NULL)
    {
        dc_dbg_log("Memory allocation failed");

        dc_res_ret_e(2, "Memory allocation failed");
    }

    for (usize i = 0; i < count; ++i)
    {
        dc_try_fail(dc_da_push(darr, values[i]));
    }

    dc_res_ret();
}

DCResultVoid dc_da_grow(DCDynArr* darr)
{
    DC_RES_void();

    if (!darr)
    {
        dc_dbg_log("got NULL DCDynArr");

        dc_res_ret_e(1, "got NULL DCDynArr");
    }

    if (darr->cap > (SIZE_MAX / darr->multiplier) / sizeof(DCDynVal))
    {
        dc_dbg_log("Array size too large, cannot allocate more memory");

        dc_res_ret_e(2, "Array size too large, cannot allocate more memory");
    }

    // Resize the array if needed (double the capacity by default or custom
    // multiplier of provided beforehand)
    DCDynVal* resized = realloc(darr->elements, darr->cap * darr->multiplier *
                                                    sizeof(DCDynVal));
    if (resized == NULL)
    {
        dc_dbg_log("Memory re-allocation failed");

        dc_res_ret_e(2, "Memory re-allocation failed");
    }

    darr->elements = resized;
    darr->cap *= darr->multiplier;

    dc_res_ret();
}

DCResultVoid dc_da_grow_by(DCDynArr* darr, usize amount)
{
    DC_RES_void();

    if (!darr)
    {
        dc_dbg_log("got NULL DCDynArr");

        dc_res_ret_e(1, "got NULL DCDynArr");
    }

    DCDynVal* resized =
        realloc(darr->elements, (darr->cap + amount) * sizeof(DCDynVal));

    if (resized == NULL)
    {
        dc_dbg_log("Memory re-allocation failed");

        dc_res_ret_e(2, "Memory re-allocation failed");
    }

    darr->cap += amount;
    darr->elements = resized;

    dc_res_ret();
}

DCResultVoid dc_da_grow_to(DCDynArr* darr, usize amount)
{
    DC_RES_void();

    if (!darr)
    {
        dc_dbg_log("got NULL DCDynArr");

        dc_res_ret_e(1, "got NULL DCDynArr");
    }

    DCDynVal* resized = realloc(darr->elements, amount * sizeof(DCDynVal));

    if (resized == NULL)
    {
        dc_dbg_log("Memory re-allocation failed");

        dc_res_ret_e(2, "Memory re-allocation failed");
    }

    darr->cap = amount;
    darr->elements = resized;

    dc_res_ret();
}

DCResultVoid dc_da_trunc(DCDynArr* darr)
{
    DC_RES_void();

    if (!darr)
    {
        dc_dbg_log("got NULL DCDynArr");

        dc_res_ret_e(1, "got NULL DCDynArr");
    }

    if (darr->count < darr->cap)
    {
        DCDynVal* resized =
            realloc(darr->elements, darr->count * sizeof(DCDynVal));

        if (resized == NULL)
        {
            dc_dbg_log("Memory re-allocation failed");

            dc_res_ret_e(2, "Memory re-allocation failed");
        }

        darr->cap = darr->count;
        darr->elements = resized;
    }

    dc_res_ret();
}

DCResultVoid dc_da_pop(DCDynArr* darr, usize count, DCDynVal** out_popped,
                       bool truncate)
{
    DC_RES_void();

    if (!darr)
    {
        dc_dbg_log("got NULL DCDynArr");

        dc_res_ret_e(1, "got NULL DCDynArr");
    }

    if (count > darr->count)
    {
        dc_dbg_log("Try to pop elements more than actual number of elements");

        dc_res_ret_e(4,
                     "Try to pop elements more than actual number of elements");
    }

    if (out_popped != NULL)
    {
        *out_popped = (DCDynVal*)malloc(count * sizeof(DCDynVal));

        if (!out_popped)
        {
            dc_dbg_log("Memory re-allocation failed");

            dc_res_ret_e(2, "Memory re-allocation failed");
        }
    }

    usize last_item_index = darr->count - 1;

    for (i32 i = count - 1; i >= 0; --i)
    {
        if (out_popped) (*out_popped)[i] = darr->elements[last_item_index - i];

        dc_try_fail(dc_dv_free(&darr->elements[last_item_index - i],
                               darr->element_free_fn));

        darr->count--;
    }

    if (truncate) dc_try_fail(dc_da_trunc(darr));

    dc_res_ret();
}

DCResultVoid dc_da_push(DCDynArr* darr, DCDynVal value)
{
    DC_RES_void();

    if (!darr)
    {
        dc_dbg_log("got NULL DCDynArr");

        dc_res_ret_e(1, "got NULL DCDynArr");
    }

    if (darr->count >= darr->cap) dc_try_fail(dc_da_grow(darr));

    // Add the new element with its type and value
    darr->elements[darr->count] = value;
    darr->count++;

    dc_res_ret();
}

DCResultVoid __dc_da_append_values(DCDynArr* darr, usize count,
                                   DCDynVal values[])
{
    DC_RES_void();

    if (!darr)
    {
        dc_dbg_log("got NULL DCDynArr");

        dc_res_ret_e(1, "got NULL DCDynArr");
    }

    if ((count + darr->count) > darr->cap)
        dc_try_fail(dc_da_grow_to(darr, count + darr->count));

    for (usize i = 0; i < count; ++i)
    {
        dc_try_fail(dc_da_push(darr, values[i]));
    }

    dc_res_ret();
}

DCResultVoid dc_da_append(DCDynArr* darr, DCDynArr* from)
{
    DC_RES_void();

    if (!darr || !from)
    {
        dc_dbg_log("got NULL DCDynArr");

        dc_res_ret_e(1, "got NULL DCDynArr");
    }

    dc_try_fail(__dc_da_append_values(darr, from->count, from->elements));

    dc_res_ret();
}

DCResultDv dc_da_get(DCDynArr* darr, usize index)
{
    DC_RES_dv();

    if (!darr)
    {
        dc_dbg_log("got NULL DCDynArr");

        dc_res_ret_e(1, "got NULL DCDynArr");
    }

    if (index >= darr->count)
    {
        dc_dbg_log("Index out of bound - try to get index='%" PRIuMAX
                   "' out of actual '%" PRIuMAX "' elements.",
                   index, darr->count);

        dc_res_ret_e(4, "Index out of bound");
    }

    dc_res_ret_ok(&darr->elements[index]);
}

DCResultBool dc_dv_eq(DCDynVal* dv1, DCDynVal* dv2)
{
    DC_RES_bool();

    if (!dv1 || !dv2)
    {
        dc_dbg_log("cannot compare DCDynVal with NULL");

        dc_res_ret_e(1, "cannot compare DCDynVal with NULL");
    }

    if (dv1->type != dv2->type) dc_res_ret_ok(false);

#define check_eq(TYPE)                                                         \
    case dc_dvt(TYPE):                                                         \
        if (dv1->value.TYPE##_val == dv2->value.TYPE##_val)                    \
            dc_res_ret_ok(true);                                               \
        break

    switch (dv1->type)
    {
        check_eq(i8);
        check_eq(i16);
        check_eq(i32);
        check_eq(i64);

        check_eq(u8);
        check_eq(u16);
        check_eq(u32);
        check_eq(u64);

        check_eq(f32);
        check_eq(f64);

        check_eq(uptr);
        check_eq(char);

        case dc_dvt(string):
        {
            if (strcmp(dv1->value.string_val, dv2->value.string_val) == 0)
                dc_res_ret_ok(true);
            break;
        }

            // clang-format off
        check_eq(voidptr);
        check_eq(fileptr);
        check_eq(size);
        check_eq(usize);
            // clang-format on

        default:
            break;
    }

    dc_res_ret_ok(false);

#undef check_eq
}

DCResultUsize dc_da_findp(DCDynArr* darr, DCDynVal* el)
{
    DC_RES_usize();

    if (!darr)
    {
        dc_dbg_log("got NULL DCDynArr");

        dc_res_ret_e(1, "got NULL DCDynArr");
    }

#define find_if(TYPE, INDEX)                                                   \
    case dc_dvt(TYPE):                                                         \
        if (element->value.TYPE##_val == el->value.TYPE##_val)                 \
            dc_res_ret_ok(INDEX);                                              \
        break

    for (usize i = 0; i < darr->count; i++)
    {
        DCDynVal* element = &darr->elements[i];
        if (element->type != el->type)
        {
            continue; // Skip if the type doesn't match
        }

        // check the actual value based on the type
        switch (el->type)
        {
            find_if(i8, i);
            find_if(i16, i);
            find_if(i32, i);
            find_if(i64, i);

            find_if(u8, i);
            find_if(u16, i);
            find_if(u32, i);
            find_if(u64, i);

            find_if(f32, i);
            find_if(f64, i);

            find_if(uptr, i);
            find_if(char, i);

            case dc_dvt(string):
            {
                if (strcmp(element->value.string_val, el->value.string_val) ==
                    0)
                    dc_res_ret_ok(i);
                break;
            }

                // clang-format off
            find_if(voidptr, i);
            find_if(fileptr, i);
            find_if(size, i);
            find_if(usize, i);
                // clang-format on

            default:
                break;
        }
    }

#undef find_if

    dc_dbg_log("Not Found");

    dc_res_ret_e(6, "Not Found");
}

DCResultUsize dc_da_find(DCDynArr* darr, DCDynVal el)
{
    return dc_da_findp(darr, &el);
}

DCResultVoid dc_dv_free(DCDynVal* element, DCDynValFreeFn custom_free_fn)
{
    DC_RES_void();

    if (!element) dc_res_ret();

    switch (element->type)
    {
        case DC_DYN_VAL_TYPE_string:
        {
            if (custom_free_fn)
                dc_try_fail_temp(DCResultVoid, custom_free_fn(element));

            if (dc_dv_is_allocated(*element) &&
                dc_dv_as(*element, string) != NULL)
                free(dc_dv_as(*element, string));

            dc_dv_set(*element, string, "");
            break;
        }

        case DC_DYN_VAL_TYPE_fileptr:
        {
            if (custom_free_fn)
                dc_try_fail_temp(DCResultVoid, custom_free_fn(element));

            if (dc_dv_is_allocated(*element) &&
                dc_dv_as(*element, fileptr) != NULL)
                fclose(dc_dv_as(*element, fileptr));

            dc_dv_set(*element, fileptr, NULL);
            break;
        }

        case DC_DYN_VAL_TYPE_voidptr:
        {
            if (custom_free_fn)
                dc_try_fail_temp(DCResultVoid, custom_free_fn(element));

            if (dc_dv_is_allocated(*element) &&
                dc_dv_as(*element, voidptr) != NULL)
                free(dc_dv_as(*element, voidptr));

            dc_dv_set(*element, voidptr, NULL);
            break;
        }

        // Do nothing for literal types (integer, float, etc.)
        default:
            dc_dbg_log("Doesn't free anything - not allocated type");
            break;
    }

    dc_res_ret();
}

DCResultVoid __dc_dv_free(voidptr dv)
{
    DC_RES_void();

    if (!dv)
    {
        dc_dbg_log("got NULL voidptr");

        dc_res_ret_e(1, "got NULL voidptr");
    }

    return dc_dv_free((DCDynVal*)dv, NULL);
}

DCResultVoid dc_da_free(DCDynArr* darr)
{
    DC_RES_void();

    if (!darr) dc_res_ret();

    for (usize i = 0; i < darr->count; ++i)
    {
        dc_try_fail_temp(DCResultVoid,
                         dc_dv_free(&darr->elements[i], darr->element_free_fn));
    }

    free(darr->elements);

    darr->elements = NULL;
    darr->cap = 0;
    darr->count = 0;

    dc_res_ret();
}

DCResultVoid __dc_da_free(voidptr darr)
{
    DC_RES_void();

    if (!darr)
    {
        dc_dbg_log("t NULL voidptr");

        dc_res_ret_e(1, "got NULL voidptr");
    }

    return dc_da_free((DCDynArr*)darr);
}

DCResultVoid dc_da_delete(DCDynArr* darr, usize index)
{
    DC_RES_void();

    if (!darr)
    {
        dc_dbg_log("got NULL DCDynArr");

        dc_res_ret_e(1, "got NULL DCDynArr");
    }

    if (index >= darr->count)
    {
        dc_dbg_log("Index out of bound - try to get index='%" PRIuMAX
                   "' out of actual '%" PRIuMAX "' elements.",
                   index, darr->count);

        dc_res_ret_e(4, "Index out of bound");
    }

    // Free the element at the specified index
    dc_try_fail_temp(DCResultVoid,
                     dc_dv_free(&darr->elements[index], darr->element_free_fn));

    // Shift the elements after the deleted one to fill the gap
    memmove(&darr->elements[index], &darr->elements[index + 1],
            (darr->count - index - 1) * sizeof(DCDynVal));

    darr->count--;

    dc_res_ret();
}

DCResultVoid dc_da_delete_elp(DCDynArr* darr, DCDynVal* el)
{
    DC_RES_void();

    if (!darr || !el)
    {
        dc_dbg_log("got NULL DCDynArr or element");

        dc_res_ret_e(1, "got NULL DCDynArr or element");
    }

    DCResultUsize found_res = dc_da_findp(darr, el);
    dc_res_fail_if_err2(found_res);

    usize index = dc_res_val2(found_res);

    return dc_da_delete(darr, index);
}


DCResultVoid dc_da_delete_el(DCDynArr* darr, DCDynVal el)
{
    return dc_da_delete_elp(darr, &el);
}

DCResultVoid dc_da_insert(DCDynArr* darr, usize index, DCDynVal value)
{
    DC_RES_void();

    if (!darr)
    {
        dc_dbg_log("got NULL DCDynArr");

        dc_res_ret_e(1, "got NULL DCDynArr");
    }

    if (index > darr->count)
    {
        dc_dbg_log("Index out of bound - try to get index='%" PRIuMAX
                   "' out of actual '%" PRIuMAX "' elements.",
                   index, darr->count);

        dc_res_ret_e(4, "Index out of bound");
    }

    if (darr->count >= darr->cap)
        dc_try_fail_temp(DCResultVoid, dc_da_grow(darr));

    // No need to memmove if inserting at the end
    if (index < darr->count)
    {
        // Shift elements starting from index to index + 1
        memmove(&darr->elements[index + 1], &darr->elements[index],
                (darr->count - index) * sizeof(DCDynVal));
    }

    // Add the new value at the desired index
    darr->elements[index] = value;
    darr->count++;

    dc_res_ret();
}

DCResultVoid __dc_da_insert_values(DCDynArr* darr, usize start_index,
                                   usize count, DCDynVal values[])
{
    DC_RES_void();

    if (!darr)
    {
        dc_dbg_log("got NULL DCDynArr");

        dc_res_ret_e(1, "got NULL DCDynArr");
    }

    if (start_index > darr->count)
    {
        dc_dbg_log("Index out of bound - try to get index='%" PRIuMAX
                   "' out of actual '%" PRIuMAX "' elements.",
                   start_index, darr->count);

        dc_res_ret_e(4, "Index out of bound");
    }

    if ((count + darr->count) > darr->cap)
        dc_try_fail_temp(DCResultVoid,
                         dc_da_grow_to(darr, count + darr->count));

    // No need to memmove if inserting at the end
    if (start_index < darr->count)
    {
        // Shift elements starting from index to index + 1
        memmove(&darr->elements[start_index + count],
                &darr->elements[start_index],
                (darr->count - start_index) * sizeof(DCDynVal));
    }

    for (usize i = 0; i < count; ++i)
    {
        darr->elements[start_index + i] = values[i];
    }

    darr->count += count;

    dc_res_ret();
}

DCResultVoid dc_da_insert_from(DCDynArr* darr, usize start_index,
                               DCDynArr* from)
{
    DC_RES_void();

    if (!darr || !from)
    {
        dc_dbg_log("got NULL DCDynArr");

        dc_res_ret_e(1, "got NULL DCDynArr");
    }

    dc_try_fail(
        __dc_da_insert_values(darr, start_index, from->count, from->elements));

    dc_res_ret();
}

DCResultUsize dc_i8_da_to_flat_arr(DCDynArr* arr, i8** out_arr, _Bool must_fail)
{
    __DC_DA_CONVERT_IMPL(i8);
}

DCResultUsize dc_i16_da_to_flat_arr(DCDynArr* arr, i16** out_arr,
                                    _Bool must_fail)
{
    __DC_DA_CONVERT_IMPL(i16);
}

DCResultUsize dc_i32_da_to_flat_arr(DCDynArr* arr, i32** out_arr,
                                    _Bool must_fail)
{
    __DC_DA_CONVERT_IMPL(i32);
}

DCResultUsize dc_i64_da_to_flat_arr(DCDynArr* arr, i64** out_arr,
                                    _Bool must_fail)
{
    __DC_DA_CONVERT_IMPL(i64);
}

DCResultUsize dc_u8_da_to_flat_arr(DCDynArr* arr, u8** out_arr, _Bool must_fail)
{
    __DC_DA_CONVERT_IMPL(u8);
}

DCResultUsize dc_u16_da_to_flat_arr(DCDynArr* arr, u16** out_arr,
                                    _Bool must_fail)
{
    __DC_DA_CONVERT_IMPL(u16);
}

DCResultUsize dc_u32_da_to_flat_arr(DCDynArr* arr, u32** out_arr,
                                    _Bool must_fail)
{
    __DC_DA_CONVERT_IMPL(u32);
}

DCResultUsize dc_u64_da_to_flat_arr(DCDynArr* arr, u64** out_arr,
                                    _Bool must_fail)
{
    __DC_DA_CONVERT_IMPL(u64);
}

DCResultUsize dc_f32_da_to_flat_arr(DCDynArr* arr, f32** out_arr,
                                    _Bool must_fail)
{
    __DC_DA_CONVERT_IMPL(f32);
}

DCResultUsize dc_f64_da_to_flat_arr(DCDynArr* arr, f64** out_arr,
                                    _Bool must_fail)
{
    __DC_DA_CONVERT_IMPL(f64);
}

DCResultUsize dc_uptr_da_to_flat_arr(DCDynArr* arr, uptr** out_arr,
                                     _Bool must_fail)
{
    __DC_DA_CONVERT_IMPL(uptr);
}

DCResultUsize dc_char_da_to_flat_arr(DCDynArr* arr, char** out_arr,
                                     _Bool must_fail)
{
    __DC_DA_CONVERT_IMPL(char);
}

DCResultUsize dc_size_da_to_flat_arr(DCDynArr* arr, size** out_arr,
                                     _Bool must_fail)
{
    __DC_DA_CONVERT_IMPL(size);
}

DCResultUsize dc_usize_da_to_flat_arr(DCDynArr* arr, usize** out_arr,
                                      _Bool must_fail)
{
    __DC_DA_CONVERT_IMPL(usize);
}

DCResultUsize dc_string_da_to_flat_arr(DCDynArr* arr, string** out_arr,
                                       _Bool must_fail)
{
    __DC_DA_CONVERT_IMPL(string);
}

DCResultUsize dc_voidptr_da_to_flat_arr(DCDynArr* arr, voidptr** out_arr,
                                        _Bool must_fail)
{
    __DC_DA_CONVERT_IMPL(voidptr);
}

DCResultUsize dc_fileptr_da_to_flat_arr(DCDynArr* arr, fileptr** out_arr,
                                        _Bool must_fail)
{
    __DC_DA_CONVERT_IMPL(fileptr);
}
