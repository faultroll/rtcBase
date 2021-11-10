
1. c11 |atomic| and |thread|, single header file
2. for my own use (only part of functions is implemented), as simple as it can be; atomic function are implemented as macros and not same as c11 |atomic| (eg. cas)
3. add single thread situation (eg. runs on stm32), |atomic| only change value, |thread| using coroutines (so use |thrd_yield| when using threads)
4. only win/posix(linux)/embedded(stm32/linux/rtos/...) env needed

reference
1. c11 win: https://github.com/cdschreiber/c11
2. stdatomic: https://gitlab.inria.fr/gustedt/stdatomic
3. threads: https://github.com/tinycthread/tinycthread
4. coroutine: https://github.com/zserge/pt
