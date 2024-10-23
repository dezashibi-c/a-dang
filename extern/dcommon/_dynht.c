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
#error "You cannot link to this source (_dynht.c) directly, please consider including dcommon.h"
#endif

#include "dcommon.h"

DCResVoid dc_ht_init(DCHashTable* ht, usize capacity, DCHashFn hash_fn, DCKeyCompFn key_cmp_fn, DCHtPairFreeFn pair_free_fn)
{
    DC_RES_void();

    if (!ht)
    {
        dc_dbg_log("got NULL DCHashTable");

        dc_ret_e(1, "got NULL DCHashTable");
    }

    ht->container = (DCDynArr*)calloc(capacity, sizeof(DCDynArr));

    if (ht->container == NULL)
    {
        dc_dbg_log("Memory allocation failed");

        dc_ret_e(2, "Memory allocation failed");
    }

    ht->cap = capacity;
    ht->key_count = 0;

    ht->hash_fn = hash_fn;
    ht->key_cmp_fn = key_cmp_fn;
    ht->pair_free_fn = pair_free_fn;

    dc_ret();
}

DCResHt dc_ht_new(usize capacity, DCHashFn hash_fn, DCKeyCompFn key_cmp_fn, DCHtPairFreeFn pair_free_fn)
{
    DC_RES_ht();

    DCHashTable* ht = (DCHashTable*)malloc(sizeof(DCHashTable));

    if (ht == NULL)
    {
        dc_dbg_log("Memory allocation failed");

        dc_ret_e(2, "Memory allocation failed");
    }

    dc_try_fail_temp(DCResVoid, dc_ht_init(ht, capacity, hash_fn, key_cmp_fn, pair_free_fn));

    dc_ret_ok(ht);
}

DCResVoid dc_ht_free(DCHashTable* ht)
{
    DC_RES_void();

    if (!ht)
    {
        dc_dbg_log("got NULL DCHashTable");

        dc_ret_e(1, "got NULL DCHashTable");
    }

    if (ht && ht->cap == 0) dc_ret();

    for (usize i = 0; i < ht->cap; ++i)
    {
        DC_HT_GET_AND_DEF_CONTAINER_ROW(darr, *ht, i);

        dc_da_for(*darr, {
            if (ht->pair_free_fn) dc_try_fail(ht->pair_free_fn((DCPair*)dc_dv_as(*_it, voidptr)));

            if (dc_dv_is_allocated(*_it) && dc_dv_as(*_it, voidptr) != NULL) free(dc_dv_as(*_it, voidptr));
        });

        free(darr->elements);
        darr->elements = NULL;
        darr->cap = 0;
        darr->count = 0;
    }

    free(ht->container);

    ht->cap = 0;
    ht->key_count = 0;
    ht->hash_fn = NULL;
    ht->key_cmp_fn = NULL;
    ht->pair_free_fn = NULL;

    dc_ret();
}

DCResVoid __dc_ht_free(voidptr ht)
{
    DC_RES_void();

    if (!ht)
    {
        dc_dbg_log("got NULL DCHashTable");

        dc_ret_e(1, "got NULL DCHashTable");
    }

    dc_try_fail(dc_ht_free((DCHashTable*)ht));

    dc_ret();
}

DCResUsize dc_ht_find_by_key(DCHashTable* ht, DCDynVal key, DCDynVal** out_result)
{
    DC_RES_usize();

    if (!ht)
    {
        dc_dbg_log("got NULL DCHashTable");

        dc_ret_e(1, "got NULL DCHashTable");
    }

    dc_try_fail_temp_ht_get_hash(_index, *ht, &key);

    DC_HT_GET_AND_DEF_CONTAINER_ROW(darr, *ht, _index);

    dc_da_for(*darr, {
        if (_it->type != dc_dvt(voidptr))
        {
            dc_dbg_log("wrong type, voidptr needed");

            dc_ret_e(3, "wrong type, voidptr needed");
        }

        DCDynVal element_key = ((DCPair*)_it->value.voidptr_val)->first;

        DCResBool cmp_res = ht->key_cmp_fn(&element_key, &key);
        dc_fail_if_err2(cmp_res);

        if (dc_unwrap2(cmp_res))
        {
            *out_result = &((DCPair*)_it->value.voidptr_val)->second;
            dc_ret_ok(_idx);
        }
    });

    *out_result = NULL;
    dc_ret_ok(0);
}

DCResVoid dc_ht_set(DCHashTable* ht, DCDynVal key, DCDynVal value, DCHashTableSetStatus set_status)
{
    DC_RES_void();

    if (!ht)
    {
        dc_dbg_log("got NULL DCHashTable");

        dc_ret_e(1, "got NULL DCHashTable");
    }

    dc_try_fail_temp_ht_get_hash(_index, *ht, &key);

    DCPair* new_pair = (DCPair*)malloc(sizeof(DCPair));
    if (new_pair == NULL)
    {
        dc_dbg_log("Memory allocation failed");

        dc_ret_e(2, "Memory allocation failed");
    }

    new_pair->first = key;
    new_pair->second = value;

    DC_HT_GET_AND_DEF_CONTAINER_ROW(current_row, *ht, _index);

    // The row has not been initialized before
    // So the key definitely does not exist
    if (current_row->cap == 0)
    {
        // We can only create the key when the set status is one of
        //  - DC_HT_SET_CREATE_OR_UPDATE
        //  - DC_HT_SET_CREATE_OR_NOTHING
        //  - DC_HT_SET_CREATE_OR_FAIL
        if (set_status == DC_HT_SET_CREATE_OR_UPDATE || set_status == DC_HT_SET_CREATE_OR_NOTHING ||
            set_status == DC_HT_SET_CREATE_OR_FAIL)
        {
            dc_try_fail(dc_da_init(current_row, NULL));
            dc_try_fail(dc_da_push(current_row, dc_dva(voidptr, new_pair)));

            ht->key_count++;

            dc_ret();
        }

        // Otherwise it's one of the following:
        //  - DC_HT_SET_UPDATE_OR_FAIL
        //  - DC_HT_SET_UPDATE_OR_NOTHING
        // That indicates user assumes key must already exist
        // First we need to free the allocated pair
        free(new_pair);

        // And if it's DC_HT_SET_UPDATE_OR_FAIL we need to return an error
        if (set_status == DC_HT_SET_UPDATE_OR_FAIL)
            dc_ret_e(dc_e_code(HT_SET), "can only update existing hash table pair, provided key not found");

        // Otherwise just return ok
        dc_ret();
    }

    DCDynVal* existed = NULL;
    DCResUsize find_res = dc_ht_find_by_key(ht, key, &existed);
    dc_fail_if_err2(find_res);

    usize existed_index = dc_unwrap2(find_res);

    // key does exists in the row
    if (existed != NULL)
    {
        // We can only update the key when the set status is one of
        //  - DC_HT_SET_CREATE_OR_UPDATE
        //  - DC_HT_SET_UPDATE_OR_NOTHING
        //  - DC_HT_SET_UPDATE_OR_FAIL
        if (set_status == DC_HT_SET_CREATE_OR_UPDATE || set_status == DC_HT_SET_UPDATE_OR_NOTHING ||
            set_status == DC_HT_SET_UPDATE_OR_FAIL)
        {
            DCPair* old_pair = (DCPair*)dc_dv_as(current_row->elements[existed_index], voidptr);

            if (ht->pair_free_fn) dc_try_fail(ht->pair_free_fn(old_pair));

            free(old_pair);

            current_row->elements[existed_index] = dc_dva(voidptr, new_pair);

            dc_ret();
        }

        // Otherwise it's one of the following:
        //  - DC_HT_SET_CREATE_OR_FAIL
        //  - DC_HT_SET_CREATE_OR_NOTHING
        // That indicates user assumes key must not exist beforehand
        // First we need to free the allocated pair
        free(new_pair);

        // And if it's DC_HT_SET_CREATE_OR_FAIL we need to return an error
        if (set_status == DC_HT_SET_CREATE_OR_FAIL)
            dc_ret_e(dc_e_code(HT_SET), "can only create hash table pair, provided key already exists");

        // Otherwise just return ok
        dc_ret();
    }

    // And at last key does not exists in the row
    // We can only create the key when the set status is one of
    //  - DC_HT_SET_CREATE_OR_UPDATE
    //  - DC_HT_SET_CREATE_OR_NOTHING
    //  - DC_HT_SET_CREATE_OR_FAIL
    if (set_status == DC_HT_SET_CREATE_OR_UPDATE || set_status == DC_HT_SET_CREATE_OR_NOTHING ||
        set_status == DC_HT_SET_CREATE_OR_FAIL)
    {
        dc_try_fail(dc_da_push(current_row, dc_dva(voidptr, new_pair)));
        ht->key_count++;

        dc_ret();
    }

    /// Otherwise it's one of the following:
    //  - DC_HT_SET_UPDATE_OR_FAIL
    //  - DC_HT_SET_UPDATE_OR_NOTHING
    // That indicates user assumes key must exists
    // First we need to free the allocated pair
    free(new_pair);

    // And if it's DC_HT_SET_UPDATE_OR_FAIL we need to return an error
    if (set_status == DC_HT_SET_UPDATE_OR_FAIL)
        dc_ret_e(dc_e_code(HT_SET), "can only update existing hash table pair, provided key not found");

    // Otherwise just return ok
    dc_ret();
}

DCResVoid __dc_ht_set_multiple(DCHashTable* ht, usize count, DCPair entries[], DCHashTableSetStatus set_status)
{
    DC_RES_void();

    if (!ht)
    {
        dc_dbg_log("got NULL DCHashTable");

        dc_ret_e(1, "got NULL DCHashTable");
    }

    for (usize i = 0; i < count; ++i)
    {
        dc_try_fail(dc_ht_set(ht, entries[i].first, entries[i].second, set_status));
    }

    dc_ret();
}

DCResVoid dc_ht_merge(DCHashTable* ht, DCHashTable* from, DCHashTableSetStatus set_status)
{
    DC_RES_void();

    if (!ht)
    {
        dc_dbg_log("got NULL DCHashTable");

        dc_ret_e(1, "got NULL DCHashTable");
    }

    for (usize i = 0; i < from->cap; ++i)
    {
        if (from->container[i].cap == 0) continue;

        for (usize j = 0; j < from->container[i].count; ++j)
        {
            DCPair* pair = dc_da_get_as(from->container[i], j, voidptr);

            dc_try_fail(dc_ht_set(ht, pair->first, pair->second, set_status));
        }
    }

    dc_ret();
}

DCResBool dc_ht_delete(DCHashTable* ht, DCDynVal key)
{
    DC_RES_bool();

    dc_try_fail_temp_ht_get_hash(_index, *ht, &key);

    DC_HT_GET_AND_DEF_CONTAINER_ROW(current_row, *ht, _index);

    if (current_row->count == 0) dc_ret_ok(false);

    DCDynVal* existed = NULL;
    DCResUsize possible_bucket = dc_ht_find_by_key(ht, key, &existed);
    dc_fail_if_err2(possible_bucket);

    usize existed_index = dc_unwrap2(possible_bucket);

    if (existed == NULL) dc_ret_ok(false);

    DCPair* old_pair = (DCPair*)dc_dv_as(current_row->elements[existed_index], voidptr);

    if (ht->pair_free_fn) dc_try_fail_temp(DCResVoid, ht->pair_free_fn(old_pair));

    dc_try_fail_temp(DCResVoid, dc_da_delete(current_row, existed_index));
    ht->key_count--;

    dc_ret_ok(true);
}

DCResUsize dc_ht_keys(DCHashTable* ht, DCDynVal** out_arr)
{
    DC_RES_usize();

    if (!ht || ht->key_count == 0)
    {
        dc_dbg_log("got NULL or Empty DCHashTable");

        dc_ret_e(1, "got NULL or Empty DCHashTable");
    }

    if (!out_arr) dc_ret_e(1, "got NULL pout_arr");

    *out_arr = (DCDynVal*)malloc((ht->key_count + 1) * sizeof(DCDynVal));
    if (!(*out_arr))
    {
        dc_dbg_log("Memory allocation failed");

        dc_ret_e(2, "Memory allocation failed");
    }

    usize key_count = 0;
    for (usize i = 0; i < ht->cap; ++i)
    {
        DC_HT_GET_AND_DEF_CONTAINER_ROW(darr, *ht, i);

        if (darr->cap == 0) continue;

        dc_da_for(*darr, {
            if (_it->type != DC_DYN_VAL_TYPE_voidptr)
            {
                free(*out_arr);
                *out_arr = NULL;
                dc_dbg_log("Bad type, DCHashTable elements must be of type voidptr");

                dc_ret_e(3, "Bad type, DCHashTable elements must be of type voidptr");
            }

            DCDynVal element_key = ((DCPair*)_it->value.voidptr_val)->first;

            (*out_arr)[key_count] = element_key;
            key_count++;
        });
    }

    (*out_arr)[ht->key_count] = dc_dv_nullptr();
    dc_ret_ok(ht->key_count);
}
