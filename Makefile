CC=gcc

#Compile with: 
#-g = debug
#-m32 = 32bit x86 executable
CFLAGS=-Wall -g -m32 -fno-pic -std=c99

all: hello

hello: main.o
	$(CC) $(CFLAGS) main.o -o hello

main.o: main.c
	$(CC) -c $(CFLAGS) main.c

clean:
	rm *.o hello


