INCLUDE = -I/usr/include/ -I.
LIBDIR  = -L/usr/X11R6/lib
COMPILERFLAGS = -Wall -g
CC = g++
CFLAGS = $(COMPILERFLAGS) $(INCLUDE)
LIBRARIES = -lX11 -lglut -lGL -lGLEW
COMMON = matrices.o files.o glutils.o

all: tutorial01 tutorial02 tutorial03

common: matrices.cpp matrices.h files.cpp files.h glutils.cpp glutils.h
	$(CC) $(CFLAGS) -c $(LIBDIR) $(LIBRARIES) matrices.cpp
	$(CC) $(CFLAGS) -c $(LIBDIR) $(LIBRARIES) files.cpp
	$(CC) $(CFLAGS) -c $(LIBDIR) $(LIBRARIES) glutils.cpp

tutorial01 : tutorial01.cpp
	$(CC) $(CFLAGS) -o tutorial01 $(LIBDIR) $(LIBRARIES) tutorial01.cpp
	  
tutorial02 : tutorial02.cpp
	$(CC) $(CFLAGS) -o tutorial02 $(LIBDIR) $(LIBRARIES) tutorial02.cpp

tutorial03 : common tutorial03.cpp
	$(CC) $(CFLAGS) -o tutorial03 $(LIBDIR) $(LIBRARIES) tutorial03.cpp $(COMMON)

clean:
	-rm *.o
	-rm tutorial01
	-rm tutorial02
	-rm tutorial03
	
