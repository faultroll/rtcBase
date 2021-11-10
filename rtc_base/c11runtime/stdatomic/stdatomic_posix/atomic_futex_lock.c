#include "pthread_impl.h"
#include "stdatomic-impl.h"
#include <time.h>
#include <stdio.h>
#include <math.h>

/* The HO bit. */
static unsigned const lockbit = (UINT_MAX/2u)+1u;
/* The HO and the LO bits. */
static unsigned const contrib = (UINT_MAX/2u)+2u;

static size_t g_total = 0;
static size_t g_slow = 0;
static size_t g_futex = 0;
static size_t g_wouldblock = 0;
static size_t g_spin = 0;
static size_t g_slow_wake = 0;

#ifdef BENCH
# define ACCOUNT(X, V) do { (X) += (V); } while(0)
#else
# define ACCOUNT(X, V) do { } while(0)
#endif

#define RAW_OUT_(F, P, T, FR, ...)              \
do {                                            \
  FILE* _f = (F);                               \
  T _v[] = { __VA_ARGS__ };                     \
  size_t _l = sizeof _v/sizeof _v[0];           \
  char _s[] = # __VA_ARGS__;                    \
  for (char* _p = _s; *_p; ++_p)                \
    if (*_p == ' ') *_p = '\t';                 \
  fprintf(_f, "#" P ":\t%s\n", _s);             \
  fprintf(_f, "#" P ":\t" FR, _v[0]);           \
  for (size_t i = 1; i < _l; ++i) {             \
    fprintf(_f, ",\t" FR, _v[i]);               \
  }                                             \
  fputc('\n', _f);                              \
 } while (0);

#define RAW_OUT(...) RAW_OUT_(__VA_ARGS__)

void atomic_summarize(FILE* out) {
  fprintf(stderr, "summary of usage of atomic lock functions\n");
  fprintf(stderr, "\tslow path, lock:\t%zu\n", g_total);
  fprintf(stderr, "\touter iterations:\t%zu\t(%.4f per slow lock)\n", g_slow, g_slow*1.0/g_total);
  fprintf(stderr, "\tinner iterations:\t%zu spin\t(%.4f per outer)\n", g_spin, g_spin*1.0/g_slow);
  fprintf(stderr, "\tinner iterations:\t%zu futex\t(%.4f per outer)\n", g_futex, g_futex*1.0/g_slow);
  fprintf(stderr, "\tfailed futex:\t\t%zu\t\t(%.4f per futex)\n", g_wouldblock, g_wouldblock*1.0/g_futex);
  fprintf(stderr, "\tslow path, unlock:\t%zu\t(%.4f per slow lock)\n", g_slow_wake, g_slow_wake*1.0/g_total);
  if (out)
    RAW_OUT(out, "lock statistic", size_t, "%zu",
            g_total, g_slow, g_spin, g_futex, g_wouldblock, g_slow_wake);
}

/* This is just a heuristic. We assume that one round of spinning is
   10 times faster than a failed call to futex_wait. */
static unsigned spins_max = 10;

void __impl_mut_lock_slow(_Atomic(unsigned)* loc, unsigned val, int mo)
{
#ifdef BENCH
  size_t slow = 0;
  size_t futex = 0;
  size_t wouldblock = 0;
  size_t spin = 0;
#endif
  unsigned const sm = spins_max;
  unsigned spins = sm;
  int mof = mo == memory_order_relaxed ? memory_order_relaxed : memory_order_consume;
  /* A first lock acquisition loop. This is to capture the case medium
     congestion. We still hope to be able to set the HO and LO bit of
     *loc all at once. */
  for (;;) {
    /* be optimistic and hope that the lock has been released */
    unsigned des;
    if (val & lockbit) {
      des = val;
      val -= contrib;
    } else {
      des = val + contrib;
    }
    if (atomic_compare_exchange_strong_explicit(loc, &val, des, mo, mof)) {
      ACCOUNT(spin, sm-spins);
      ACCOUNT(slow, 1);
      goto FINISH;
    }
    --spins;
    if (!spins) break;
  }
  ACCOUNT(spin, sm);
  val = 1+atomic_fetch_add_explicit(loc, 1, memory_order_relaxed);
  if (!(val & lockbit)) goto BIT_UNSET;
  /* The lock acquisition loop. This has been designed such that the
     only possible change that is done inside that loop is setting
     the lock bit. This has a double objective. First all atomic
     operations are expensive and doing a pair of ++ and -- inside
     the loop would just waste memory bandwidth. Then, less changes
     to the count, means that other threads that are inside this
     same loop are less perturbed. */
  for (;;) {
    /* The lock bit is set by someone else, spin until it is unset. */
    spins = sm;
    for (;;) {
      /* be optimistic and hope that the lock has been released */
      unsigned des = val-1;
      val -= contrib;
      if (atomic_compare_exchange_strong_explicit(loc, &val, des, mo, mof)) {
        ACCOUNT(spin, sm-spins);
        ACCOUNT(slow, 1);
        goto FINISH;
      }
      if (!(val & lockbit)) goto BIT_UNSET;
      --spins;
      if (!spins) break;
    }
    /* The same inner loop as before, but with futex wait instead of
       a_spin. */
    for (;;) {
      ACCOUNT(futex, 1);
      if (__syscall(SYS_futex, loc, FUTEX_WAIT|FUTEX_PRIVATE, val, 0) == -EWOULDBLOCK)
        ACCOUNT(wouldblock, 1);
      /* be optimistic and hope that the lock has been released */
      unsigned des = val-1;
      val -= contrib;
      if (atomic_compare_exchange_strong_explicit(loc, &val, des, mo, mof)) {
        ACCOUNT(spin, sm-spins);
        ACCOUNT(slow, 1);
        goto FINISH;
      }
      if (!(val & lockbit)) goto BIT_UNSET;
    }
    /* The lock bit isn't set, try to acquire it. */
  BIT_UNSET:
    ACCOUNT(spin, sm-spins);
    ACCOUNT(slow, 1);
    for (unsigned des = val|lockbit;;des = val|lockbit) {
      if (atomic_compare_exchange_strong_explicit(loc, &val, des, mo, mof))
        goto FINISH;
      if (val & lockbit) break;
    }
  }
 FINISH:
  /* We hold the lock here, so these additions are fine as long we
     only test with one particular lock. But only then, be careful. */
  ACCOUNT(g_total, 1);
  ACCOUNT(g_slow, slow);
  ACCOUNT(g_futex, futex);
  ACCOUNT(g_wouldblock, wouldblock);
  ACCOUNT(g_spin, spin);
  return;
}

/* Even though this might look costly, this isn't. All our attempts to
   short cut this lead to a slowdown of the application. */
void __impl_mut_unlock_slow(_Atomic(unsigned)* loc)
{
  __syscall(SYS_futex, loc, FUTEX_WAKE|FUTEX_PRIVATE, 1);
  ACCOUNT(g_slow_wake, 1);
}

static
double timespecdiff(struct timespec* end, struct timespec* start) {
  double ret = end->tv_sec - start->tv_sec;
  if (end->tv_nsec >= start->tv_nsec)
    ret += (end->tv_nsec - start->tv_nsec)*1E-9;
  else
    ret += (start->tv_nsec - end->tv_nsec)*1E-9 - 1.0;
  /* fprintf(stderr, "bench start %lld %ld\n", start->tv_sec, start->tv_nsec); */
  /* fprintf(stderr, "bench end %lld %ld\n", end->tv_sec, end->tv_nsec); */
  /* fprintf(stderr, "difference %g\n", ret); */
  return ret;
}

void atomic_calibrate(FILE* out) {
  _Atomic(unsigned) loc = ATOMIC_VAR_INIT(42u);
  unsigned val = 0;
  size_t i;
  enum { iterI = 1000, };
  size_t j;
  enum { iterJ = 10, };
  double futex_time[iterJ] = { 0 };
  double wake_time[iterJ] = { 0 };
  double spin_time[iterJ] = { 0 };
  struct timespec all_start, all_end,
    futex_start, futex_end,
    wake_start, wake_end,
    spin_start, spin_end;

  clock_gettime(CLOCK_MONOTONIC, &all_start);

  for (j = 0; j < iterJ; ++j) {
    /* Determine the cost of mis-predicted futex_wait */
    clock_gettime(CLOCK_MONOTONIC, &futex_start);
    for (i = 0; i < iterI; ++i) {
    SHORTCUT0:
      __syscall(SYS_futex, &loc, FUTEX_WAIT|FUTEX_PRIVATE, val, 0);
      /* Change the value in the same way as in the real loop. */
      unsigned des = val-1;
      val -= contrib;
      /* have the same conditionals as in the original, only that we
         know that this will never trigger */
      if (atomic_compare_exchange_strong_explicit(&loc, &val, des, memory_order_acq_rel, memory_order_consume))
        goto SHORTCUT0;
      /* Make sure that val never is correct. */
      val = 0;
    }
    clock_gettime(CLOCK_MONOTONIC, &futex_end);

    /* Determine the cost of mis-predicted futex_wake */
    clock_gettime(CLOCK_MONOTONIC, &wake_start);
    for (i = 0; i < iterI; ++i) {
      __syscall(SYS_futex, &loc, FUTEX_WAKE|FUTEX_PRIVATE, 1);
    }
    clock_gettime(CLOCK_MONOTONIC, &wake_end);

    /* Determine the cost of mis-predicted round of spinning */
    clock_gettime(CLOCK_MONOTONIC, &spin_start);
    for (i = 0; i < iterI; ++i) {
    SHORTCUT1:;
      /* Change the value in the same way as in the real loop. */
      unsigned des = val-1;
      val -= contrib;
      /* have the same conditionals as in the original, only that we
         know that this will never trigger */
      if (atomic_compare_exchange_strong_explicit(&loc, &val, des, memory_order_acq_rel, memory_order_consume))
        goto SHORTCUT1;
      /* Make sure that val never is correct. */
      val = 0;
    }
    clock_gettime(CLOCK_MONOTONIC, &spin_end);

    futex_time[j] = timespecdiff(&futex_end, &futex_start)/iterI;
    wake_time[j] = timespecdiff(&wake_end, &wake_start)/iterI;
    spin_time[j] = timespecdiff(&spin_end, &spin_start)/iterI;
  }

  double ft1 = 0;
  double fmin= futex_time[0];
  double fmax= futex_time[0];
  double wt1 = 0;
  double wmin= futex_time[0];
  double wmax= futex_time[0];
  double st1 = 0;
  double smin= spin_time[0];
  double smax= spin_time[0];

  for (j = 0; j < iterJ; ++j) {
    ft1 += futex_time[j];
    if (futex_time[j] < fmin) fmin = futex_time[j];
    if (fmax < futex_time[j]) fmax = futex_time[j];
    wt1 += wake_time[j];
    if (wake_time[j] < wmin) wmin = wake_time[j];
    if (wmax < wake_time[j]) wmax = wake_time[j];
    st1 += spin_time[j];
    if (spin_time[j] < smin) smin = spin_time[j];
    if (smax < spin_time[j]) smax = spin_time[j];
  }
  /* Throw away the extreme points of our measurements. */
  double futex_wait_fail_mean = (ft1-fmin-fmax)/(iterJ-2);
  double futex_wake_mean = (wt1-wmin-wmax)/(iterJ-2);
  double spin_mean = (st1-smin-smax)/(iterJ-2);

  ft1 = 0;
  wt1 = 0;
  st1 = 0;

  double factor = 2.1;
  double estimate = futex_wait_fail_mean*factor/spin_mean;
  if (estimate < 5.0) spins_max = 5u;
  else {
    if (100.0 < estimate) spins_max = 100u;
    else spins_max = estimate;
  }
  clock_gettime(CLOCK_MONOTONIC, &all_end);

  double ft2 = 0;
  double wt2 = 0;
  double st2 = 0;

  for (j = 0; j < iterJ; ++j) {
    double fd1 = futex_time[j] - futex_wait_fail_mean;
    ft1 += fd1;
    ft2 += fd1*fd1;
    double wd1 = wake_time[j] - futex_wake_mean;
    wt1 += wd1;
    wt2 += wd1*wd1;
    double sd1 = spin_time[j] - spin_mean;
    st1 += sd1;
    st2 += sd1*sd1;
  }

  double futex_wait_fail_dev = sqrt((ft2 - (ft1*ft1)/iterJ)/(iterJ-1));
  double futex_wake_dev = sqrt((wt2 - (wt1*wt1)/iterJ)/(iterJ-1));
  double spin_dev = sqrt((st2 - (st1*st1)/iterJ)/(iterJ-1));

  if (out) {
    fprintf(stderr, "end of calibration after %g sec:\n",
            timespecdiff(&all_end, &all_start));
    fprintf(stderr, "\tspin\t= %g (+%g)\n", spin_mean, spin_dev);
    fprintf(stderr, "\tfail\t= %g (+%g)\n", futex_wait_fail_mean, futex_wait_fail_dev);
    fprintf(stderr, "\twake\t= %g (+%g)\n", futex_wake_mean, futex_wake_dev);
    fprintf(stderr, "\tspins_max\t= %u (%g)\n", spins_max, estimate);
    RAW_OUT(out, "lock calibration", double, "%g",
            futex_wait_fail_mean, futex_wait_fail_dev,
            futex_wake_mean, futex_wake_dev,
            spin_mean, spin_dev);
  }
}

#pragma weak atomic_inject
void atomic_inject(void) {
  /* empty */
}

_Thread_local _Bool atomic_faulty = 0;
