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

void dc_ht_init(DCHashTable* ht, usize capacity, DCHashFunc hash_func,
                DCKeyCompFunc key_cmp_func, DCDynValFreeFunc element_free_func)
{
    ht->container = (DCDynArr*)calloc(capacity, sizeof(DCDynArr));

    ht->cap = capacity;
    ht->key_count = 0;

    ht->hash_func = hash_func;
    ht->key_cmp_func = key_cmp_func;
    ht->element_free_func = element_free_func;
}

DCHashTable* dc_ht_create(usize capacity, DCHashFunc hash_func,
                          DCKeyCompFunc key_cmp_func,
                          DCDynValFreeFunc element_free_func)
{
    DCHashTable* ht = (DCHashTable*)malloc(sizeof(DCHashTable));

    dc_ht_init(ht, capacity, hash_func, key_cmp_func, element_free_func);

    return ht;
}

void dc_ht_free(DCHashTable* ht)
{
    dc_dbg_log("dc_ht_free, capacity: %zu", ht->cap);

    if (ht && ht->cap == 0) return;

    for (usize i = 0; i < ht->cap; ++i)
    {
        dc_ht_get_container_row(darr, *ht, i);

        dc_da_free(darr);
    }

    free(ht->container);

    ht->cap = 0;
    ht->key_count = 0;
    ht->hash_func = NULL;
    ht->key_cmp_func = NULL;
    ht->element_free_func = NULL;
}

void dc_ht_free__(voidptr ht)
{
    dc_dbg_log("dc_ht_free__ for address: %p", ht);
    dc_action_on(ht == NULL, return, "got null pointer");

    dc_ht_free((DCHashTable*)ht);
}

usize dc_ht_find_by_key(DCHashTable* ht, voidptr key, DCDynValue** out_result)
{
    dc_ht_get_hash(_index, *ht, key);

    dc_ht_get_container_row(darr, *ht, _index);

    dc_da_for(*darr)
    {
        DCDynValue* element = &darr->elements[_idx];
        if (element->type != dc_dvt(voidptr))
        {
            dc_log("Wrong type other than voidptr");
            exit(EXIT_FAILURE);
        }

        voidptr element_key = ((DCHashEntry*)element->value.voidptr_val)->key;
        if (ht->key_cmp_func(element_key, key))
        {
            *out_result = &((DCHashEntry*)element->value.voidptr_val)->value;
            return _idx;
        }
    }

    *out_result = NULL;
    return 0;
}

void dc_ht_set(DCHashTable* ht, voidptr key, DCDynValue value)
{
    dc_ht_get_hash(_index, *ht, key);

    DCHashEntry* new_entry = (DCHashEntry*)malloc(sizeof(DCHashEntry));
    if (new_entry == NULL)
    {
        dc_log("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    new_entry->key = key;
    new_entry->value = value;

    dc_ht_get_container_row(current, *ht, _index);

    if (current->cap == 0)
    {
        dc_da_init(current, ht->element_free_func);
        dc_da_push(current, dc_dv(voidptr, new_entry));
        ht->key_count++;

        return;
    }

    DCDynValue* existed = NULL;
    usize existed_index = dc_ht_find_by_key(ht, key, &existed);

    if (existed != NULL)
    {
        dc_dv_free(&current->elements[existed_index], ht->element_free_func);
        current->elements[existed_index] = dc_dv(voidptr, new_entry);

        return;
    }

    dc_da_push(current, dc_dv(voidptr, new_entry));
    ht->key_count++;
}

void ___dc_ht_set_multiple(DCHashTable* ht, usize count, DCHashEntry entries[])
{
    for (usize i = 0; i < count; ++i)
    {
        dc_ht_set(ht, entries[i].key, entries[i].value);
    }
}

void dc_ht_merge(DCHashTable* ht, DCHashTable* from)
{
    for (usize i = 0; i < from->cap; ++i)
    {
        if (from->container[i].cap == 0) continue;

        for (usize j = 0; j < from->container[i].count; ++j)
        {
            DCHashEntry* entry = dc_da_get_as(&from->container[i], j, voidptr);

            dc_ht_set(ht, entry->key, entry->value);
        }
    }
}

bool dc_ht_delete(DCHashTable* ht, voidptr key)
{
    dc_ht_get_hash(_index, *ht, key);

    dc_ht_get_container_row(current, *ht, _index);

    if (current->count == 0) return false;

    DCDynValue* existed = NULL;
    usize existed_index = dc_ht_find_by_key(ht, key, &existed);

    if (existed == NULL) return false;

    dc_da_delete(current, existed_index);
    ht->key_count--;

    return true;
}

usize dc_ht_keys(DCHashTable* ht, voidptr** out_arr)
{
    if (!ht || ht->key_count == 0 || !out_arr)
    {
        return 0;
    }

    *out_arr = (voidptr*)malloc((ht->key_count + 1) * sizeof(voidptr));
    if (!(*out_arr))
    {
        return 0;
    }

    usize key_count = 0;
    for (usize i = 0; i < ht->cap; ++i)
    {
        dc_ht_get_container_row(darr, *ht, i);

        if (darr->cap == 0) continue;

        dc_da_for(*darr)
        {
            DCDynValue* elem = &darr->elements[_idx];
            if (elem->type != DC_DYN_VAL_TYPE_voidptr)
            {
                free(*out_arr);
                *out_arr = ((void*)0);
                return 0;
            }

            voidptr element_key = ((DCHashEntry*)elem->value.voidptr_val)->key;

            (*out_arr)[key_count] = element_key;
            key_count++;
        }
    }

    (*out_arr)[ht->key_count] = (((void*)0));
    return ht->key_count;
}
