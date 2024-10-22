# How to use the help?

- Macros doesn't have `;` at the end so you can find out which ones are macros and which ones are functions.
- Those in ALL CAPS means that they are constants or if they look like functions means that what they do must not be used in a branch like if, for loop, etc.
- There are conventions in namings
  - `dc`_`thing`_`expectation`
  - Things:
    - `dv` -> Dynamic Value
    - `da` -> Dynamic Array
    - `ht` -> Hash Table
    - `res` -> result
    - ...

## Primitive Types

**👉 Checkout [example 1](/examples/test_arrays.c).**

```c
// Types defs

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef uintptr_t uptr;

typedef ptrdiff_t size;
typedef size_t usize;

typedef char* string;

typedef void* voidptr;
typedef FILE* fileptr;

/**
 * Returns boolean equivalent value (true/false or 1/0) for given type and
 * value.
 *
 * @example dc_as_bool(u8, 10); // true
 * @example dc_as_bool(voidptr, NULL); // false
 */
dc_as_bool(TYPE, VAL)

/**
 * Default value of boolean based on u8 dynamic value of 1
 */
#define dc_dv_true()

/**
 * Default value of boolean based on u8 dynamic value of 0
 */
#define dc_dv_false()

/**
 * Default value of boolean based on voidptr dynamic value of NULL
 */
#define dc_dv_nullptr()

/**
 * Provides proper stopper for given type
 *
 * NOTE: These are values that can be used as a stopping point in an array
 */
dc_stopper(TYPE)

/**
 * Checks if the given value is an stopper for the given type
 */
dc_is_stopper(TYPE, VALUE)

// ***************************************************************************************
// * NON-POINTER ARRAY MACROS
// ***************************************************************************************

/**
 * Creates a literal array of the given type that is terminated by the specific
 * stopper for that type
 */
dc_array(TYPE, ...)                                                 

/**
 * Defines a variable of the given `TYPE`[] and initialize it with given data
 * terminated with proper stopper
 */
DC_DEF_ARRAY(NAME, TYPE, ...)

/**
 * Expands to actual number of elements in a literal array
 *
 * NOTE: For strings and stopper terminated array this will shows +1
 */
dc_count(ARR)

/**
 * Expands to actual useful elements in a stopper terminated array
 */
dc_len(ARR)

/**
 * Expands to the last item in a stopper terminated array
 */
dc_last(ARR)

/**
 * Iterator in a stopper terminated array provided with pointer to the current
 * element in each iteration as `_it`
 */
dc_foreach(ARR, TYPE)                                                 

/**
 * Iterator that perform given `FN` on each element in a stopper terminated
 * array
 */
dc_oneach(ARR, TYPE, FN)

/**
 * Expands to an iterator over a literal array of given data and returns pointer
 * to current element in each iteration as `_it`
 *
 * @example dc_foreach_lit(u8, 1, 2, 3, 4) printf("%dn", *_it);
 */
dc_foreach_lit(TYPE, ...)                                             

/**
 * Iterator that performs given `FN` on each provided element
 */
dc_oneach_lit(TYPE, FN, ...)


// ***************************************************************************************
// * POINTER ARRAY MACROS
// ***************************************************************************************

/**
 * Creates a literal array of the given pointer type that is terminated by NULL
 *
 * @example dc_parr_lit(MyStruct*, &s1, &s2, &s3);
 *
 * NOTE: You need to include `*` in the type
 */
dc_parr_lit(TYPE, ...)                                                

/**
 * Defines a literal array of pointer to the given type
 *
 * @example DC_PARRAY(my_arr, MyStruct, &s1, &s2, &s3);
 *
 * NOTE: You don't need to include `*` in the type
 */
DC_PARRAY(NAME, TYPE, ...)

/**
 * Iterates over a literal array of pointers to given type and provide
 * pointer to the current element as `_it`
 *
 * NOTE: You don't need to include `*` in the type
 */
dc_pforeach(ARR, TYPE)                                                

/**
 * Iterates over the given pointer array of `TYPE` and pass each
 * element to `FN`
 */
dc_poneach(ARR, TYPE, FN)

/**
 * Iterates over the given pointers of `TYPE` and provide pointer to the current
 * element as `_it`
 */
dc_pforeach_lit(TYPE, ...)                                            

/**
 * Iterates over the given pointers of `TYPE` and pass them to `FN`
 *
 * @example dc_poneach_lit(MyStruct, print_my_struct, &m1, &m2, &m3);
 */
dc_poneach_lit(TYPE, FN, ...)                                         

// ***************************************************************************************
// * STRUCT ARRAY MACROS
// ***************************************************************************************

/**
 * Creates literal array for the given struct type
 *
 * NOTE: The array is not terminated with stoppers
 */
dc_sarr_lit(TYPE, ...)                                                

/**
 * Defines the given NAME as an array of given TYPE with given data
 *
 * NOTE: The array is note terminated with stoppers
 */
DC_SARRAY(NAME, TYPE, ...) 

/**
 * Iterates over an array of structs and stops when the TERMINATION_CONDITION is
 * met
 *
 * The pointer to the current item in each iteration is provided as `_it`
 *
 * NOTE: You can use `_it` in the condition as well
 */
dc_sforeach(ARR, TYPE, TERMINATION_CONDITION)                         

/**
 * Iterates over an array of structs and pass pointer to each element to `FN`
 *
 * Iteration will be stopped when the TERMINATION_CONDITION is met
 *
 * The pointer to the current item in each iteration is provided as `_it`
 *
 * NOTE: You can use `_it` in the condition as well
 */
dc_soneach(ARR, TYPE, TERMINATION_CONDITION, FN)                      

/**
 * Iterates over given struct items of same TYPE and stops when the
 * TERMINATION_CONDITION is met
 *
 * The pointer to the current item in each iteration is provided as `_it`
 *
 * NOTE: You can use `_it` in the condition as well
 */
dc_sforeach_lit(TYPE, TERMINATION_CONDITION, ...)                     

/**
 * Iterates over given struct items of same TYPE and pass pointer to each
 * element to `FN`
 *
 * Iteration will be stopped when the TERMINATION_CONDITION is met
 *
 * The pointer to the current item in each iteration is provided as `_it`
 *
 * NOTE: You can use `_it` in the condition as well
 */
dc_soneach_lit(TYPE, TERMINATION_CONDITION, FN, ...)                  

/**
 * Converts a string to number of type i8
 *
 * @return DCResI8 (If OK i8 value can be retrieved otherwise it contains
 * proper error)
 */
DCResI8 dc_str_to_i8(const string str);

/**
 * Converts a string to number of type i16
 *
 * @return DCResI16 (If OK i16 value can be retrieved otherwise it contains
 * proper error)
 */
DCResI16 dc_str_to_i16(const string str);

/**
 * Converts a string to number of type i32
 *
 * @return DCResI32 (If OK i32 value can be retrieved otherwise it contains
 * proper error)
 */
DCResI32 dc_str_to_i32(const string str);

/**
 * Converts a string to number of type i64
 *
 * @return DCResI64 (If OK i64 value can be retrieved otherwise it contains
 * proper error)
 */
DCResI64 dc_str_to_i64(const string str);

/**
 * Converts a string to number of type u8
 *
 * @return DCResU8 (If OK u8 value can be retrieved otherwise it contains
 * proper error)
 */
DCResU8 dc_str_to_u8(const string str);

/**
 * Converts a string to number of type u16
 *
 * @return DCResU16 (If OK u16 value can be retrieved otherwise it contains
 * proper error)
 */
DCResU16 dc_str_to_u16(const string str);

/**
 * Converts a string to number of type u32
 *
 * @return DCResU32 (If OK u32 value can be retrieved otherwise it contains
 * proper error)
 */
DCResU32 dc_str_to_u32(const string str);

/**
 * Converts a string to number of type u64
 *
 * @return DCResU64 (If OK u64 value can be retrieved otherwise it contains
 * proper error)
 */
DCResU64 dc_str_to_u64(const string str);

/**
 * Converts a string to number of type f32
 *
 * @return DCResF32 (If OK f32 value can be retrieved otherwise it contains
 * proper error)
 */
DCResF32 dc_str_to_f32(const string str);

/**
 * Converts a string to number of type f64
 *
 * @return DCResF64 (If OK f64 value can be retrieved otherwise it contains
 * proper error)
 */
DCResF64 dc_str_to_f64(const string str);
```

## Result Types

To be able to do proper error management and remove all the old way of `0` is fine, `-1` is not fine, etc. this has been made.

**👉 Checkout [example 1](/examples/test_result.c).**

**👉 Checkout [example 2](/examples/test_hash_table.c).**

```c
/**
 * Enum for two possible result status, ok and error
 */
typedef enum
{
    DC_RES_OK,
    DC_RES_ERR,
} DCResStatus;

/**
 * Holds error data when the result is an error
 *
 * Notes:
 *
 * 1. There are reserved codes (Not mandatory but `dcommon` follows that)
 * - '1' Null Value detected
 * - '2' Memory management failed, malloc, calloc, realloc, etc.
 * - '3' Unknown type or un-matched type detected
 * - '4' Index out of bound
 * - '5' Internal errors
 * - '6' Not Found
 *
 * 2. If message is allocated the `allocated` field must be set to true.
 */
typedef struct
{
    i8 code;
    string message;
    bool allocated;
} DCError;


/**
 * Macro to define new Result type
 *
 * NOTE: See (alias.h) for usage in definition of default result types
 */
DCResType(TYPE, NAME)                                              

/**
 * Minimum structure to be able to be cased and used as a Result is DCResVoid
 */
typedef struct
{
    DCResStatus status;
    union
    {
        DCError e;
    } data;
} DCResVoid;

// ***************************************************************************************
// * DEFAULT PRIMITIVE RESULT TYPE DECLARATIONS
// ***************************************************************************************
DCResType(i8, DCResI8);
DCResType(i16, DCResI16);
DCResType(i32, DCResI32);
DCResType(i64, DCResI64);
DCResType(u8, DCResU8);
DCResType(u16, DCResU16);
DCResType(u32, DCResU32);
DCResType(u64, DCResU64);
DCResType(f32, DCResF32);
DCResType(f64, DCResF64);
DCResType(uptr, DCResUptr);
DCResType(size, DCResSize);
DCResType(usize, DCResUsize);
DCResType(string, DCResString);
DCResType(voidptr, DCResVoidptr);
DCResType(fileptr, DCResFileptr);
DCResType(bool, DCResBool);

// ***************************************************************************************
// * DCOMMON CUSTOM TYPES RESULT TYPE DECLARATIONS
// ***************************************************************************************

DCResType(DCDynVal, DCRes);
DCResType(DCStringView, DCResSv);
DCResType(DCDynArr*, DCResDa);
DCResType(DCHashTable*, DCResHt);
DCResType(DCDynVal*, DCResDv);

/**
 * Defines the main result variable (__dc_res) as DCRes and initiates it as
 * DC_RES_OK
 */
DC_RES()                                                              

/**
 * Defines the main result variable (__dc_res) as the given result type and
 * initiates it as DC_RES_OK
 */
DC_RES2(DC_RESULT_TYPE)                                               

/**
 * Defines the main result variable (__dc_res) as DCResBool type and
 * initiates it as DC_RES_OK
 */
DC_RES_bool()

/**
 * Defines the main result variable (__dc_res) as DCResVoid type and
 * initiates it as DC_RES_OK
 */
DC_RES_void()

/**
 * Defines the main result variable (__dc_res) as DCResSv type and
 * initiates it as DC_RES_OK
 */
DC_RES_sv()

/**
 * Defines the main result variable (__dc_res) as DCResDa type and
 * initiates it as DC_RES_OK
 */
DC_RES_da()

/**
 * Defines the main result variable (__dc_res) as DCResHt type and
 * initiates it as DC_RES_OK
 */
DC_RES_ht()

/**
 * Defines the main result variable (__dc_res) as DCResDv type and
 * initiates it as DC_RES_OK
 */
DC_RES_dv()

/**
 * Defines the main result variable (__dc_res) as DCResI8 type and
 * initiates it as DC_RES_OK
 */
DC_RES_i8()

/**
 * Defines the main result variable (__dc_res) as DCResI16 type and
 * initiates it as DC_RES_OK
 */
DC_RES_i16()

/**
 * Defines the main result variable (__dc_res) as DCResI32 type and
 * initiates it as DC_RES_OK
 */
DC_RES_i32()

/**
 * Defines the main result variable (__dc_res) as DCResI64 type and
 * initiates it as DC_RES_OK
 */
DC_RES_i64()

/**
 * Defines the main result variable (__dc_res) as DCResU8 type and
 * initiates it as DC_RES_OK
 */
DC_RES_u8()

/**
 * Defines the main result variable (__dc_res) as DCResU16 type and
 * initiates it as DC_RES_OK
 */
DC_RES_u16()

/**
 * Defines the main result variable (__dc_res) as DCResU32 type and
 * initiates it as DC_RES_OK
 */
DC_RES_u32()

/**
 * Defines the main result variable (__dc_res) as DCResU64 type and
 * initiates it as DC_RES_OK
 */
DC_RES_u64()

/**
 * Defines the main result variable (__dc_res) as DCResF32 type and
 * initiates it as DC_RES_OK
 */
DC_RES_f32()

/**
 * Defines the main result variable (__dc_res) as DCResF64 type and
 * initiates it as DC_RES_OK
 */
DC_RES_f64()

/**
 * Defines the main result variable (__dc_res) as DCResUptr type and
 * initiates it as DC_RES_OK
 */
DC_RES_uptr()

/**
 * Defines the main result variable (__dc_res) as DCResSize type and
 * initiates it as DC_RES_OK
 */
DC_RES_size()

/**
 * Defines the main result variable (__dc_res) as DCResUsize type and
 * initiates it as DC_RES_OK
 */
DC_RES_usize()

/**
 * Defines the main result variable (__dc_res) as DCResString type and
 * initiates it as DC_RES_OK
 */
DC_RES_string()

/**
 * Defines the main result variable (__dc_res) as DCResVoidptr type and
 * initiates it as DC_RES_OK
 */
DC_RES_voidptr()

/**
 * Defines the main result variable (__dc_res) as DCResFileptr type and
 * initiates it as DC_RES_OK
 */
DC_RES_fileptr()

/**
 * Defines the main result variable (__dc_res) as DCResFileptr type and
 * initiates it as DC_RES_OK
 */
DC_RES_fileptr()

/**
 * Sets the main result variable (__dc_res) status to DC_RES_ERR and
 * initiates the error value with given error code (NUM) and literal string
 *
 * NOTE: MSG must be literal string not an allocated string variable (see
 * dc_ea)
 */
dc_e(NUM, MSG)                                                    

/**
 * Sets the main result variable (__dc_res) status to DC_RES_ERR and
 * initiates the error value with given error code (NUM), it also create
 * formatted string (allocated) and sets the error message
 *
 * NOTE: Allocates memory
 */
dc_ea(NUM, ...)                                                   

/**
 * Copies error data from RES2 to RES1
 * 
 * NOTE: Treat to this like a move semantic, so if you free the original or
 * the copy that would be fine
 *
 * NOTE: Does Nothing on successful results
 */
dc_err_cpy2(RES1, RES2)                                           

/**
 * Copies error data from RES to the main result variable (__dc_res)
 * 
 * NOTE: Treat to this like a move semantic, so if you free the original or
 * the copy that would be fine
 *
 * NOTE: Does Nothing on successful results
 */
dc_err_cpy(RES)


/**
 * Sets the main result variable (__dc_res) status to DC_RES_OK and also
 * initiates the value with the given VALUE
 */
dc_ok(VALUE)                                                      

/**
 * In case the result is of type DCRes this is a shortcut to create a literal
 * dynamic value and setting it to the success value on the fly
 *
 * NOTE: The value must not be allocated (see dc_ok_dva)
 */
dc_ok_dv(TYPE, VALUE) 

/**
 * In case the result is of type DCRes this is a shortcut to create a literal
 * dynamic value and setting it to the success value on the fly
 *
 * NOTE: The value must be allocated (see dc_ok_dv)
 */
dc_ok_dva(TYPE, VALUE)

/**
 * Returns the main result variable (__dc_res)
 */
dc_ret()

/**
 * Sets the main result variable (__dc_res) to error and returns it right away.
 *
 * NOTE: MSG must be literal string not an allocated string variable
 * (see dc_ret_ea)
 */
dc_ret_e(NUM, MSG)                                                

/**
 * Sets the main result variable (__dc_res) to error and returns it right away.
 *
 * NOTE: Allocates memory (see dc_ret_e)
 */
dc_ret_ea(NUM, ...)                                               

/**
 * Sets the main result variable (__dc_res) to success and returns it right
 * away.
 */
dc_ret_ok(VALUE)                                                  

/**
 * Sets the main result variable (__dc_res) to success and returns it right
 * away.
 *
 * NOTE: The main result variable must be of type DCRes
 *
 * NOTE: The VALUE must not be allocated (see dc_ret_ok_dva)
 */
dc_ret_ok_dv(TYPE, VALUE)                                         

/**
 * Sets the main result variable (__dc_res) to success and returns it right
 * away.
 *
 * NOTE: The main result variable must be of type DCRes
 *
 * NOTE: The VALUE must be allocated (see dc_ret_ok_dv)
 */
dc_ret_ok_dva(TYPE, VALUE)                                        

/**
 * Renders an if statements that checks if main result variable (__dc_res)
 * status is error and if so returns it
 */
dc_fail_if_err()                                                       

/**
 * Expands to a new result variable declaration initialized with the CALL
 * and returning error with failure actions in case of errors
 */
#define dc_try_or_fail_with3(RES_TYPE, RES, CALL, FAILURE_ACTIONS)   

/**
 * Expands to assigning an existing result variable with the CALL
 * and returning error with failure actions in case of errors
 */
#define dc_try_or_fail_with2(RES, CALL, FAILURE_ACTIONS)

/**
 * Expands to assigning main result variable (__dc_res) to the CALL
 * and returning error with failure actions in case of errors
 */
#define dc_try_or_fail_with(CALL, FAILURE_ACTIONS)

/**
 * Checks if the main result variable (__dc_res) is error does
 * PRE_RETURN_ACTIONS and then return __dc_res
 */
dc_ret_if_err(PRE_RETURN_ACTIONS)

/**
 * Assigns the main result variable (__dc_res) to the provided function call
 */
dc_try(CALL)

/**
 * First it assigns the main result variable (__dc_res) to the provided function
 * call then it checks if it has error and in that case will return __dc_res
 */
dc_try_fail(CALL)                                                     

/**
 * Checks provided result variable and if its status is error populates the main
 * result variable (__dc_res) error and returns it
 */
dc_fail_if_err2(RES)                                             

/**
 * Checks if the given result variable is error copies the error data and does
 * PRE_RETURN_ACTIONS and then return __dc_res
 */
dc_ret_if_err2(RES, PRE_RETURN_ACTIONS)

/**
 * In case you want to call a function that returns a result type and you don't
 * care it's return type (might be DCResVoid as an example) but you do care
 * if the result is error and you want to halt the process this macro is the
 * solution
 *
 * @brief Creates temporary result type assign it with the function call (CALL)
 * and return of the temporary result is an error
 */
dc_try_fail_temp(RES_TYPE, CALL)                                      

/**
 * There are places that at the beginning of a function we might need to define
 * and initiate the main result variable (__dc_res) and exactly call to a
 * function and fail if it is error or hold the success data in the main result
 * variable (__dc_res) in that situation this is the macro we want to use
 *
 * @brief First initialize the main result variable with DCRes then try
 * calling the CALL and fail if it has error
 */
DC_TRY_DEF(CALL)                                                      

/**
 * There are places that at the beginning of a function we might need to define
 * and initiate the main result variable (__dc_res) and exactly call to a
 * function and fail if it is error or hold the success data in the main result
 * variable (__dc_res) in that situation this is the macro we want to use
 *
 * @brief First initialize the main result variable with given result type then
 * try calling the CALL and fail if it has error
 */
DC_TRY_DEF2(DC_RESULT_TYPE, CALL)                                     

/**
 * Can be used to get specific type from main result variable (__dc_res) under
 * two condition:
 *
 * - The main result variable (__dc_res) is of type DCRes
 * - The result status is ok
 */
dc_as(TYPE)

/**
 * Returns true if main result variable (__dc_res) status is ok
 */
dc_is_ok()

/**
 * Returns true if the given result variable status is ok
 */
dc_is_ok2(RES) 

/**
 * Returns true if main result variable (__dc_res) status is error
 */
dc_is_err()

/**
 * Returns true if the given result variable status is error
 */
dc_is_err2(RES) 

/**
 * Retrieves the value of the main result variable (__dc_res)
 *
 * NOTE: You must have already checked to make sure the current status is ok
 */
dc_unwrap()

/**
 * Retrieves the value of the given result variable
 *
 * NOTE: You must have already checked to make sure the current status is ok
 */
dc_unwrap2(RES)

/**
 * Expands to current status of the main result variable (__dc_res)
 */
dc_status()

/**
 * Expands to the status of the given result variable
 */
dc_status2(RES) 

/**
 * Retrieves the error data of the main result variable (__dc_res)
 *
 * NOTE: You must have already checked to make sure the current status is error
 */
dc_err()

/**
 * Retrieves the error data of the given result variable
 *
 * NOTE: You must have already checked to make sure the current status is error
 */
dc_err2(RES)

/**
 * Retrieves the error code of the main result variable (__dc_res)
 *
 * NOTE: You must have already checked to make sure the current status is error
 */
dc_err_code() 

/**
 * Retrieves the error code of the given result variable
 *
 * NOTE: You must have already checked to make sure the current status is error
 */
dc_err_code2(RES) 

/**
 * Retrieves the error message of the main result variable (__dc_res)
 *
 * NOTE: You must have already checked to make sure the current status is error
 */
dc_err_msg()

/**
 * Retrieves the error message of the given result variable
 *
 * NOTE: You must have already checked to make sure the current status is error
 */
dc_err_msg2(RES)

/**
 * Debug logs the given result's error result with provided string literal
 * message prepended
 *
 * NOTE: The result must be error
 */
dc_err_dbg_log2(RES, PRE_MSG)                                      

/**
 * Debug logs main result variable (__dc_res) with provided string literal
 * message prepended
 *
 * NOTE: The result must be error
 */
dc_err_dbg_log(PRE_MSG) 

/**
 * Logs the given result's error result with provided string literal
 * message prepended
 *
 * NOTE: The result must be error
 */
dc_err_log2(RES, PRE_MSG)                                          

/**
 * Logs main result variable (__dc_res) with provided string literal
 * message prepended
 *
 * NOTE: The result must be error
 */
dc_err_log(PRE_MSG) 

/**
 * Frees the error message of result variables if they have any allocated error
 * message string
 *
 * @return nothing or error
 *
 * NOTE: Only proper result type based pointers must be sent otherwise undefined
 * behavior might happen
 *
 * NOTE: Checkout aliases.h if you want to create your own compatible type
 */
DCResVoid dc_result_free(voidptr res_ptr);

```

## Dynamic Values and Dynamic Arrays

**👉 Checkout [example](/examples/test_dyn_array.c).**

```c
/**
 * All the types that DCDynVal can accept
 */
typedef enum
{
    ...
} DCDynValType;

/**
 * Dynamic value type with ability to keep track of holding allocated string or
 * voidptr for further cleanup
 */
typedef struct
{
    DCDynValType type;
    bool allocated;
    union
    {
        i8 i8_val;
        i16 i16_val;
        i32 i32_val;
        i64 i64_val;

        u8 u8_val;
        u16 u16_val;
        u32 u32_val;
        u64 u64_val;

        f32 f32_val;
        f64 f64_val;

        uptr uptr_val;
        char char_val;
        string string_val;
        voidptr voidptr_val;
        fileptr fileptr_val;

        size size_val;
        usize usize_val;
    } value;
} DCDynVal;

/**
 * Custom function type for cleaning up a dynamic value
 *
 * As an example a Dynamic value might contain a voidptr to another struct that
 * holds allocated fields and they need to be de-allocated before the actual
 * voidptr value to be freed
 *
 * NOTE: Also the voidptr might be marked as allocated=false but still it's
 * corresponding value that it is pointing to might need to have a clean up
 * process (see dc_dv_free)
 */
typedef DCResVoid (*DCDynValFreeFn)(DCDynVal*);

/**
 * Dynamic array with ability to keep any number of dynamic values
 *
 * Initial capacity and multiplication on grow is also customizable
 *
 * Dynamic arrays or Darr for short can be grown, truncated, popped, etc.
 */
typedef struct
{
    DCDynVal* elements;
    usize cap;
    usize count;
    usize multiplier;

    DCDynValFreeFn element_free_fn;
} DCDynArr;

/**
 * Returns string representation of the type the given dynamic value holds right now
 *
 * @return the string representation of the type, "(null dynamic value)" and "unknown or unimplemented"
 */
string dc_tostr_dvt(DCDynVal* dv);

/**
 * Checks for falsy values in a given dynamic value and return the result as
 * DCResBool
 *
 * @return DCResBool (If OK bool value can be retrieved otherwise it contains
 * proper error)
 */
DCResBool dc_dv_as_bool(DCDynVal* dv);

/**
 * Initializes a given pointer to dynamic array
 *
 * @param element_free_fn can be NULL or a proper function name which handles
 * complex cleanup for dynamic values stored in the defined dynamic array
 *
 * @return nothing or error
 */
DCResVoid dc_da_init(DCDynArr* darr, DCDynValFreeFn element_free_fn);

/**
 * Initializes a given pointer to dynamic array with custom capacity and grow
 * multiplier amounts
 *
 * @param element_free_fn can be NULL or a proper function name which handles
 * complex cleanup for dynamic values stored in the defined dynamic array
 *
 * @return nothing or error
 */
DCResVoid dc_da_init2(DCDynArr* darr, usize capacity,
                         usize capacity_grow_multiplier,
                         DCDynValFreeFn element_free_fn);

/**
 * Creates, allocates, initializes and returns a pointer to dynamic array
 *
 * @param element_free_fn can be NULL or a proper function name which handles
 * complex cleanup for dynamic values stored in the defined dynamic array
 *
 * @return dynamic array pointer (DCDynArr*) or error
 *
 * NOTE: Allocates memory
 */
DCResDa dc_da_new(DCDynValFreeFn element_free_fn);

/**
 * Creates, initializes and returns a pointer to dynamic array with custom
 * capacity and grow multiplier amounts
 *
 * @param element_free_fn can be NULL or a proper function name which handles
 * complex cleanup for dynamic values stored in the defined dynamic array
 *
 * @return dynamic array pointer (DCDynArr*) or error
 *
 * NOTE: Allocates memory
 */
DCResDa dc_da_new2(usize capacity, usize capacity_grow_multiplier,
                      DCDynValFreeFn element_free_fn);

/**
 * Grows the capacity of given dynamic array pointer by current capacity *
 * registered multiplier
 *
 * NOTE: default multiplier is 2
 *
 * @return nothing or error
 */
DCResVoid dc_da_grow(DCDynArr* darr);

/**
 * Grows the capacity of given dynamic array pointer by current capacity +
 * provided amount
 *
 * @return nothing or error
 */
DCResVoid dc_da_grow_by(DCDynArr* darr, usize amount);

/**
 * Grows the capacity of given dynamic array pointer to provided amount
 *
 * @return nothing or error
 */
DCResVoid dc_da_grow_to(DCDynArr* darr, usize amount);

/**
 * Truncates unused capacity of given dynamic array pointer
 *
 * @return nothing or error
 */
DCResVoid dc_da_trunc(DCDynArr* darr);

/**
 * Pops out the elements of a given dynamic array by amount of the given count
 *
 * @param out_popped is a pointer to a dynamic value pointer
 * @param truncate indicates whether to truncate the original array after
 * popping out values or not
 *
 * @return nothing or error
 */
DCResVoid dc_da_pop(DCDynArr* darr, usize count, DCDynVal** out_popped,
                       bool truncate);

/**
 * Pushes given dynamic value to the given dynamic array
 *
 * @return nothing or error
 */
DCResVoid dc_da_push(DCDynArr* darr, DCDynVal value);

/**
 * Appends the elements from the `from` dynamic array to the given `darr`
 * dynamic array
 *
 * @return nothing or error
 */
DCResVoid dc_da_append(DCDynArr* darr, DCDynArr* from);

/**
 * Returns a pointer to an element of a dynamic array by its index
 *
 * @return a pointer to dynamic value or an error
 */
DCResDv dc_da_get(DCDynArr* darr, usize index);

/**
 * Checks whether two given dynamic values are equal or not
 *
 * @return bool or error
 */
DCResBool dc_dv_eq(DCDynVal* dv1, DCDynVal* dv2);

/**
 * Searches for given element (a pointer to a dynamic value) in an array
 *
 * @return index or error
 *
 * NOTE: error code 6 means not found, other error types might happen as well
 */
DCResUsize dc_da_findp(DCDynArr* darr, DCDynVal* el);

/**
 * Searches for given element (a literal dynamic value) in an array
 *
 * @return index or error
 *
 * NOTE: error code 6 means not found, other error types might happen as well
 */
DCResUsize dc_da_find(DCDynArr* darr, DCDynVal el);

/**
 * Frees allocated string or voidptr, does nothing for the rest of dynamic value
 * types
 *
 * NOTES:
 *
 * - For string types if they are allocated first sends them to `custom_free_fn`
 * if provided and then if the value is not `NULL` tries to `free` it and set it
 * to "".
 *
 * - For voidptr types it first send them to `custom_free_fn` then if it is
 * allocated frees it and set it to `NULL`.
 *
 * @return nothing or error
 */
DCResVoid dc_dv_free(DCDynVal* element, DCDynValFreeFn custom_free_fn);

/**
 * General function for cleanup process of a dynamic value
 *
 * NOTE: see cleanup related macros -> `dc_cleanup_push_dv`
 *
 * NOTE: If you need a custom cleanup function for your dynamic value that
 * handle some complex stuff you need to create it and use `dc_cleanup_push`
 * instead.
 *
 * @return nothing or error
 */
DCResVoid __dc_dv_free(voidptr dv);

/**
 * Frees all the dynamic values of the given dynamic array and then the array
 * itself and reset capacity back to zero
 *
 * @return nothing or error
 */
DCResVoid dc_da_free(DCDynArr* darr);

/**
 * Tries to delete an element by index in the given darr
 *
 * @return nothing or error
 */
DCResVoid dc_da_delete(DCDynArr* darr, usize index);

/**
 * Tries to delete an element by pointer in the given darr
 *
 * NOTE: it first tries to find the item then it will delete it, so an error
 * with code 6 (not found) can happen.
 *
 * @return nothing or error (including code 6 -> not found)
 */
DCResVoid dc_da_delete_elp(DCDynArr* darr, DCDynVal* el);

/**
 * Tries to delete an element in the given darr
 *
 * NOTE: it first tries to find the item then it will delete it, so an error
 * with code 6 (not found) can happen.
 *
 * @return nothing or error (including code 6 -> not found)
 */
DCResVoid dc_da_delete_el(DCDynArr* darr, DCDynVal el);

/**
 * Inserts given value at the given `start_index` (resize might happen)
 *
 * @return nothing or error
 */
DCResVoid dc_da_insert(DCDynArr* darr, usize index, DCDynVal value);

/**
 * Inserts items from a given `from` array to the original `darr` array at the
 * given `start_index`
 *
 * @return nothing or error
 */
DCResVoid dc_da_insert_from(DCDynArr* darr, usize start_index,
                               DCDynArr* from);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than i8, when false the unmatched types will be
 * ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_i8_da_to_flat_arr(DCDynArr* arr, i8** out_arr, bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than i16, when false the unmatched types will be
 * ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_i16_da_to_flat_arr(DCDynArr* arr, i16** out_arr,
                                    bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than i32, when false the unmatched types will be
 * ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_i32_da_to_flat_arr(DCDynArr* arr, i32** out_arr,
                                    bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than i64, when false the unmatched types will be
 * ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_i64_da_to_flat_arr(DCDynArr* arr, i64** out_arr,
                                    bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than u8, when false the unmatched types will be
 * ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_u8_da_to_flat_arr(DCDynArr* arr, u8** out_arr, bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than u16, when false the unmatched types will be
 * ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_u16_da_to_flat_arr(DCDynArr* arr, u16** out_arr,
                                    bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than u32, when false the unmatched types will be
 * ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_u32_da_to_flat_arr(DCDynArr* arr, u32** out_arr,
                                    bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than u64, when false the unmatched types will be
 * ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_u64_da_to_flat_arr(DCDynArr* arr, u64** out_arr,
                                    bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than f32, when false the unmatched types will be
 * ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_f32_da_to_flat_arr(DCDynArr* arr, f32** out_arr,
                                    bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than f64, when false the unmatched types will be
 * ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_f64_da_to_flat_arr(DCDynArr* arr, f64** out_arr,
                                    bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than uptr, when false the unmatched types will
 * be ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_uptr_da_to_flat_arr(DCDynArr* arr, uptr** out_arr,
                                     bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than char, when false the unmatched types will
 * be ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_char_da_to_flat_arr(DCDynArr* arr, char** out_arr,
                                     bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than size, when false the unmatched types will
 * be ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_size_da_to_flat_arr(DCDynArr* arr, size** out_arr,
                                     bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than usize, when false the unmatched types will
 * be ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_usize_da_to_flat_arr(DCDynArr* arr, usize** out_arr,
                                      bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than string, when false the unmatched types will
 * be ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_string_da_to_flat_arr(DCDynArr* arr, string** out_arr,
                                       bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than voidptr, when false the unmatched types
 * will be ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_voidptr_da_to_flat_arr(DCDynArr* arr, voidptr** out_arr,
                                        bool must_fail);

/**
 * Converts given array to actual array of literal values in dynamic values
 * supposing all or most of the values are of a same type (see `must_fail`
 * parameter)
 *
 * @param must_fail when true causes the process to break with error code -1
 * when receives any type other than fileptr, when false the unmatched types
 * will be ignored
 *
 * @return the number of exported values or error
 */
DCResUsize dc_fileptr_da_to_flat_arr(DCDynArr* arr, fileptr** out_arr,
                                        bool must_fail);

/**
 * Default initial capacity for dynamic array
 *
 * NOTE: You can define it with your desired amount before including `dcommon.h`
 */
DC_DA_INITIAL_CAP 4

/**
 * Default initial capacity multiplier for dynamic array
 *
 * NOTE: You can define it with your desired amount before including `dcommon.h`
 */
DC_DA_CAP_MULTIPLIER 2

/**
 * Macro to define custom free function for dynamic values
 */
DC_DV_FREE_FN_DECL(NAME)

/**
 * Expands to proper enum value for given type that is used in dynamic values
 */
dc_dvt(TYPE)

/**
 * Expands to proper field name of the dynamic value union field (value)
 */
#define dc_dvf(TYPE)

/**
 * Defines a dynamic value literal which holds given type and value and is
 * marked as not allocated
 *
 * NOTE: The value must not be an allocated value
 */
dc_dv(TYPE, VALUE)                                                     

/**
 * Defines a dynamic value literal which holds given type and value and is
 * marked as allocated
 *
 * NOTE: The value must be an allocated value
 */
dc_dva(TYPE, VALUE)                                                    

/**
 * Defines new variable of NAME with given type, value and allocation status
 */
DC_DV_DEF(NAME, TYPE, VALUE, ALLOC)                                    

/**
 * Expands to setting type and value of an existing dynamic value variable and
 * reset the allocation status to false
 *
 * NOTE: The VALUE must not be allocated
 */
dc_dv_set(NAME, TYPE, VALUE)                                           

/**
 * Marks given dynamic value variable as allocated
 */
dc_dv_mark_alloc(NAME) (NAME).allocated = true

/**
 * Expands to setting type and value of an existing dynamic value variable and
 * set the allocation status to true
 *
 * NOTE: The VALUE must be allocated
 */
dc_dv_seta(NAME, TYPE, VALUE)                                          

/**
 * Expands to type checking for the given dynamic value variable
 */
dc_dv_is(NAME, TYPE)

/**
 * Expands to negative type checking for the given dynamic value variable
 */
dc_dv_is_not(NAME, TYPE)

/**
 * Retrieves the value of the given dynamic value for the given type
 */
dc_dv_as(NAME, TYPE)

/**
 *  Checks if the dynamic value is marked as allocated
 */
dc_dv_is_allocated(NAME)

/**
 * Checks if the given index is correct according to the dynamic array number of
 * elements
 */
dc_da_is_index_valid(DARR, INDEX)

/**
 * Retrieves dynamic value element at certain index as is
 *
 * NOTE: There is no boundary check in this macro, you have to do it beforehand
 */
#define dc_da_get2(DARR, INDEX)

/**
 * Retrieves dynamic value element at certain index and return the wanted type
 *
 * NOTE: There is no boundary check in this macro, you have to do it beforehand
 */
dc_da_get_as(DARR, INDEX, TYPE)

/**
 * Checks if element at certain index is of the given type
 *
 * NOTE: There is no boundary check in this macro, you have to do it beforehand
 */
dc_da_is(DARR, INDEX, TYPE) 

/**
 * Checks if element at certain index is not of the given type
 *
 * NOTE: There is no boundary check in this macro, you have to do it beforehand
 */
dc_da_is_not(DARR, INDEX, TYPE)                                        

/**
 * Expands to a for loop for the given dynamic array, index can be accessed by
 * `_idx`
 */
dc_da_for(DARR)

/**
 * Macro to initialize the dynamic array with initial values without providing
 * the count
 *
 * @param DARRPTR address of the array to be initialized (DCDynArr*)
 */
dc_da_init_with_values(DARRPTR, FREE_FUNC, ...)                        

/**
 * Tries to initialize the dynamic array with initial values without providing
 * the count and saves the result (ok or error) in the given RES
 *
 * @param DARRPTR address of the array to be initialized (DCDynArr*)
 */
dc_try_da_init_with_values(RES, DARRPTR, FREE_FUNC, ...)               

/**
 * Tries to initialize the dynamic array with initial values without providing
 * the count and saves the result (ok or error) in the main result variable
 * (__dc_res) and returns if it encounter error
 *
 * @param DARRPTR address of the array to be initialized (DCDynArr*)
 */
dc_try_fail_da_init_with_values(DARRPTR, FREE_FUNC, ...)               

/**
 * Tries to initialize the dynamic array with initial values without providing
 * the count and saves the result (ok or error) in a temporary result variable
 * and returns if it encounter error
 *
 * @param DARRPTR address of the array to be initialized (DCDynArr*)
 */
dc_try_fail_temp_da_init_with_values(DARRPTR, FREE_FUNC, ...)          

/**
 * Macro to append given data to the dynamic array without providing the count
 *
 * @param DARRPTR address of the array to be initialized (DCDynArr*)
 */
dc_da_append_values(DARRPTR, ...)                                      

/**
 * Tries to append given data to the dynamic array without providing the count
 * and saves the result (ok or error) in the given RES
 *
 * @param DARRPTR address of the array to be initialized (DCDynArr*)
 */
dc_try_da_append_values(RES, DARRPTR, ...)                             

/**
 * Tries to append given data to the dynamic array without providing the count
 * and saves the result (ok or error) in the main result variable
 * (__dc_res) and returns if it encounter error
 *
 * @param DARRPTR address of the array to be initialized (DCDynArr*)
 */
dc_try_fail_da_append_values(DARRPTR, ...)                             

/**
 * Tries to append given data to the dynamic array without providing the count
 * and saves the result (ok or error) in a temporary result variable and returns
 * if it encounter error
 *
 * @param DARRPTR address of the array to be initialized (DCDynArr*)
 */
dc_try_fail_temp_da_append_values(DARRPTR, ...)                        

/**
 * Macro to insert given data to the dynamic array at certain index without
 * providing the count
 *
 * @param DARRPTR address of the array to be initialized (DCDynArr*)
 */
dc_da_insert_values(DARRPTR, INDEX, ...)                               

/**
 * Tries to insert given data to the dynamic array at certain index without
 * providing the count and saves the result (ok or error) in the given RES
 *
 * @param DARRPTR address of the array to be initialized (DCDynArr*)
 */
dc_try_da_insert_values(RES, DARRPTR, INDEX, ...)                      

/**
 * Tries to insert given data to the dynamic array at certain index without
 * providing the count and saves the result (ok or error) in the main result
 * variable (__dc_res) and returns if it encounter error
 *
 * @param DARRPTR address of the array to be initialized (DCDynArr*)
 */
dc_try_fail_da_insert_values(DARRPTR, INDEX, ...)                      

/**
 * Tries to insert given data to the dynamic array at certain index without
 * providing the count and saves the result (ok or error) in a temporary result
 * variable (__dc_res) and returns if it encounter error
 *
 * @param DARRPTR address of the array to be initialized (DCDynArr*)
 */
dc_try_fail_temp_da_insert_values(DARRPTR, INDEX, ...)                 

```

## String View

**👉 Checkout [example](/examples/test_string_view.c)**

```c
/**
 * It is used to address a portion of a string without memory allocation.
 *
 * NOTE: cstr when initiated will be allocated to the exact copy of the string
 * piece SV pointing to
 */
typedef struct
{
    string str;
    usize len;
    string cstr;
} DCStringView;

/**
 * Creates and return a string view literal struct
 *
 * @return a literal DCStringView or error
 */
DCResSv dc_sv_create(string base, usize start, usize length);

/**
 * Renders the string view into the cstr field or returns it if it is already
 * rendered
 *
 * @return string or error
 */
DCResString dc_sv_as_cstr(DCStringView* sv);

/**
 * Frees the string view's cstr field if it is initiated
 *
 * @return nothing or error
 */
DCResVoid dc_sv_free(DCStringView* sv);

/**
 * Format specifier for string views to be used in printf, dc_sprintf, etc.
 *
 * NOTE: If used `dc_sv_fmt` must be provided to provide proper data for this
 * format specifier
 *
 * @example printf("Hello " DCPRIsv "!", dc_sv_fmt(my_string_view));
 */
DCPRIsv

/**
 * If DCPRIsv is used as format specifier this macro should be used to provide
 * both length and pointer to the base string
 *
 * @example printf("Hello " DCPRIsv "!", dc_sv_fmt(my_string_view));
 */
dc_sv_fmt(SV)

/**
 * Checks if a string view and a given string are equal or not
 */
dc_sv_str_eq(SV, STR)                                                 

/**
 * Checks if two given string views are equal
 */
dc_sv_sv_eq(SV1, SV2)                                                 

```

## Hash Table

**👉 Checkout [example](/examples/test_hash_table.c).**

```c
/**
 * Each key_value of a hash table can have a dynamic value
 *
 * NOTE: The key is voidptr and must be fixed or properly handled throughout
 * hash function process
 */
typedef struct
{
    voidptr key;
    DCDynVal value;
} DCKeyValuePair;

/**
 * Function pointer type as an acceptable hash function for an Hash Table
 */
typedef DCResU32 (*DCHashFn)(voidptr);

/**
 * Key comparison function type for an Hash Table
 */
typedef DCResBool (*DCKeyCompFn)(voidptr, voidptr);

/**
 * A Hash Table with track of capacity and number of registered keys
 *
 * Container is a fixed (one time allocated) array of dynamic arrays which will
 * help in case any collision happen with different keys
 */
typedef struct
{
    DCDynArr* container;
    usize cap;
    usize key_count;

    DCHashFn hash_fn;
    DCKeyCompFn key_cmp_fn;
    DCDynValFreeFn element_free_fn;
} DCHashTable;

/**
 * Initializes the given pointer to hash table with wanted capacity and other
 * information (see params)
 *
 * NOTE: capacity cannot and must not be changed after initialization
 *
 * @param hash_fn is the function that hashes the provided keys, keys are
 * voidptr so they can be anything so to say
 *
 * @param key_cmp_fn is the function that compares a provided key and keys in
 * the buckets
 *
 * @param element_free_fn as they values are dynamic values if they must be
 * freed using special process this is the parameter to be provided
 *
 * @return nothing or error
 */
DCResVoid dc_ht_init(DCHashTable* ht, usize capacity, DCHashFn hash_fn,
                        DCKeyCompFn key_cmp_fn, DCDynValFreeFn element_free_fn);

/**
 * Creates, allocates, initializes and returns a pointer to hash table
 *
 * @return hash table pointer (DCHashTable*) or error
 *
 * NOTE: Allocates memory
 */
DCResHt dc_ht_new(usize capacity, DCHashFn hash_fn, DCKeyCompFn key_cmp_fn,
                     DCDynValFreeFn element_free_fn);

/**
 * Frees the given hash table and all the values
 *
 * @return nothing or error
 */
DCResVoid dc_ht_free(DCHashTable* ht);

/**
 * Searches for the key and provides the value
 *
 * @param out_result is the pointer to the dynamic value pointer in the hash
 * table
 *
 * @return index of the value in its bucket or error
 */
DCResUsize dc_ht_find_by_key(DCHashTable* ht, voidptr key,
                                DCDynVal** out_result);

/**
 * Sets a value for the given key
 *
 * @return nothing or error
 */
DCResVoid dc_ht_set(DCHashTable* ht, voidptr key, DCDynVal value);

/**
 * Merges and overwrites the key/values from the `from` hash table to the
 * original `ht` hash table
 *
 * @return nothing or error
 */
DCResVoid dc_ht_merge(DCHashTable* ht, DCHashTable* from);

/**
 * Deletes the given key if key does not exists return false
 *
 * @return true if key exists, false if it doesn't or error
 */
DCResBool dc_ht_delete(DCHashTable* ht, voidptr key);

/**
 * Exports all the stored keys to the provided `out_arr`
 *
 * @return the number of exported keys or error
 *
 * NOTE: Allocates memory
 */
DCResUsize dc_ht_keys(DCHashTable* ht, voidptr** out_arr);

/**
 * Expands to standard hash function declaration
 */
DC_HT_HASH_FN_DECL(NAME) DCResU32 NAME(voidptr _key)

/**
 * Expands to standard hash key comparison function declaration
 */
DC_HT_KEY_CMP_FN_DECL(NAME)                                           

/**
 * Gets the results of hash table's hash function for the given key
 *
 * And saves the results to the temporary variable of type DCResU32 then
 * extracts the u32 value of it without checking if the result was an OK or an
 * error
 *
 * NOTE: This assumes operations in the hash function is always OK not error
 */
dc_ht_get_hash(VAR_NAME, HT, KEY)                                     

/**
 * Gets the results of hash table's hash function for the given key
 *
 * And saves the results to the main result variable (__dc_res) then
 * extracts the u32 value of it only checking if the result was an OK otherwise
 * it will return the error
 */
dc_try_fail_ht_get_hash(VAR_NAME, HT, KEY)                            

/**
 * Gets the results of hash table's hash function for the given key
 *
 * And saves the results to the temporary variable of type DCResU32 then
 * extracts the u32 value of it only checking if the result was an OK otherwise
 * it will return the error
 */
dc_try_fail_temp_ht_get_hash(VAR_NAME, HT, KEY)                       

/**
 * Defines VAR_NAME as a pointer to the row of hash table container that the
 * hash indexes to
 *
 * NOTE: VAR_NAME would be DCDynArr*
 */
DC_HT_GET_AND_DEF_CONTAINER_ROW(VAR_NAME, HT, HASH)                           

/**
 * Creates a literal hash table key/value pair (key_value)
 */
dc_ht_key_value(KEY, VAL)                                                 

/**
 * Sets multiple key value pairs in a hash table without providing the count
 *
 * NOTE: It does not check whether the result of the success is OK or error
 */
dc_ht_set_multiple(HT, ...)                                           

/**
 * Tries to sets multiple key value pairs in a hash table without providing the
 * count and saves the result in the given RES (must be defined beforehand of
 * type DCResVoid)
 *
 * NOTE: It does not check whether the result of the success is OK or error
 */
dc_try_ht_set_multiple(RES, HT, ...)                                  

/**
 * Tries to sets multiple key value pairs in a hash table without providing the
 * count and saves the result in the given main result variable (__dc_res) and
 * returns if the result is a failure
 */
dc_try_fail_ht_set_multiple(HT, ...)                                  

/**
 * Tries to sets multiple key value pairs in a hash table without providing the
 * count and saves the result in a temporary variable of type DCResVoid) and
 * returns if the result is a failure
 */
dc_try_fail_temp_ht_set_multiple(HT, ...)                             

```

## Utilities: Cleanups, Logs, Console Colors, etc

**👉 Checkout [example 1](/examples/test_utils.c).**

**👉 Checkout [example 2](/examples/test_hash_table.c).**

### System

Based on the operating system one of the following macros will be defined:

```c
DC_WINDOWS
DC_MACOS
DC_LINUX
DC_ANDROID
DC_UNIX
```

Also based on the architectures one of the following macros will be defined:

```c
DC_WINDOWS_X64
DC_WINDOWS_X86
DC_WINDOWS_ARM64
DC_WINDOWS_ARM

DC_MACOS_X64
DC_MACOS_X86
DC_LINUX_X64

DC_LINUX_ARM64
DC_LINUX_ARM
DC_LINUX_X86

DC_ANDROID_X64
DC_ANDROID_ARM64
DC_ANDROID_ARM
DC_ANDROID_X86

DC_UNIX_X64
DC_UNIX_ARM64
DC_UNIX_ARM
DC_UNIX_X86

```

👉 Beside the macros there are couple of helper functions and macro functions:

```c
/**
 * Returns address of user's home directory
 *
 * @return string (might be NULL)
 */
string dc_get_home_dir_path();

/**
 * Returns current username
 *
 * @return string (might be NULL)
 */
string dc_get_username();

/**
 * Returns string representation of the operating system
 *
 * @returns string ("unknown" in case of failure)
 */
string dc_get_os();

/**
 * Returns string representation of the system architecture
 *
 * @returns string ("unknown" in case of failure)
 */
string dc_get_arch();

/**
 * Creates temporary string of given format and data and runs it under the
 * system and keeps the returned value in the OUT_VAL
 */
dc_system(OUT_VAL, ...)

```

### Logging

**👉 Note:** All logs have timestamps and file/line number information.

```c
/**
 * General macro that returns current __FILE__ filename only
 */
__FILENAME__

/**
 * Logs to `stderr` or global `dc_error_logs` if not `NULL` with `["LOG"]`
 * prefix
 */
dc_log(...)

/**
 * Logs to `stderr` or global `dc_error_logs` if not `NULL` with `["LOG"]`
 * prefix if the CONDITION is met
 */
dc_log_if(CONDITION, ...) 

/**
 * Checks whether the CONDITION is true and performs FAILURE_ACTION and
 * logs to `stderr` or global `dc_error_logs` if not `NULL` with `["LOG"]`
 * prefix
 */
dc_action_on(CONDITION, FAILURE_ACTION, ...)                          


/**
 * If `DC_DEBUG` is defined logs to `stderr` or global `dc_error_logs` if not
 * `NULL` with `["LOG"]` prefix
 */
dc_dbg_log(...)

/**
 * If `DC_DEBUG` is defined logs to `stderr` or global `dc_error_logs` if not
 * `NULL` with `["LOG"]` prefix if the CONDITION is met
 */
dc_dbg_log_if(CONDITION, ...)                                         

/**
 * If `DC_DEBUG` is defined checks whether the CONDITION is true and performs
 * FAILURE_ACTION and logs to `stderr` or global `dc_error_logs` if not `NULL`
 * with `["LOG"]` prefix
 *
 * NOTE: It doesn't do anything when `DC_DEBUG` is not defined, not that it
 * won't just output anything
 */
dc_dbg_action_on(CONDITION, FAILURE_ACTION, ...)                      
```

👉 Logs uses a global variable called `dc_error_logs` as a reference for where to put the logs, it's `NULL` by default so the logs goes to `stderr`.

You can use the following functions:

```c
/**
 * Writes date and time with given format to given stream or stdout
 */
void dc_fprintf_datetime(fileptr stream, const string format);

/**
 * Writes date and time with the default format of "%Y-%m-%d %H:%M:%S" to the
 * stream
 */
void dc_now(fileptr stream);

/**
 * Writes time with the default format of "%H:%M:%S" to the stream
 */
void dc_time(fileptr stream);

/**
 * Writes date with the default format of "%Y-%m-%d" to the stream
 */
void dc_date(fileptr stream);

/**
 * It will open the give filename or `_error_logs.log` if file name is NULL and
 * initialize global `dc_error_logs` with it, so all the logs (normal or debugs)
 * will be written to the file instead of stderr
 *
 * @return nothing or exit with proper error code
 */
void dc_error_logs_init(string filename, bool append);

/**
 * Closes `dc_error_logs` if it's already opened
 *
 * NOTE: Only cleanup pool (DC_CLEANUP_POOL mode) automatically does this
 * otherwise you must do this manually
 */
void dc_error_logs_close();

```

### Strings, Paths, File, etc

```c
/**
 * Simple macro to create a switch case for an enum and returning the string
 * literal of it
 */
dc_str_case(ITEM)                                                      

/**
 * returns "true" or "false" as strings for given expression
 */
dc_tostr_bool(B)

/**
 * Allocates and fill out the given string `str` with formats and data
 *
 * @return size of allocated string or error
 *
 * NOTE: Allocates memory
 */
DCResUsize dc_sprintf(string* str, string fmt, ...);

/**
 * Appends the given string `str` with given formats and data
 *
 * @return size of allocated string or error
 *
 * NOTE: Allocates memory when str is empty or reallocates it
 */
DCResUsize dc_sappend(string* str, const string fmt, ...);

/**
 * Duplicates given string and returns it
 *
 * @return string or error
 *
 * NOTE: Allocates memory
 */
DCResString dc_strdup(const string in);

/**
 * Converts the current value of the dynamic value to string
 *
 * @return string or error
 */
DCResString dc_tostr_dv(DCDynVal* dv);

/**
 * Prints the current value of the dynamic value (no new line)
 *
 * @return nothing or error
 */
DCResVoid dc_dv_print(DCDynVal* dv);

/**
 * Prints the current value of the dynamic value (with new line)
 *
 * @return nothing or error
 */
DCResVoid dc_dv_println(DCDynVal* dv);

/**
 * Replaces all the '' with '/' in the original provided path
 *
 * @return nothing or error
 */
DCResVoid dc_normalize_path_to_posix(string path);

/**
 * Replaces the filename in the path with the given filename and returns a new
 * string as the result
 *
 * @returns string or error
 *
 * NOTE: Allocates memory
 */
DCResString dc_replace_file_in_path(const string path,
                                       const string file_name);

/**
 * Returns address of user's home directory
 *
 * @return string (might be NULL)
 */
string dc_get_home_dir_path();

/**
 * Tries to open a file and return proper fileptr or error code with error
 * message
 */
DCResFileptr dc_file_open(const string file, const string mode);

```

👉 There are also some macros for colorizing texts in terminal:

```c
// ***************************************************************************************
// * COLOR MACROS
// ***************************************************************************************

DC_FG_BLACK
DC_FG_RED
DC_FG_GREEN 
DC_FG_YELLOW 
DC_FG_BLUE 
DC_FG_MAGENTA 
DC_FG_CYAN
DC_FG_WHITE
DC_FG_GRAY
DC_FG_LRED
DC_FG_LGREEN 
DC_FG_LYELLOW
DC_FG_LBLUE 
DC_FG_LMAGENTA 
DC_FG_LCYAN
DC_FG_LWHITE

DC_BG_BLACK
DC_BG_RED 
DC_BG_GREEN
DC_BG_YELLOW
DC_BG_BLUE 
DC_BG_MAGENTA
DC_BG_CYAN
DC_BG_WHITE
DC_BG_GRAY
DC_BG_LRED
DC_BG_LGREEN
DC_BG_LYELLOW 
DC_BG_LBLUE 
DC_BG_LMAGENTA
DC_BG_LCYAN 
DC_BG_LWHITE

DC_COLOR_RESET

/**
 * Colorizes given literal string by adding background and foreground string at
 * the beginning and color reset at the end
 * 
 * NOTE: Just provide color name in ALL CAPS like RED, LRED, ...
 *
 * NOTE: This is not a string allocation process this is simply putting string
 * literals together
 */
dc_colorize(BG_COLOR, FG_COLOR, TEXT)                                  

/**
 * Colorizes given literal string by adding foreground string at
 * the beginning and color reset at the end
 * 
 * NOTE: Just provide color name in ALL CAPS like RED, LRED, ...
 *
 * NOTE: This is not a string allocation process this is simply putting string
 * literals together
 */
dc_colorize_fg(FG_COLOR, TEXT)

/**
 * Colorizes given literal string by adding background string at
 * the beginning and color reset at the end
 * 
 * NOTE: Just provide color name in ALL CAPS like RED, LRED, ...
 *
 * NOTE: This is not a string allocation process this is simply putting string
 * literals together
 */
dc_colorize_bg(BG_COLOR, TEXT)

```

### Cleanups

The concept behind `Cleanups` is simply to be able to have a pool of cleanup batches containing various cleanup jobs and each batch can be cleaned up at certain point or the whole pool get cleaned up all at once.

👉 One example usage as mentioned above is in the [test_hash_table.c](/examples/test_hash_table.c).

```c
/**
 * It is an alias for regular dynamic array that holds voidptr to multiple
 * DCCleanupJob (a batch of cleanup jobs)
 */
typedef DCDynArr DCCleanupBatch;

/**
 * Is a pool of cleanups that can be triggered at any phase of program life
 * cycle. (a pool of cleanup batches)
 *
 * As an example you might separate your program into multiple phases and
 * you want to have separate batch for each phase, you can use this method.
 *
 * NOTE: The default batch is batch index 0
 */
typedef struct
{
    DCCleanupBatch* pool;
    usize count;
} DCCleanupPool;

/**
 * Function pointer type for acceptable functions in cleanup process
 *
 * NOTE: If a dynamic value has a complex cleanup process you need to create
 * proper function with this signature and register it with the created function
 */
typedef DCResVoid (*DCCleanupFn)(voidptr);

/**
 * Is a memory allocated object that needs to be managed
 */
typedef struct
{
    voidptr element;
    DCCleanupFn cleanup_fn;
} DCCleanupJob;

/**
 * Initializes the global cleanup pool
 *
 * You can have 1 Batch with bigger capacity if you want to cleanup only once
 *
 * Or you can have multiple batches in the pool and cleanup them in different
 * parts of the program
 *
 * This also registers signal catcher functions `__dc_handle_signal` for proper
 * cleanups where applies
 */
void dc_cleanup_pool_init2(usize count, usize batch_capacity);

/**
 * Performs the cleanup process for any given cleanup batch
 *
 * NOTE: This function is meant to be used internally by the
 * `dc_cleanup_pool_run`, but you can indeed define your own
 * `DCCleanupBatch` variable and use `dc_cleanup_push2` pushing to it and
 * cleaning it up using this function and that's totally ok if you need to.
 *
 * @return nothing or error
 */
DCResVoid dc_cleanup_batch_run(DCCleanupBatch* batch);

/**
 * Performs the whole pool or selected cleanup or nothing
 *
 * @param selection choose DC_CLEANUP_POOL or -1, DC_NO_CLEANUP or -2 or other
 * valid batch index starting from 0
 *
 * @return nothing or exits with error code
 */
void dc_cleanup_pool_run(i32 selection);

/**
 * Pushes an allocated address and corresponding cleanup function to be
 * stored in the given cleanup batch
 *
 * @return nothing or exits with error code
 */
void dc_cleanup_push2(DCCleanupBatch* batch, voidptr element,
                      DCCleanupFn cleanup_fn);

/**
 * Wrapper for `free` in case to be able to be used in cleanup process
 *
 * @returns nothing or error
 *
 * NOTE: Non-Null allocated pointer must be sent otherwise undefined behavior
 * might happen
 */
DCResVoid dc_free(voidptr variable);

/**
 * Frees the error message of result variables if they have any allocated error
 * message string
 *
 * @return nothing or error
 *
 * NOTE: Only proper result type based pointers must be sent otherwise undefined
 * behavior might happen
 *
 * NOTE: Checkout aliases.h if you want to create your own compatible type
 */
DCResVoid dc_result_free(voidptr res_ptr);

/**
 * Constant for DC_EXIT_SECTION and dc_cleanup_pool_run
 *
 * It means the exit section will trigger dc_cleanup_pool_run in cleaning up
 * the whole pool mode
 */
DC_CLEANUP_POOL -1

/**
 * Constant for DC_EXIT_SECTION and dc_cleanup_pool_run
 *
 * It means the exit section will trigger dc_cleanup_pool_run to holdup and
 * does nothing
 */
DC_NO_CLEANUP -2

/**
 * Expands to standard function declarations for cleanup process
 */
DC_CLEANUP_FN_DECL(NAME)

/**
 * Runs the cleanup process of a cleanup job key_value
 */
dc_cleanup_job_run(ENTRY)

/**
 * Initializes a cleanup batch with batch capacity only if it is not initialized
 * yet
 */
DC_CLEANUP_BATCH_INIT(CLEANUP_BATCH, BATCH_CAPACITY)                   

/**
 * Expands to initializing pool with one default batch
 */
dc_cleanup_pool_init(BATCH_CAPACITY)                                   

/**
 * Declares __dc_res with given type and initial value
 *
 * NOTE: This is useful for non-result types, for result type you can use
 * DC_RES() or DC_RES_<type>()
 */
DC_RET_VAL_INIT(RETVAL_TYPE, RETVAL_INIT)                              

/**
 * Pushes an allocated memory address (ELEMENT) and its corresponding cleanup
 * function pointer to the given batch index
 */
dc_cleanup_pool_push(BATCH_INDEX, ELEMENT, CLEAN_FUNC)                 

/**
 * Flushes (frees) selected batch index
 *
 * NOTE: No bound check for BATCH_INDEX warning
 *
 * @return nothing or error
 */
dc_cleanup_batch_flush(BATCH_INDEX)                                    

/**
 * Pops last n elements from the selected batch index
 *
 * NOTE: No bound check for BATCH_INDEX warning
 *
 * @return nothing or error
 */
dc_cleanup_batch_pop(BATCH_INDEX, COUNT)                               

/**
 * Pushes an allocated memory address (ELEMENT) and its corresponding cleanup
 * function pointer to the default batch index which is 0
 */
dc_cleanup_default_pool_push(ELEMENT, CLEAN_FUNC)                      

/**
 * Pushes given hash table address with default standard hash table cleanup in
 * the default batch (index 0)
 */
dc_cleanup_push_ht(ELEMENT)                                            

/**
 * Pushes given hash table address with default standard hash table cleanup in
 * the given batch index
 */
dc_cleanup_push_ht2(BATCH_INDEX, ELEMENT)                              

/**
 * Pushes given dynamic array address with default standard dynamic array
 * cleanup in the default batch (index 0)
 */
dc_cleanup_push_da(ELEMENT)                                            

/**
 * Pushes given dynamic array address with default standard dynamic array
 * cleanup in the given batch index
 */
dc_cleanup_push_da2(BATCH_INDEX, ELEMENT)                              

/**
 * Pushes given dynamic value address with default standard dynamic value
 * cleanup in the default batch (index 0)
 *
 * NOTE: If your dynamic value has complex cleanup process you need to implement
 * your own standard cleanup function for that and use regular cleanup push or
 * push2 for it
 */
dc_cleanup_push_dv(ELEMENT)                                            

/**
 * Pushes given dynamic value address with default standard dynamic value
 * cleanup in the given batch index
 *
 * NOTE: If your dynamic value has complex cleanup process you need to implement
 * your own standard cleanup function for that and use regular cleanup push or
 * push2 for it
 */
dc_cleanup_push_dv2(BATCH_INDEX, ELEMENT)                              

/**
 * Pushes given result variable address with default standard result variable
 * cleanup in the default batch (index 0)
 */
dc_cleanup_push_res(ELEMENT)                                           

/**
 * Pushes given result variable address with default standard result variable
 * cleanup in the given batch index
 */
dc_cleanup_push_res2(BATCH_INDEX, ELEMENT)                             

/**
 * Pushes given allocated memory address with default standard allocated memory
 * cleanup (free) in the default batch (index 0)
 */
dc_cleanup_push_free(ELEMENT)                                          

/**
 * Pushes given allocated memory address with default standard allocated memory
 * cleanup (free) in the given batch index
 */
dc_cleanup_push_free2(BATCH_INDEX, ELEMENT)                            

/**
 * Pushes given allocated memory address with `fclose` in the default batch
 * (index 0)
 */
dc_cleanup_push_file(ELEMENT)                                          

/**
 * Pushes given allocated memory address with `fclose` in the given batch index
 */
dc_cleanup_push_file2(BATCH_INDEX, ELEMENT)                            

/**
 * Defines a label that first triggers the cleanup of choice and then returns
 * the current value of __dc_res
 *
 * NOTE: For scopes that the __dc_res is of result type you don't want to use
 * any direct try_fail stuff as you want to catch the error and jump to the exit
 * point using dc_return or dc_return_with_val
 *
 * @param SELECTION can be `DC_CLEANUP_POOL` or -1 to trigger cleaning up the
 * whole pool and `DC_NO_CLEANUP` or -2 for no cleanup at all, otherwise you can
 * choose to set a valid index for a batch like 0, 1, ... (as long as you've
 * already initialized the pool with correct number of batches)
 */
DC_EXIT_SECTION(SELECTION)                                           

/**
 * Jumps to the __dc_exit_label
 *
 * NOTE: DC_EXIT_SECTION must also be used at the end of current scope
 */
dc_return() goto __dc_exit_label

/**
 * Sets the __dc_res and jumps to the __dc_exit_label
 *
 * NOTE: DC_EXIT_SECTION must also be used at the end of current scope
 */
dc_return_with_val(RETVAL)                                             
    do                                                                         
    {                                                                          
        __dc_res = RETVAL;                                                     
        dc_return();                                                           
    } while (0)

/**
 * If provided RES is an error result it catches the error by copying it to
 * __dc_res does PRE_RETURN_ACTIONS and the jumps to __dc_exit_label
 *
 * NOTE: DC_EXIT_SECTION must also be used at the end of current scope
 */
dc_return_if_err(RES, PRE_RETURN_ACTIONS)                              

/**
 * If provided RES is an error result it does the PRE_RETURN_ACTIONS then sets
 * the __dc_res to RET_VAL and the jumps to __dc_exit_label
 *
 * NOTE: DC_EXIT_SECTION must also be used at the end of current scope
 */
dc_return_if_err2(RES, RET_VAL, PRE_RETURN_ACTIONS)                    

```

## License

This project is licensed under the BSD 3-Clause [LICENSE](/LICENSE).

BSD 3-Clause License

Copyright (c) 2024, Navid Dezashibi <navid@dezashibi.com>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
