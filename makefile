GCC=gcc

MPICC=mpicc

CFLAGS=-o

DEPS_PROG=bubblesort_parallel.c

program:
	$(MPICC) $(DEPS_PROG) $(CFLAGS) a.out

generator:
	$(GCC) input_generator.c $(CFLAGS) generator.out

iterate:
	for number in $$(seq 1 1 10); do \
		mpiexec -n $$number ./a.out input_10000.txt ; \
		((number=$$number+1)) ; \
	done

clean:
	rm -rf *.out *.o