#ifndef _STDATOMIC_GCC_ATOMIC_H_
#define _STDATOMIC_GCC_ATOMIC_H_ 1

#include <atomic_stub.h>

#define ATOMIC_VAR_INIT(...) __VA_ARGS__
#define atomic_init(X, V) ((void)((*(X))=(V)))

/* Map all non-explicit macros to the explicit version. */
#define atomic_fetch_add(X, Y)                  atomic_fetch_add_explicit((X), (Y), memory_order_seq_cst)
#define atomic_fetch_sub(X, Y)                  atomic_fetch_sub_explicit((X), (Y), memory_order_seq_cst)
#define atomic_fetch_and(X, Y)                  atomic_fetch_and_explicit((X), (Y), memory_order_seq_cst)
#define atomic_fetch_or(X, Y)                   atomic_fetch_or_explicit((X), (Y), memory_order_seq_cst)
#define atomic_fetch_xor(X, Y)                  atomic_fetch_xor_explicit((X), (Y), memory_order_seq_cst)
#define atomic_load(X)                          atomic_load_explicit((X), memory_order_seq_cst)
#define atomic_store(X, V)                      atomic_store_explicit((X), (V), memory_order_seq_cst)
#define atomic_exchange(X, V)                   atomic_exchange_explicit((X), (V), memory_order_seq_cst)
#define atomic_compare_exchange_weak(X, E, V)   atomic_compare_exchange_weak_explicit((X), (E), (V), memory_order_seq_cst, memory_order_seq_cst)
#define atomic_compare_exchange_strong(X, E, V) atomic_compare_exchange_strong_explicit((X), (E), (V), memory_order_seq_cst, memory_order_seq_cst)

/* The arithmetic operations don't have to use a memory operand. */
#define atomic_fetch_and_explicit(X, Y, MO) __atomic_fetch_and((X), (Y), (MO))
#define atomic_fetch_or_explicit(X, Y, MO)  __atomic_fetch_or((X), (Y), (MO))
#define atomic_fetch_xor_explicit(X, Y, MO) __atomic_fetch_xor((X), (Y), (MO))

/* gcc's implementation of the __atomic_fetch_add and
   __atomic_fetch_sub has a bug in that it views the second argument
   as a byte increment. Detect that X's base type is a pointer type
   and do the correct increment in that case.

   A lot of programming effort is done to ensure that the resulting
   code is as efficient as a direct call of the builtin would be. */

/* R can be of any scalar or array type. If R is a pointer or decays
   to one, return (void*)0. Otherwise return X. */
#define _IS_POINTER(R, X)                                       \
  (_Generic((R)+(ptrdiff_t)0,                                   \
            /* R is an integer that promotes to ptrdiff_t */    \
            ptrdiff_t: (X),                                     \
            default: (_Generic((R)-(R),                         \
                               /* R itself can't be ptrdiff_t*/ \
                               ptrdiff_t: (void*)0,             \
                               default: (X)))))

static inline
uintptr_t __atomic_fetch_add_ptr(void* x,  size_t offset, int mo) {
  _Atomic(uintptr_t)* _x = (_Atomic(uintptr_t)*)x;
  return __atomic_fetch_add(_x, offset, mo);
}

#define atomic_fetch_add_explicit(X, Y, MO)                             \
({                                                                      \
  register int const _mo = (MO);                                        \
  register size_t const _y = (Y);                                       \
  register __typeof__(__atomic_fetch_add((X), _y, _mo)) _r = 0;         \
  /* Compute the offset for an integer or pointer type. */              \
  /* Ensure that the correct arithmetic for the type is used */         \
  register uintptr_t const _offset = (uintptr_t)(_r+_y)-(uintptr_t)_r;  \
  __typeof__(_IS_POINTER(_r, (X))) _x = (X);                            \
  (__typeof__(_r))_Generic(_x,                                          \
           void*: __atomic_fetch_add_ptr,                               \
           default: __atomic_fetch_add)(_x, _offset, _mo);              \
 })

static inline
uintptr_t __atomic_fetch_sub_ptr(void* x,  size_t offset, int mo) {
  _Atomic(uintptr_t)* _x = (_Atomic(uintptr_t)*)x;
  return __atomic_fetch_sub(_x, offset, mo);
}

#define atomic_fetch_sub_explicit(X, Y, MO)                             \
({                                                                      \
  register int const _mo = (MO);                                        \
  register size_t const _y = (Y);                                       \
  register __typeof__(__atomic_fetch_sub((X), _y, _mo)) _r = 0;         \
  /* Compute the offset for an integer or pointer type. */              \
  /* Ensure that the correct arithmetic for the type is used */         \
  register size_t const _offset = (uintptr_t)(_r+_y)-(uintptr_t)_r;     \
  __typeof__(_IS_POINTER(_r, (X))) _x = (X);                            \
  (__typeof__(_r))_Generic(_x,                                          \
           void*: __atomic_fetch_sub_ptr,                               \
           default: __atomic_fetch_sub)(_x, _offset, _mo);              \
 })

/* The interfaces for the universal functions need to operate on
   memory operands, only. */

#define atomic_load_explicit(X, MO)             \
({                                              \
  __atyp(*X) _r;                                \
  __atomic_load((X), _r, (MO));                 \
  __aret(_r[0]);                                \
 })

#define atomic_store_explicit(X, V, MO)         \
  __atomic_store((X), &__atmp(*X, V), (MO))

#define atomic_exchange_explicit(X, V, MO)              \
({                                                      \
  __atyp(*X) _r;                                        \
  __atomic_exchange((X), &__atmp(*X, V), _r, (MO));     \
  __aret(_r[0]);                                        \
 })

#define atomic_compare_exchange_weak_explicit(X, E, V, MOS, MOF)        \
  __atomic_compare_exchange((X), (E), &__atmp(*(X), (V)), 1, (MOS), (MOF))

#define atomic_compare_exchange_strong_explicit(X, E, V, MOS, MOF)       \
  __atomic_compare_exchange((X), (E), &__atmp(*(X), (V)), 0, (MOS), (MOF))


#define INSTANTIATE_STUB_LF(N, T)                                       \
T __impl_fetch_add_ ## N(_Atomic(T)* X, T const V, int M);              \
T __impl_fetch_sub_ ## N(_Atomic(T)* X, T const V, int M);              \
T __impl_fetch_and_ ## N(_Atomic(T)* X, T const V, int M);              \
T __impl_fetch_xor_ ## N(_Atomic(T)* X, T const V, int M);              \
T __impl_fetch_nand_ ## N(_Atomic(T)* X, T const V, int M);             \
T __impl_fetch_or_ ## N(_Atomic(T)* X, T const V, int M);               \
T __impl_add_fetch_ ## N(_Atomic(T)* X, T const V, int M);              \
T __impl_sub_fetch_ ## N(_Atomic(T)* X, T const V, int M);              \
T __impl_and_fetch_ ## N(_Atomic(T)* X, T const V, int M);              \
T __impl_xor_fetch_ ## N(_Atomic(T)* X, T const V, int M);              \
T __impl_nand_fetch_ ## N(_Atomic(T)* X, T const V, int M);             \
T __impl_or_fetch_ ## N(_Atomic(T)* X, T const V, int M);               \
T __impl_load_ ## N(_Atomic(T)* X, int M);                              \
void __impl_store_ ## N(_Atomic(T)* X, T const V, int M);               \
T __impl_exchange_ ## N(_Atomic(T)* X, T const V, int M);               \
_Bool __impl_compare_exchange_ ## N(_Atomic(T)* X, T* E, T const V, int MS, int MF); \
T __impl_fetch_add_ ## N(_Atomic(T)* X, T const V, int M) {             \
  return __atomic_fetch_add(X, V, M);                                   \
}                                                                       \
T __impl_fetch_sub_ ## N(_Atomic(T)* X, T const V, int M) {             \
  return __atomic_fetch_sub(X, V, M);                                   \
}                                                                       \
T __impl_fetch_and_ ## N(_Atomic(T)* X, T const V, int M) {             \
  return __atomic_fetch_and(X, V, M);                                   \
}                                                                       \
T __impl_fetch_xor_ ## N(_Atomic(T)* X, T const V, int M) {             \
  return __atomic_fetch_xor(X, V, M);                                   \
}                                                                       \
T __impl_fetch_nand_ ## N(_Atomic(T)* X, T const V, int M) {            \
  return __atomic_fetch_nand(X, V, M);                                  \
}                                                                       \
T __impl_fetch_or_ ## N(_Atomic(T)* X, T const V, int M) {              \
  return __atomic_fetch_or(X, V, M);                                    \
}                                                                       \
T __impl_add_fetch_ ## N(_Atomic(T)* X, T const V, int M) {             \
  return __atomic_add_fetch(X, V, M);                                   \
}                                                                       \
T __impl_sub_fetch_ ## N(_Atomic(T)* X, T const V, int M) {             \
  return __atomic_sub_fetch(X, V, M);                                   \
}                                                                       \
T __impl_and_fetch_ ## N(_Atomic(T)* X, T const V, int M) {             \
  return __atomic_and_fetch(X, V, M);                                   \
}                                                                       \
T __impl_xor_fetch_ ## N(_Atomic(T)* X, T const V, int M) {             \
  return __atomic_xor_fetch(X, V, M);                                   \
}                                                                       \
T __impl_nand_fetch_ ## N(_Atomic(T)* X, T const V, int M) {            \
  return __atomic_nand_fetch(X, V, M);                                  \
}                                                                       \
T __impl_or_fetch_ ## N(_Atomic(T)* X, T const V, int M) {              \
  return __atomic_or_fetch(X, V, M);                                    \
}                                                                       \
T __impl_load_ ## N(_Atomic(T)* X, int M) {                             \
  return __atomic_load_n(X, M);                                         \
}                                                                       \
void __impl_store_ ## N(_Atomic(T)* X, T const V, int M) {              \
  __atomic_store_n(X, V, M);                                            \
}                                                                       \
T __impl_exchange_ ## N(_Atomic(T)* X, T const V, int M) {              \
  return __atomic_exchange_n(X, V, M);                                  \
}                                                                       \
_Bool __impl_compare_exchange_ ## N(_Atomic(T)* X, T* E, T const V, int MS, int MF) { \
  return __atomic_compare_exchange_n(X, E, V, 0, MS, MF);               \
}


#endif
