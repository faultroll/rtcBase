TARGET = libatomic.a

MUSL = ${HOME}/build/musl

SOURCES :=					\
	atomic_flag.c				\
	atomic_fence.c

GENERICS :=					\
	atomic_generic.c			\
	atomic_generic_1.c			\
	atomic_generic_2.c			\
	atomic_generic_4.c			\
	atomic_generic_8.c			\
	atomic_generic_16.c

FUTEX := atomic_futex_lock.c

OBJECTS := ${SOURCES:.c=.o} ${GENERICS:.c=.o} lock-repl.o
ASSEMBS := ${SOURCES:.c=.s} ${GENERICS:.c=.s}
DEPENDS := ${OBJECTS:.o=.dep}
MEMBERS := ${patsubst %.o, ${TARGET}(%.o),${OBJECTS}}
FUTEXO  := ${FUTEX:.c=.o}

EFUNCS =					\
	load					\
	store					\
	exchange				\
	compare_exchange

RFUNCS = load_1					\
	store_1					\
	exchange_1				\
	compare_exchange_1			\
	fetch_add_1				\
	fetch_sub_1				\
	fetch_and_1				\
	fetch_xor_1				\
	fetch_nand_1				\
	fetch_or_1				\
	add_fetch_1				\
	sub_fetch_1				\
	and_fetch_1				\
	xor_fetch_1				\
	nand_fetch_1				\
	or_fetch_1				\
	load_2					\
	store_2					\
	exchange_2				\
	compare_exchange_2			\
	fetch_add_2				\
	fetch_sub_2				\
	fetch_and_2				\
	fetch_xor_2				\
	fetch_nand_2				\
	fetch_or_2				\
	add_fetch_2				\
	sub_fetch_2				\
	and_fetch_2				\
	xor_fetch_2				\
	nand_fetch_2				\
	or_fetch_2				\
	load_4					\
	store_4					\
	exchange_4				\
	compare_exchange_4			\
	fetch_add_4				\
	fetch_sub_4				\
	fetch_and_4				\
	fetch_xor_4				\
	fetch_nand_4				\
	fetch_or_4				\
	add_fetch_4				\
	sub_fetch_4				\
	and_fetch_4				\
	xor_fetch_4				\
	nand_fetch_4				\
	or_fetch_4				\
	load_8					\
	store_8					\
	exchange_8				\
	compare_exchange_8			\
	fetch_add_8				\
	fetch_sub_8				\
	fetch_and_8				\
	fetch_xor_8				\
	fetch_nand_8				\
	fetch_or_8				\
	add_fetch_8				\
	sub_fetch_8				\
	and_fetch_8				\
	xor_fetch_8				\
	nand_fetch_8				\
	or_fetch_8				\
	load_16					\
	store_16				\
	exchange_16				\
	compare_exchange_16			\
	fetch_add_16				\
	fetch_sub_16				\
	fetch_and_16				\
	fetch_xor_16				\
	fetch_nand_16				\
	fetch_or_16                             \
	add_fetch_16				\
	sub_fetch_16				\
	and_fetch_16				\
	xor_fetch_16				\
	nand_fetch_16				\
	or_fetch_16

SFUNCS = bool_compare_and_swap_1		\
	val_compare_and_swap_1			\
	fetch_and_add_1				\
	fetch_and_sub_1				\
	fetch_and_and_1				\
	fetch_and_xor_1				\
	fetch_and_or_1				\
	add_and_fetch_1				\
	sub_and_fetch_1				\
	and_and_fetch_1				\
	xor_and_fetch_1				\
	or_and_fetch_1				\
	bool_compare_and_swap_2			\
	val_compare_and_swap_2			\
	fetch_and_add_2				\
	fetch_and_sub_2				\
	fetch_and_and_2				\
	fetch_and_xor_2				\
	fetch_and_or_2				\
	add_and_fetch_2				\
	sub_and_fetch_2				\
	and_and_fetch_2				\
	xor_and_fetch_2				\
	or_and_fetch_2				\
	bool_compare_and_swap_4			\
	val_compare_and_swap_4			\
	fetch_and_add_4				\
	fetch_and_sub_4				\
	fetch_and_and_4				\
	fetch_and_xor_4				\
	fetch_and_or_4				\
	add_and_fetch_4				\
	sub_and_fetch_4				\
	and_and_fetch_4				\
	xor_and_fetch_4				\
	or_and_fetch_4				\
	bool_compare_and_swap_8			\
	val_compare_and_swap_8			\
	fetch_and_add_8				\
	fetch_and_sub_8				\
	fetch_and_and_8				\
	fetch_and_xor_8				\
	fetch_and_or_8				\
	add_and_fetch_8				\
	sub_and_fetch_8				\
	and_and_fetch_8				\
	xor_and_fetch_8				\
	or_and_fetch_8				\
	bool_compare_and_swap_16		\
	val_compare_and_swap_16			\
	fetch_and_add_16			\
	fetch_and_sub_16			\
	fetch_and_and_16			\
	fetch_and_xor_16			\
	fetch_and_or_16                         \
	add_and_fetch_16			\
	sub_and_fetch_16			\
	and_and_fetch_16			\
	xor_and_fetch_16			\
	or_and_fetch_16

.INTERMEDIATE :  ${GENERICS:.c=-tmp.o} redefine_syms.txt

OBJOPTS :=  --redefine-syms=redefine_syms.txt

COPTS ?= -O3 -march=native

CFLAGS ?= -Wall -Wno-shadow -isystem `pwd`  -fno-common -fdata-sections -ffunction-sections ${COPTS}

CFLAGS := ${CFLAGS} ${CONFIG}

target : libatomic.a

ifeq (${MAKELEVEL},0)
# compile all objects in parallel mode and then recurse
libatomic.a : ${OBJECTS}
	${MAKE} libatomic.a
else
# now, addition of the archive members should be done sequentially
.NOTPARALLEL :
libatomic.a : ${MEMBERS}
endif

libstdatomic.a : libatomic.a ${MUSL}/src/stdatomic/${FUTEXO}
	cp libatomic.a libstdatomic.a
	${AR} rc libstdatomic.a ${MUSL}/src/stdatomic/${FUTEXO}

redefine_syms.txt : Makefile
	@echo -n ${RFUNCS} | sed 's/\([a-z0-9_][a-z0-9_]*\) */__impl_\1 __atomic_\1\n/g'  > $@
	@echo -n ${SFUNCS} | sed 's/\([a-z0-9_][a-z0-9_]*\) */__impl_\1 __sync_\1\n/g'    >>$@
	@echo -n ${EFUNCS} | sed 's/\([a-z0-9_][a-z0-9_]*\) */__impl_\1 __atomic_\1\n/g'  >>$@

atomic_pragma.h : redefine_syms.txt Makefile
	sed 's/\(.*\)/#pragma redefine_extname \1/1' $< > $@

%.s : %.c
	${CC} -S ${CFLAGS} $*.c

%.dep : %.c
	@${CC} -M ${CFLAGS} -o $@ $*.c
	@${CC} -M ${CFLAGS} $*.c | sed 's/[.]o\>/.s/g' >> $@

clean :
	rm -fr ${OBJECTS} ${DEPENDS} ${ASSEMBS} libatomic.a

beautify :
	astyle --options=.astylerc *.c *.h

musl : ${SOURCES} ${GENERICS} ${FUTEX} atomic_pragma.h
	-mkdir ${MUSL}/src/stdatomic 2> /dev/null || true
	cp ${SOURCES} ${GENERICS} ${FUTEX} ${MUSL}/src/stdatomic/
	cp atomic*.h  ${MUSL}/include
	cp stdatomic-impl.h ${MUSL}/include/stdatomic.h

#	cp atomic*.h  stdatomic-impl.h ${MUSL}/src/internal/

${MUSL}/src/stdatomic/${FUTEXO} : musl
	make -C ${MUSL} $@

-include ${DEPENDS}
