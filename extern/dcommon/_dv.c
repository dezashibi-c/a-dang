// ***************************************************************************************
//    Project: dcommon -> https://github.com/dezashibi-c/dcommon
//    File: _dv.c
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
// *  Description: private implementation file for definition of dynamic value
// *               functionalities
// *               DO NOT LINK TO THIS DIRECTLY
// ***************************************************************************************

#ifndef __DC_BYPASS_PRIVATE_PROTECTION
#error "You cannot link to this source (_dv.c) directly, please consider including dcommon.h"
#endif

#include "_headers/aliases.h"
#include "_headers/general.h"
#include "_headers/macros.h"

string dc_dv_fmt(DCDynVal* dv)
{
    if (!dv) return "";

#define dv_fmt_case(TYPE)                                                                                                      \
    case dc_dvt(TYPE):                                                                                                         \
        return dc_fmt(TYPE)

    switch (dv->type)
    {
        dv_fmt_case(b1);

        dv_fmt_case(u8);
        dv_fmt_case(u16);
        dv_fmt_case(u32);
        dv_fmt_case(u64);
        dv_fmt_case(i8);
        dv_fmt_case(i16);
        dv_fmt_case(i32);
        dv_fmt_case(i64);
        dv_fmt_case(f32);
        dv_fmt_case(f64);
        dv_fmt_case(uptr);
        dv_fmt_case(char);
        dv_fmt_case(string);
        dv_fmt_case(voidptr);
        dv_fmt_case(fileptr);
        dv_fmt_case(size);
        dv_fmt_case(usize);

        dv_fmt_case(DCStringView);

        dv_fmt_case(DCDynArrPtr);
        dv_fmt_case(DCHashTablePtr);
        dv_fmt_case(DCPairPtr);
        dv_fmt_case(DCDynValPtr);

        default:
            return "";
    };
#undef dv_fmt_case
}

string dc_tostr_dvt(DCDynVal* dv)
{
    if (!dv) return "(null dynamic value)";

#define dvt_case(TYPE)                                                                                                         \
    case dc_dvt(TYPE):                                                                                                         \
        return #TYPE

    switch (dv->type)
    {
        dvt_case(b1);

        dvt_case(u8);
        dvt_case(u16);
        dvt_case(u32);
        dvt_case(u64);
        dvt_case(i8);
        dvt_case(i16);
        dvt_case(i32);
        dvt_case(i64);
        dvt_case(f32);
        dvt_case(f64);
        dvt_case(uptr);
        dvt_case(char);
        dvt_case(string);
        dvt_case(voidptr);
        dvt_case(fileptr);
        dvt_case(size);
        dvt_case(usize);

        dvt_case(DCStringView);

        dvt_case(DCDynArrPtr);
        dvt_case(DCHashTablePtr);
        dvt_case(DCPairPtr);

        dvt_case(DCDynValPtr);

        default:
            return "unknown or unimplemented";
    };

#undef dvt_case
}

DCResString dc_tostr_dv(DCDynVal* dv)
{
    DC_RES_string();

    if (!dv) dc_ret_e(1, "got NULL dynamic value");

#define stringify(TYPE)                                                                                                        \
    case dc_dvt(TYPE):                                                                                                         \
        dc_sprintf(&result, dc_dv_fmt(dv), dc_dv_as(*dv, TYPE));                                                               \
        break

    string result = NULL;

    switch (dv->type)
    {
        stringify(u8);
        stringify(u16);
        stringify(u32);
        stringify(u64);
        stringify(i8);
        stringify(i16);
        stringify(i32);
        stringify(i64);
        stringify(f32);
        stringify(f64);
        stringify(uptr);
        stringify(char);
        stringify(string);
        stringify(voidptr);
        stringify(fileptr);
        stringify(size);
        stringify(usize);

        case dc_dvt(DCStringView):
            dc_sprintf(&result, DCPRIsv, dc_sv_fmt(dc_dv_as(*dv, DCStringView)));
            break;

        case dc_dvt(DCHashTablePtr):
        {
            dc_sprintf(&result, "%s", "{");
            if (!dc_dv_as(*dv, DCHashTablePtr))
            {
                dc_sappend(&result, "%s", "}");
                break;
            }

            DCHashTablePtr _ht = dc_dv_as(*dv, DCHashTablePtr);
            usize key_no = 0;
            for (usize i = 0; i < _ht->cap; ++i)
            {
                DC_HT_GET_AND_DEF_CONTAINER_ROW(darr, *_ht, i);

                dc_da_for(ht_pair_print, *darr, {
                    dc_try_or_fail_with3(DCResString, pair, dc_tostr_dv(_it), {});
                    dc_sappend(&result, "%s", dc_unwrap2(pair));

                    free(dc_unwrap2(pair));

                    if (key_no < _ht->key_count - 1) dc_sappend(&result, "%s", ", ");
                    ++key_no;
                });
            }

            dc_sappend(&result, "%s", "}");

            break;
        }

        case dc_dvt(DCDynArrPtr):
        {
            dc_sprintf(&result, "%s", "[");
            if (!dc_dv_as(*dv, DCDynArrPtr))
            {
                dc_sappend(&result, "%s", "]");
                break;
            }

            DCDynArrPtr _darr = dc_dv_as(*dv, DCDynArrPtr);
            dc_da_for(da_print_elements, *_darr, {
                dc_try_or_fail_with3(DCResString, item, dc_tostr_dv(_it), {});
                dc_sappend(&result, "%s", dc_unwrap2(item));

                free(dc_unwrap2(item));

                if (_idx < _darr->count - 1) dc_sappend(&result, "%s", ", ");
            });

            dc_sappend(&result, "%s", "]");

            break;
        }

        case dc_dvt(DCPairPtr):
        {
            DCPairPtr _pair = dc_dv_as(*dv, DCPairPtr);
            dc_try_or_fail_with3(DCResString, first, dc_tostr_dv(&_pair->first), {});
            dc_try_or_fail_with3(DCResString, second, dc_tostr_dv(&_pair->second), { free(dc_unwrap2(first)); });

            dc_sappend(&result, "(%s, %s)", dc_unwrap2(first), dc_unwrap2(second));

            free(dc_unwrap2(first));
            free(dc_unwrap2(second));

            break;
        }

        case dc_dvt(b1):
            dc_sappend(&result, "%s", dc_tostr_bool(dc_dv_as(*dv, b1)));
            break;

        case dc_dvt(DCDynValPtr):
            return dc_tostr_dv(dc_dv_as(*dv, DCDynValPtr));
            break;

        default:
            dc_sprintf(&result, "%s", "(unknown dynamic value)");
            break;
    };

    dc_ret_ok(result);

#undef stringify
}

DCResVoid dc_dv_print(DCDynVal* dv)
{
    DC_RES_void();

    dc_try_or_fail_with3(DCResString, res, dc_tostr_dv(dv), {});

    printf("%s", dc_unwrap2(res));

    if (dc_unwrap2(res)) free(dc_unwrap2(res));

    dc_ret();
}

DCResVoid dc_dv_println(DCDynVal* dv)
{
    DC_TRY_DEF2(DCResVoid, dc_dv_print(dv));

    printf("%s", "\n");

    dc_ret();
}


DCResBool dc_dv_to_bool(DCDynVal* dv)
{
    DC_RES_bool();

    // NULL DCDynVal is going to turn into false value
    if (!dv) dc_ret_ok(false);

#define type_to_bool(TYPE)                                                                                                     \
    case dc_dvt(TYPE):                                                                                                         \
        dc_ret_ok(dc_to_bool(TYPE, dv->value.dc_dvf(TYPE)));

    switch (dv->type)
    {
        type_to_bool(b1);

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

        type_to_bool(DCStringView);

        type_to_bool(DCDynArrPtr);
        type_to_bool(DCHashTablePtr);
        type_to_bool(DCPairPtr);

        type_to_bool(DCDynValPtr);

        default:
            break;
    };

    dc_dbg_log("Exiting Function on an unknown type");
    dc_ret_e(3, "unknown dynamic value type");
#undef type_to_bool
}

DCResBool dc_dv_eq(DCDynVal* dv1, DCDynVal* dv2)
{
    DC_RES_bool();

    if (!dv1 || !dv2)
    {
        dc_dbg_log("cannot compare DCDynVal with NULL");

        dc_ret_e(1, "cannot compare DCDynVal with NULL");
    }

    DCDynVal* lval = (dv1->type == dc_dvt(DCDynValPtr)) ? dc_dv_as(*dv1, DCDynValPtr) : dv1;
    DCDynVal* rval = (dv2->type == dc_dvt(DCDynValPtr)) ? dc_dv_as(*dv2, DCDynValPtr) : dv2;

    if (lval->type != rval->type) dc_ret_ok(false);

#define check_eq(TYPE)                                                                                                         \
    case dc_dvt(TYPE):                                                                                                         \
        if (lval->value.dc_dvf(TYPE) == rval->value.dc_dvf(TYPE)) dc_ret_ok(true);                                             \
        break

    switch (lval->type)
    {
        check_eq(b1);

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

        check_eq(voidptr);
        check_eq(fileptr);
        check_eq(size);
        check_eq(usize);

        case dc_dvt(string):
        {
            if (strcmp(dc_dv_as(*lval, string), dc_dv_as(*rval, string)) == 0) dc_ret_ok(true);
            break;
        }

        case dc_dvt(DCStringView):
        {
            if (dc_dv_as(*lval, DCStringView).str && dc_dv_as(*rval, DCStringView).str &&
                (dc_dv_as(*lval, DCStringView).str == dc_dv_as(*rval, DCStringView).str) &&
                (dc_dv_as(*lval, DCStringView).len == dc_dv_as(*rval, DCStringView).len))
                dc_ret_ok(true);

            break;
        }

        case dc_dvt(DCPairPtr):
        {
            DCPairPtr _pair1 = dc_dv_as(*lval, DCPairPtr);
            DCPairPtr _pair2 = dc_dv_as(*rval, DCPairPtr);

            if ((_pair1 && !_pair2) || (!_pair1 && _pair2)) dc_ret_ok(false);

            dc_try_or_fail_with3(DCResBool, key, dc_dv_eq(&_pair1->first, &_pair2->first), {});
            dc_try_or_fail_with3(DCResBool, value, dc_dv_eq(&_pair1->second, &_pair2->second), {});

            dc_ret_ok(dc_unwrap2(key) && dc_unwrap2(value));

            break;
        }

        case dc_dvt(DCDynValPtr):
            return dc_dv_eq(lval, rval);

        default:
            break;
    }

    dc_ret_ok(false);

#undef check_eq
}

DCResBool dc_dv_eq2(DCDynVal* dv1, DCDynVal dv2)
{
    return dc_dv_eq(dv1, &dv2);
}

DCResBool dc_dv_eq3(DCDynVal dv1, DCDynVal dv2)
{
    return dc_dv_eq(&dv1, &dv2);
}

DCResVoid dc_dv_free(DCDynVal* element, DCDynValFreeFn custom_free_fn)
{
    DC_RES_void();

    if (!element) dc_ret();

    switch (element->type)
    {
        case dc_dvt(string):
        {
            if (custom_free_fn) dc_try_fail(custom_free_fn(element));

            if (dc_dv_is_allocated(*element) && dc_dv_as(*element, string) != NULL) free(dc_dv_as(*element, string));

            dc_dv_set(*element, string, "");
            break;
        }

        case dc_dvt(fileptr):
        {
            if (custom_free_fn) dc_try_fail(custom_free_fn(element));

            if (dc_dv_is_allocated(*element) && dc_dv_as(*element, fileptr) != NULL) fclose(dc_dv_as(*element, fileptr));

            dc_dv_set(*element, fileptr, NULL);
            break;
        }

        case dc_dvt(voidptr):
        {
            if (custom_free_fn) dc_try_fail(custom_free_fn(element));

            if (dc_dv_is_allocated(*element) && dc_dv_as(*element, voidptr) != NULL) free(dc_dv_as(*element, voidptr));

            dc_dv_set(*element, voidptr, NULL);
            break;
        }

        case dc_dvt(DCPairPtr):
        {
            if (custom_free_fn) dc_try_fail(custom_free_fn(element));

            if (dc_dv_is_allocated(*element) && dc_dv_as(*element, DCPairPtr) != NULL)
            {
                DCPairPtr _pair = dc_dv_as(*element, DCPairPtr);

                dc_try_fail(dc_dv_free(&_pair->first, custom_free_fn));
                dc_try_fail(dc_dv_free(&_pair->second, custom_free_fn));

                free(_pair);
            }

            dc_dv_set(*element, DCPairPtr, NULL);
            break;
        }

        case dc_dvt(DCDynArrPtr):
            if (custom_free_fn) dc_try_fail(custom_free_fn(element));

            if (dc_dv_is_allocated(*element) && dc_dv_as(*element, DCDynArrPtr) != NULL)
            {
                dc_try_fail(dc_da_free(dc_dv_as(*element, DCDynArrPtr)));

                free(dc_dv_as(*element, DCDynArrPtr));
            }

            dc_dv_set(*element, DCDynArrPtr, NULL);

            break;

        case dc_dvt(DCHashTablePtr):
            if (custom_free_fn) dc_try_fail(custom_free_fn(element));

            if (dc_dv_is_allocated(*element) && dc_dv_as(*element, DCHashTablePtr) != NULL)
            {
                dc_try_fail(dc_ht_free(dc_dv_as(*element, DCHashTablePtr)));

                free(dc_dv_as(*element, DCHashTablePtr));
            }

            dc_dv_set(*element, DCHashTablePtr, NULL);

            break;

        case dc_dvt(DCStringView):
        {
            if (custom_free_fn) dc_try_fail(custom_free_fn(element));

            dc_try_fail(dc_sv_free(&dc_dv_as(*element, DCStringView)));

            break;
        }

        case dc_dvt(DCDynValPtr):
            if (custom_free_fn) dc_try_fail(custom_free_fn(element));

            // This is tricky, a DCDynVal is not a type to be created using malloc
            // so being allocated here simply means that this pointer is responsible to free out the
            // guest dynamic value not that it itself is an allocated one
            if (dc_dv_is_allocated(*element)) dc_try_fail(dc_dv_free(dc_dv_as(*element, DCDynValPtr), custom_free_fn));

            break;

        // Do nothing for literal types (integer, float, etc.)
        default:
            // This is important due to being able to manage custom types
            // They must be marked as allocated, managing there memory de-allocation should be
            // Handled in the custom_free_fn including freeing the field itself if it's a pointer type
            if (dc_dv_is_allocated(*element) && custom_free_fn) dc_try_fail(custom_free_fn(element));

            dc_dbg_log("Doesn't free anything - not allocated type");
            break;
    }

    dc_ret();
}

DCResVoid __dc_dv_free(voidptr dv)
{
    DC_RES_void();

    if (!dv)
    {
        dc_dbg_log("got NULL voidptr");

        dc_ret_e(1, "got NULL voidptr");
    }

    return dc_dv_free((DCDynVal*)dv, NULL);
}
