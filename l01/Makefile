.PHONY: default
default: program

program.o: program.c number.h
	gcc -c -o program.o -fPIC program.c
	
number.o: number.c number.h
	gcc -c -o number.o -fPIC number.c
	gcc -o libnumber.so -shared number.o

program: number.o program.o
	gcc -o program program.o -L. -lnumber

.PHONY: clean
clean: 
	rm -rf *.o

.PHONY: distclean
distclean: 
	rm -rf *.o *.so program

.PHONY: test
test: program
	@export LD_LIBRARY_PATH=${PWD} && ./program || echo $$?
