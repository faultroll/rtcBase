#ifndef _STDATOMIC_H_
#define _STDATOMIC_H_ 1

/* Copyright 2015, Jens Gustedt, France. */

/**
 ** @file
 **
 ** @brief An realization of the stdatomic.h interface by means of gcc
 ** or clang compiler extensions.
 **
 ** This has three different realizations, using intrinsics for modern
 ** clang (__c11_atomic ...), modern gcc (__atomic ...) or for the old
 ** gcc __sync interface. The later should be available on a lot of
 ** platforms, many other compilers, including clang implement these
 ** interfaces.
 **
 ** For the first two, user code should be able to use all C11 atomic
 ** features without problems.
 **
 ** For the __sync interface, we can't assume that there is support
 ** for operators on atomics, so such code should simply not use
 ** them. But the "functional" approach to atomics should work even
 ** then. That is code that uses the _Atomic() variant to declare
 ** atomic objects and only uses the atomic_... macros as of the C11
 ** standard to act upon these objects should work.
 **
 ** This code also builds upon other gcc extensions to C:
 **
 ** - compound expressions
 ** - <code>__typeof__</code>
 ** - <code>#pragma redefine_extname</code>
 **
 ** and may eventually use
 ** - <code>__alignof__</code>
 ** - <code>__attribute__((aligned(something)))</code>
 **/


#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
/* This uses a special feature of gcc and friends to redefine a
   symbol's external name with a pragma. If your compiler doesn't
   support this you find yourself with a whole bunch of functions with
   names starting with __impl_. You'd then have to find an equivalent
   feature for your compiler (please let me know) or use an external
   tool such as objcopy to redefine the symbols.*/
#ifdef __PRAGMA_REDEFINE_EXTNAME
# include <atomic_pragma.h>
#endif
#include <atomic_constants.h>
#include <atomic_flag.h>
#include <atomic_fence.h>
#include <atomic_generic.h>
#include <atomic_stub.h>

/* In some places we need a type that is almost the same as base type
   T, but

   - returns a pointer to T in evaluation context
   - can't be assigned to

   T can be a type or an expression.
*/
#define __atyp(T) __typeof__(__typeof__(T)[1])

/* To evaluate expressions we sometimes need temporaries of that type
   with a certain value. */
#define __atmp(T, V) (__atyp(T)){ [0] = (V), }

/* When evaluating lvalues in gcc's compound expressions to return a
   value, we want to take care that these lvalues can't be
   accidentally be subject to the & operator. Force it to be an
   rvalue. */
#define __aret(V) (1 ? (V) : (V))


#ifdef __ATOMIC_FORCE_SYNC
#include <atomic_gcc_sync.h>
#elif defined(__clang__)
#include <atomic_clang_c11.h>
#else
#include <atomic_gcc_atomic.h>
#endif

#include <atomic_types.h>

#endif
