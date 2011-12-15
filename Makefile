INCLUDE = -I/usr/include/ -I.
LIBDIR  = -L/usr/X11R6/lib
COMPILERFLAGS = -Wall -g -pg -std=c++0x
CXX = g++
CXXFLAGS = $(COMPILERFLAGS) $(INCLUDE)
LIBRARIES = -lX11 -lglut -lGL -lGLEW -lpng
EXECUTABLES = tutorial01 tutorial02 tutorial03 tutorial04 tutorial05 tutorial06

all: $(EXECUTABLES)

#.c.o: 
#	$(CXX) $(CFLAGS) $(LIBDIR) –c $(.SOURCE) 

matrices.o: matrices.hpp
files.o: files.h
glutils.o: glutils.h
glxwindow.o: glwindow.hpp glxwindow.hpp

tutorial01: tutorial01.cpp
	g++ -Wall -g -std=c++0x -o tutorial01 tutorial01.cpp -lX11 -lGL -lGLEW

tutorial02: tutorial02.cpp
	g++ -Wall -g -std=c++0x -o tutorial02 tutorial02.cpp -lX11 -lGL -lGLEW
	
tutorial03: tutorial03.cpp
	g++ -Wall -g -std=c++0x -o tutorial03 tutorial03.cpp -lX11 -lGL -lGLEW -lSDL
	
tutorial04: tutorial04.cpp
	g++ -Wall -g -std=c++0x -o tutorial04 tutorial04.cpp -lX11 -lGL -lGLEW -lSDL
	
tutorial05: tutorial05.o matrices.o files.o glutils.o utils.h
	$(CXX) $(CXXFLAGS) -o $@ tutorial05.o matrices.o files.o glutils.o $(LIBRARIES)
	
tutorial06: tutorial06.o matrices.o files.o glutils.o utils.h torus.o torus.h
	$(CXX) $(CXXFLAGS) -o $@ tutorial06.o matrices.o files.o glutils.o torus.o $(LIBRARIES)

clean:
	-rm *.o
	-rm gmon.out
	-rm $(EXECUTABLES)
