# Makefile for encode and decode

CC = gcc $(CFLAGS)
CFLAGS = -Wall -Wextra -Wpedantic -std=c99
OBJECTS = queue.o stack.o huffman.o

.PHONY: all
all: encode decode

encode: $(OBJECTS) encode.o
	$(CC) $(OBJECTS) encode.o -o encode

decode: $(OBJECTS) decode.o
	$(CC) $(OBJECTS) decode.o -o decode

encode.o: encode.c
	$(CC)  -c -g encode.c

decode.o: decode.c
	$(CC) -c -g decode.c

huffman.o: huffman.c
	$(CC)  -c -g huffman.c

queue.o: queue.c
	$(CC)  -c -g queue.c

stack.o: stack.c
	$(CC)  -c -g stack.c

.PHONY: clean
clean:
	rm -f $(OBJECTS) encode.o decode.o encode decode
