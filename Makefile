INCLUDE = -I/usr/include/ -I.
LIBDIR  = -L/usr/X11R6/lib
COMPILERFLAGS = -Wall -g -std=c99
CC = gcc
CFLAGS = $(COMPILERFLAGS) $(INCLUDE)
LIBRARIES = -lX11 -lglut -lGL -lGLEW
COMMON = matrices.o files.o

all: tutorial01 tutorial02 tutorial03

common: matrices.c matrices.h files.c files.h
	$(CC) $(CFLAGS) -c $(LIBDIR) $(LIBRARIES) matrices.c
	$(CC) $(CFLAGS) -c $(LIBDIR) $(LIBRARIES) files.c

tutorial01 : tutorial01.c
	$(CC) $(CFLAGS) -o tutorial01 $(LIBDIR) $(LIBRARIES) tutorial01.c
	  
tutorial02 : tutorial02.c
	$(CC) $(CFLAGS) -o tutorial02 $(LIBDIR) $(LIBRARIES) tutorial02.c

tutorial03 : common tutorial03.c
	$(CC) $(CFLAGS) -o tutorial03 $(LIBDIR) $(LIBRARIES) tutorial03.c $(COMMON)

clean:
	-rm *.o
	-rm tutorial01
	-rm tutorial02
	-rm tutorial03
	