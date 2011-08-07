INCLUDE = -I/usr/include/ -I.
LIBDIR  = -L/usr/X11R6/lib
COMPILERFLAGS = -Wall -g -pg
CC = g++
CFLAGS = $(COMPILERFLAGS) $(INCLUDE)
LIBRARIES = -lX11 -lglut -lGL -lGLEW -lpng
TARGETDIR = bin
COMMON = $(TARGETDIR)/matrices.o $(TARGETDIR)/files.o $(TARGETDIR)/glutils.o

all: init common tutorial01 tutorial02 tutorial03 tutorial04 tutorial05

init:
	mkdir -p $(TARGETDIR)

common: matrices.cpp matrices.h files.cpp files.h glutils.cpp glutils.h
	$(CC) $(CFLAGS) -c $(LIBDIR) $(LIBRARIES) matrices.cpp -o $(TARGETDIR)/matrices.o
	$(CC) $(CFLAGS) -c $(LIBDIR) $(LIBRARIES) files.cpp -o $(TARGETDIR)/files.o
	$(CC) $(CFLAGS) -c $(LIBDIR) $(LIBRARIES) glutils.cpp -o $(TARGETDIR)/glutils.o

tutorial01 : tutorial01.cpp
	$(CC) $(CFLAGS) -o $(TARGETDIR)/tutorial01 $(LIBDIR) $(LIBRARIES) tutorial01.cpp
	  
tutorial02 : tutorial02.cpp
	$(CC) $(CFLAGS) -o $(TARGETDIR)/tutorial02 $(LIBDIR) $(LIBRARIES) tutorial02.cpp

tutorial03 : tutorial03.cpp
	$(CC) $(CFLAGS) -o $(TARGETDIR)/tutorial03 $(LIBDIR) $(LIBRARIES) tutorial03.cpp $(COMMON)

tutorial04 : tutorial04.cpp
	$(CC) $(CFLAGS) -o $(TARGETDIR)/tutorial04 $(LIBDIR) $(LIBRARIES) tutorial04.cpp $(COMMON)

tutorial05 : tutorial05.cpp
	$(CC) $(CFLAGS) -o $(TARGETDIR)/tutorial05 $(LIBDIR) $(LIBRARIES) tutorial05.cpp $(COMMON)
	
clean:
	-rm -rf bin
	-rm gmon.out
	
