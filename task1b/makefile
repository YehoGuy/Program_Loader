all: loader

loader: loader.o
	gcc -g -Wall -m32 -o loader loader.o

loader.o: loader.c
	gcc -g -Wall -m32 -c -o loader.o loader.c

.PHONY: clean

clean:
	rm -f *.o loader
