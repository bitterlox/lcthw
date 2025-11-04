CFLAGS=-Wall -g -DNDEBUG

all: ex22 ex22_main

ex22_main:
	gcc ./ex22_main.c ./build/ex22.o -o ex22_main

ex22:
	gcc -c -o ./build/ex22.o ex22.c

clean:
	rm -f ex22
