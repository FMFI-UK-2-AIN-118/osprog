.PHONY: default clean distclean test
default: program libnumber.so
program: program.o libnumber.so
	gcc -o $@ program.o -L. -lnumber
program.o: program.c number.h
	gcc -c -fPIC -o $@ program.c
libnumber.so: number.o
	gcc -shared -o $@ number.o
number.o: number.c number.h
	gcc -c -fPIC -o $@ number.c
clean:
	rm -f *.o
distclean: clean
	rm -f program libnumber.so
test:
	@LD_LIBRARY_PATH=$(PWD) ./program; echo $$? 
