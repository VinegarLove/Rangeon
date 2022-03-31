CC=g++
CFLAGS=-I.

rangeon: rangeon.o builder.o
	$(CC) -o rangeon rangeon.o builder.o

.PHONY: clean

clean:
	rm ./*.o ./rangeon
