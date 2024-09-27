// ***************************************************************************************
//    Project: dcommon -> https://github.com/dezashibi-c/dcommon
//    File: _dynht.c
//    Date: 2024-09-15
//    Author: Navid Dezashibi
//    Contact: navid@dezashibi.com
//    Website: https://dezashibi.com | https://github.com/dezashibi
//    License:
//     Please refer to the LICENSE file, repository or website for more
//     information about the licensing of this work. If you have any questions
//     or concerns, please feel free to contact me at the email address provided
//     above.
// ***************************************************************************************
// *  Description: private implementation file for definition of Hash Table
// *               Functionalities
// *               DO NOT LINK TO THIS DIRECTLY
// ***************************************************************************************

#ifndef __DC_BYPASS_PRIVATE_PROTECTION
#error                                                                         \
    "You cannot link to this source (_dynht.c) directly, please consider including dcommon.h"
#endif

#include "dcommon.h"

DCResultVoid dc_ht_init(DCHashTable* ht, usize capacity, DCHashFn hash_fn,
                        DCKeyCompFn key_cmp_fn, DCDynValFreeFn element_free_fn)
{
    DC_RES_void();

    if (!ht)
    {
        dc_dbg_log("got NULL DCHashTable");

        dc_res_ret_e(1, "got NULL DCHashTable");
    }

    ht->container = (DCDynArr*)calloc(capacity, sizeof(DCDynArr));

    if (ht->container == NULL)
    {
        dc_dbg_log("Memory allocation failed");

        dc_res_ret_e(2, "Memory allocation failed");
    }

    ht->cap = capacity;
    ht->key_count = 0;

    ht->hash_fn = hash_fn;
    ht->key_cmp_fn = key_cmp_fn;
    ht->element_free_fn = element_free_fn;

    dc_res_ret();
}

DCResultHt dc_ht_new(usize capacity, DCHashFn hash_fn, DCKeyCompFn key_cmp_fn,
                     DCDynValFreeFn element_free_fn)
{
    DC_RES_ht();

    DCHashTable* ht = (DCHashTable*)malloc(sizeof(DCHashTable));

    if (ht == NULL)
    {
        dc_dbg_log("Memory allocation failed");

        dc_res_ret_e(2, "Memory allocation failed");
    }

    dc_try_fail_temp(DCResultVoid, dc_ht_init(ht, capacity, hash_fn, key_cmp_fn,
                                              element_free_fn));

    dc_res_ret_ok(ht);
}

DCResultVoid dc_ht_free(DCHashTable* ht)
{
    DC_RES_void();

    if (!ht)
    {
        dc_dbg_log("got NULL DCHashTable");

        dc_res_ret_e(1, "got NULL DCHashTable");
    }

    if (ht && ht->cap == 0) dc_res_ret();

    for (usize i = 0; i < ht->cap; ++i)
    {
        dc_ht_get_container_row(darr, *ht, i);

        dc_try_fail_temp(DCResultVoid, dc_da_free(darr));
    }

    free(ht->container);

    ht->cap = 0;
    ht->key_count = 0;
    ht->hash_fn = NULL;
    ht->key_cmp_fn = NULL;
    ht->element_free_fn = NULL;

    dc_res_ret();
}

DCResultVoid __dc_ht_free(voidptr ht)
{
    DC_RES_void();

    if (!ht)
    {
        dc_dbg_log("got NULL DCHashTable");

        dc_res_ret_e(1, "got NULL DCHashTable");
    }

    dc_try_fail(dc_ht_free((DCHashTable*)ht));

    dc_res_ret();
}

DCResultUsize dc_ht_find_by_key(DCHashTable* ht, voidptr key,
                                DCDynVal** out_result)
{
    DC_RES_usize();

    if (!ht)
    {
        dc_dbg_log("got NULL DCHashTable");

        dc_res_ret_e(1, "got NULL DCHashTable");
    }

    dc_try_fail_temp_ht_get_hash(_index, *ht, key);

    dc_ht_get_container_row(darr, *ht, _index);

    dc_da_for(*darr)
    {
        DCDynVal* element = &darr->elements[_idx];
        if (element->type != dc_dvt(voidptr))
        {
            dc_dbg_log("wrong type, voidptr needed");

            dc_res_ret_e(3, "wrong type, voidptr needed");
        }

        voidptr element_key = ((DCHashEntry*)element->value.voidptr_val)->key;

        DCResultBool cmp_res = ht->key_cmp_fn(element_key, key);
        dc_res_fail_if_err2(cmp_res);

        if (dc_res_val2(cmp_res))
        {
            *out_result = &((DCHashEntry*)element->value.voidptr_val)->value;
            dc_res_ret_ok(_idx);
        }
    }

    *out_result = NULL;
    dc_res_ret_ok(0);
}

DCResultVoid dc_ht_set(DCHashTable* ht, voidptr key, DCDynVal value)
{
    DC_RES_void();

    if (!ht)
    {
        dc_dbg_log("got NULL DCHashTable");

        dc_res_ret_e(1, "got NULL DCHashTable");
    }

    dc_try_fail_temp_ht_get_hash(_index, *ht, key);

    DCHashEntry* new_entry = (DCHashEntry*)malloc(sizeof(DCHashEntry));
    if (new_entry == NULL)
    {
        dc_dbg_log("Memory allocation failed");

        dc_res_ret_e(2, "Memory allocation failed");
    }

    new_entry->key = key;
    new_entry->value = value;

    dc_ht_get_container_row(current, *ht, _index);

    if (current->cap == 0)
    {
        dc_try_fail(dc_da_init(current, ht->element_free_fn));
        dc_try_fail(dc_da_push(current, dc_dva(voidptr, new_entry)));

        ht->key_count++;

        dc_res_ret();
    }

    DCDynVal* existed = NULL;
    DCResultUsize find_res = dc_ht_find_by_key(ht, key, &existed);
    dc_res_fail_if_err2(find_res);

    usize existed_index = dc_res_val2(find_res);

    if (existed != NULL)
    {
        dc_try_fail(
            dc_dv_free(&current->elements[existed_index], ht->element_free_fn));

        current->elements[existed_index] = dc_dva(voidptr, new_entry);

        dc_res_ret();
    }

    dc_try_fail(dc_da_push(current, dc_dva(voidptr, new_entry)));
    ht->key_count++;

    dc_res_ret();
}

DCResultVoid __dc_ht_set_multiple(DCHashTable* ht, usize count,
                                  DCHashEntry entries[])
{
    DC_RES_void();

    if (!ht)
    {
        dc_dbg_log("got NULL DCHashTable");

        dc_res_ret_e(1, "got NULL DCHashTable");
    }

    for (usize i = 0; i < count; ++i)
    {
        dc_try_fail(dc_ht_set(ht, entries[i].key, entries[i].value));
    }

    dc_res_ret();
}

DCResultVoid dc_ht_merge(DCHashTable* ht, DCHashTable* from)
{
    DC_RES_void();

    if (!ht)
    {
        dc_dbg_log("got NULL DCHashTable");

        dc_res_ret_e(1, "got NULL DCHashTable");
    }

    for (usize i = 0; i < from->cap; ++i)
    {
        if (from->container[i].cap == 0) continue;

        for (usize j = 0; j < from->container[i].count; ++j)
        {
            DCHashEntry* entry = dc_da_get_as(from->container[i], j, voidptr);

            dc_try_fail(dc_ht_set(ht, entry->key, entry->value));
        }
    }

    dc_res_ret();
}

DCResultBool dc_ht_delete(DCHashTable* ht, voidptr key)
{
    DC_RES_bool();

    dc_try_fail_temp_ht_get_hash(_index, *ht, key);

    dc_ht_get_container_row(current, *ht, _index);

    if (current->count == 0) dc_res_ret_ok(false);

    DCDynVal* existed = NULL;
    DCResultUsize possible_bucket = dc_ht_find_by_key(ht, key, &existed);
    dc_res_fail_if_err2(possible_bucket);

    usize existed_index = dc_res_val2(possible_bucket);

    if (existed == NULL) dc_res_ret_ok(false);

    dc_try_fail_temp(DCResultVoid, dc_da_delete(current, existed_index));
    ht->key_count--;

    dc_res_ret_ok(true);
}

DCResultUsize dc_ht_keys(DCHashTable* ht, voidptr** out_arr)
{
    DC_RES_usize();

    if (!ht || ht->key_count == 0)
    {
        dc_dbg_log("got NULL or Empty DCHashTable");

        dc_res_ret_e(1, "got NULL or Empty DCHashTable");
    }

    if (!out_arr) dc_res_ret_e(1, "got NULL pout_arr");

    *out_arr = (voidptr*)malloc((ht->key_count + 1) * sizeof(voidptr));
    if (!(*out_arr))
    {
        dc_dbg_log("Memory allocation failed");

        dc_res_ret_e(2, "Memory allocation failed");
    }

    usize key_count = 0;
    for (usize i = 0; i < ht->cap; ++i)
    {
        dc_ht_get_container_row(darr, *ht, i);

        if (darr->cap == 0) continue;

        dc_da_for(*darr)
        {
            DCDynVal* elem = &darr->elements[_idx];
            if (elem->type != DC_DYN_VAL_TYPE_voidptr)
            {
                free(*out_arr);
                *out_arr = NULL;
                dc_dbg_log(
                    "Bad type, DCHashTable elements must be of type voidptr");

                dc_res_ret_e(
                    3,
                    "Bad type, DCHashTable elements must be of type voidptr");
            }

            voidptr element_key = ((DCHashEntry*)elem->value.voidptr_val)->key;

            (*out_arr)[key_count] = element_key;
            key_count++;
        }
    }

    (*out_arr)[ht->key_count] = NULL;
    dc_res_ret_ok(ht->key_count);
}
