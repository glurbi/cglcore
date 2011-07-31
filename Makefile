INCLUDE = -I/usr/include/
LIBDIR  = -L/usr/X11R6/lib 
COMPILERFLAGS = -Wall -g -std=c99
CC = gcc
CFLAGS = $(COMPILERFLAGS) $(INCLUDE)
LIBRARIES = -lX11 -lglut -lGL -lGLEW

all: tutorial01 tutorial02

tutorial01 : tutorial01.c
	$(CC) $(CFLAGS) -o tutorial01 $(LIBDIR) $(LIBRARIES) tutorial01.c
	  
tutorial02 : tutorial02.c
	$(CC) $(CFLAGS) -o tutorial02 $(LIBDIR) $(LIBRARIES) tutorial02.c  

clean:
	-rm *.o
	-rm tutorial01
	-rm tutorial02
	