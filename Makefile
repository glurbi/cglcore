INCLUDE = -I/usr/include/
LIBDIR  = -L/usr/X11R6/lib 
COMPILERFLAGS = -Wall -g
CC = gcc
CFLAGS = $(COMPILERFLAGS) $(INCLUDE)
LIBRARIES = -lX11 -lglut -lGL -lGLEW

all: tutorial02

tutorial02 : tutorial02.c
	$(CC) $(CFLAGS) -o tutorial02 $(LIBDIR) $(LIBRARIES) tutorial02.c  

clean:
	-rm *.o
	-rm tutorial02
