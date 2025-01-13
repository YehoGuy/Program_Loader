all: loader

loader: loader.o start.o
	ld -o loader loader.o start.o -m32 -nostdlib -T linking_script

loader.o: loader.c
	gcc -g -Wall -m32 -c -o loader.o loader.c

start.o: start.s
	gcc -g -Wall -m32 -c -o start.o start.s

.PHONY: clean

clean:
	rm -f *.o loader
