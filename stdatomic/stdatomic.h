
// https://en.cppreference.com/w/c/atomic
// https://gitlab.inria.fr/gustedt/stdatomic
// https://github.com/cdschreiber/c11

#ifndef _STDATOMIC_H
#define _STDATOMIC_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) \
    && !defined(__STDC_NO_ATOMICS__)
    #define _USE_STD
#elif defined(__GNUC__)
    #define _USE_GCC
#elif defined(_MSC_VER)
    #define _USE_MSVC
#elif defined(__clang__)
    #define _USE_CLANG
#else
    #define _USE_NONE
#endif /* __STDC_NO_ATOMICS__ */

// override
#undef _USE_STD
#undef _USE_GCC
#undef _USE_MSVC
#undef _USE_CLANG
#undef _USE_NONE
#define _USE_GCC

#if !defined(_USE_STD)
// common defines

/*
 *  7.17.2 Initialization
 */

#define ATOMIC_VAR_INIT(value) (value)
// #define atomic_init(obj, desired) (void)(*(obj) = (desired))

/*
 *  7.17.3 Order and consistency
 */

typedef enum memory_order {
    memory_order_relaxed,
    memory_order_consume,
    memory_order_acquire,
    memory_order_release,
    memory_order_acq_rel,
    memory_order_seq_cst
} memory_order;

/*
 *  7.17.6 Atomic integer types
 */
#define _Atomic(T) T
/* typedef _Atomic(bool)               atomic_bool;
typedef _Atomic(char)               atomic_char;
typedef _Atomic(unsigned char)      atomic_uchar;
typedef _Atomic(short)              atomic_short;
typedef _Atomic(unsigned short)     atomic_ushort;
typedef _Atomic(int)                atomic_int;
typedef _Atomic(unsigned int)       atomic_uint;
typedef _Atomic(long)               atomic_long;
typedef _Atomic(unsigned long)      atomic_ulong;
typedef _Atomic(__int64)            atomic_llong;
typedef _Atomic(unsigned __int64)   atomic_ullong;
typedef _Atomic(uint_least16_t)     atomic_wchar_t;
typedef _Atomic(uint_least16_t)     atomic_char16_t;
typedef _Atomic(uint_least32_t)     atomic_char32_t;
typedef _Atomic(int_least8_t)       atomic_int_least8_t;
typedef _Atomic(uint_least8_t)      atomic_uint_least8_t;
typedef _Atomic(int_least16_t)      atomic_int_least16_t;
typedef _Atomic(uint_least16_t)     atomic_uint_least16_t;
typedef _Atomic(int_least32_t)      atomic_int_least32_t;
typedef _Atomic(uint_least32_t)     atomic_uint_least32_t;
typedef _Atomic(int_least64_t)      atomic_int_least64_t;
typedef _Atomic(uint_least64_t)     atomic_uint_least64_t;
typedef _Atomic(int_fast8_t)        atomic_int_fast8_t;
typedef _Atomic(uint_fast8_t)       atomic_uint_fast8_t;
typedef _Atomic(int_fast16_t)       atomic_int_fast16_t;
typedef _Atomic(uint_fast16_t)      atomic_uint_fast16_t;
typedef _Atomic(int_fast32_t)       atomic_int_fast32_t;
typedef _Atomic(uint_fast32_t)      atomic_uint_fast32_t;
typedef _Atomic(int_fast64_t)       atomic_int_fast64_t;
typedef _Atomic(uint_fast64_t)      atomic_uint_fast64_t;
typedef _Atomic(intptr_t)           atomic_intptr_t;
typedef _Atomic(uintptr_t)          atomic_uintptr_t;
typedef _Atomic(size_t)             atomic_size_t;
typedef _Atomic(ptrdiff_t)          atomic_ptrdiff_t;
typedef _Atomic(intmax_t)           atomic_intmax_t;
typedef _Atomic(uintmax_t)          atomic_uintmax_t; */

/*
 *  7.17.7 Operations on atomic types
 */
/* Map all non-explicit macros to the explicit version. */
#define atomic_store(obj, desired) \
    atomic_store_explicit((obj), (desired), memory_order_seq_cst)
#define atomic_load(obj) \
    atomic_load_explicit((obj), memory_order_seq_cst)
#define atomic_exchange(obj, desired) \
    atomic_exchange_explicit((obj), (desired), memory_order_seq_cst)
#define atomic_compare_exchange_strong(obj, expected, desired) \
    atomic_compare_exchange_strong_explicit((obj), (expected), (desired), memory_order_seq_cst, memory_order_seq_cst)
#define atomic_compare_exchange_weak(obj, expected, desired) \
    atomic_compare_exchange_weak_explicit((obj), (expected), (desired), memory_order_seq_cst, memory_order_seq_cst)
#define atomic_fetch_add(obj, desired) \
    atomic_fetch_add_explicit((obj), (desired), memory_order_seq_cst)
#define atomic_fetch_sub(obj, desired) \
    atomic_fetch_sub_explicit((obj), (desired), memory_order_seq_cst)
#define atomic_fetch_or(obj, desired) \
    atomic_fetch_or_explicit((obj), (desired), memory_order_seq_cst)
#define atomic_fetch_xor(obj, desired) \
    atomic_fetch_xor_explicit((obj), (desired), memory_order_seq_cst)
#define atomic_fetch_and(obj, desired) \
    atomic_fetch_and_explicit((obj), (desired), memory_order_seq_cst)

/*
 *  7.17.8 Atomic flag type and operations
 */
#define atomic_flag _Atomic(bool) // typedef atomic_bool atomic_flag;
#define ATOMIC_FLAG_INIT false
#define atomic_flag_test_and_set(obj) \
    atomic_flag_test_and_set_explicit((obj), memory_order_seq_cst)
#define atomic_flag_clear(obj) \
    atomic_flag_clear_explicit((obj), memory_order_seq_cst)
/* #define atomic_flag_test_and_set_explicit(obj, order) \
    atomic_exchange_explicit((obj), true, (order))
#define atomic_flag_clear_explicit(obj, order) \
    atomic_store_explicit((obj), false, (order)) */

#endif

// |_Atomic| should be smaller than WORDSIZE(maybe (u)intptr_t)
#if defined(_USE_STD)
#include <stdatomic.h>
#elif defined(_USE_GCC)
// use __sync or __atomic
// var
/* #define _Atomic(_type) _type
#define ATOMIC_VAR_INIT(_val) (_val) */
/* static inline
bool atomic_compare_exchange_strong_explicit(
    volatile uintptr_t *obj,
    uintptr_t *expected,
    uintptr_t desired,
    memory_order succ,
    memory_order fail)
{
    if (__sync_bool_compare_and_swap(obj, *expected, desired)) {
        return true;
    } else {
        *expected = *obj;
        return false;
    }
    (void)succ;
    (void)fail;
} */
#define atomic_compare_exchange_strong_explicit(_ptr, _optr, _nval, _mo) \
    __extension__({ \
        __typeof__(*_ptr) oval = *(__typeof__(_ptr))optr; \
        if (__sync_bool_compare_and_swap((_ptr), (_oval), (_nval))){ \
            true; \
        } \
        else{ \
            *_optr = *_ptr; \
            false; \
        } \
    })
#define atomic_fetch_add_explicit(_ptr, _val) __sync_fetch_and_add((_ptr), (_val))
#define atomic_store_explicit(_ptr, _val) (void)(*(_ptr) = (_val))
#define atomic_load_explicit(_ptr) (*(_ptr))
// flag
/* #define atomic_flag volatile bool
#define ATOMIC_FLAG_INIT (false) */
#define atomic_flag_test_and_set_explicit(_ptr) __sync_lock_test_and_set(_ptr, true)
#define atomic_flag_clear_explicit(_ptr) __sync_lock_release(_ptr)
#elif defined(_USE_MSVC)
// var
#define _Atomic(_type) _type
#define ATOMIC_VAR_INIT(_val) (_val)
#define atomic_compare_exchange_strong_explicit(_ptr, _oval, _nval) (_InterlockedCompareExchange((_ptr), (_oval), (_nval)) == (_oval))
#define atomic_fetch_add_explicit(_ptr, _val) _InterlockedExchangeAdd((_ptr), (_val))
#define atomic_store_explicit(_ptr, _val) (void)(*(_ptr) = (_val))
#define atomic_load_explicit(_ptr) (*(_ptr))
// flag
#define atomic_flag volatile bool
#define ATOMIC_FLAG_INIT (false)
#define atomic_flag_test_and_set_explicit(_ptr) _InterlockedExchange(_ptr, true)
#define atomic_flag_clear_explicit(_ptr) _InterlockedExchange(_ptr, false)
#elif defined(_USE_CLANG)
// #include <atomic_stub.h>
// // var
// #define _Atomic(_type) _Atomic(_type)
// // #define ATOMIC_VAR_INIT(_val)
// #define atomic_compare_exchange_strong_explicit(_ptr, _oval, _nval) __c11_atomic_compare_exchange_strong((_ptr), (_oval), (_nval), memory_order_seq_cst, memory_order_seq_cst)
// #define atomic_fetch_add_explicit(_ptr, _val) __c11_atomic_fetch_add((_ptr), (_val), memory_order_seq_cst)
// #define atomic_store_explicit(_ptr, _val) __c11_atomic_store((_ptr), (_val), memory_order_seq_cst)
// #define atomic_load_explicit(_ptr) __c11_atomic_load((_ptr), memory_order_seq_cst)
// // flag
// #define atomic_flag volatile atomic_flag
// // #define ATOMIC_FLAG_INIT
// #define atomic_flag_test_and_set_explicit(_ptr) __c11_atomic_flag_test_and_set(_ptr)
// #define atomic_flag_clear_explicit(_ptr) __c11_atomic_flag_clear(_ptr)
#elif defined(_USE_NONE)
// var
#define _Atomic(_type) _type
#define ATOMIC_VAR_INIT(_val) (_val)
#define atomic_compare_exchange_strong_explicit(_ptr, _oval, _nval) ({ bool __ret; if ((uintptr_t)_oval == *(uintptr_t *)(_ptr)) { *(_ptr) = (_nval); __ret = true; } else { __ret = false; } __ret; })
#define atomic_fetch_add_explicit(_ptr, _val) ({ __typeof__(*(_ptr)) __oval = *(_ptr); *(_ptr) += (_val); __oval; })
#define atomic_store_explicit(_ptr, _val) (void)(*(_ptr) = (_val))
#define atomic_load_explicit(_ptr) (*(_ptr))
// flag
#define atomic_flag volatile bool
#define ATOMIC_FLAG_INIT (false)
#define atomic_flag_test_and_set_explicit(_ptr) ({ bool __oval = *(_ptr); if (!__oval) { *(_ptr) = true; } __oval; })
#define atomic_flag_clear_explicit(_ptr) (void)({ *(_ptr) = false; })
#endif

#endif // _STDATOMIC_H
