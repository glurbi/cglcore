INCLUDE = -I/usr/include/ -I.
LIBDIR  = -L/usr/X11R6/lib
COMPILERFLAGS = -Wall -g -pg
CXX = g++
CXXFLAGS = $(COMPILERFLAGS) $(INCLUDE)
LIBRARIES = -lX11 -lglut -lGL -lGLEW -lpng
EXECUTABLES = tutorial01 tutorial02 tutorial03 tutorial04 tutorial05 tutorial06

all: $(EXECUTABLES)

.c.o: 
	$(CXX) $(CFLAGS) $(LIBDIR) –c $(.SOURCE) 

matrices.o: matrices.hpp
files.o: files.h
glutils.o: glutils.h
glxwindow.o: glwindow.hpp glxwindow.hpp

tutorial01: tutorial01.o
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBRARIES)

tutorial02: tutorial02.o
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBRARIES)
	
tutorial03: tutorial03.o matrices.o files.o glutils.o
	$(CXX) $(CXXFLAGS) -o $@ tutorial03.o matrices.o files.o glutils.o $(LIBRARIES) 
	
tutorial04: tutorial04.o matrices.o files.o glutils.o utils.h
	$(CXX) $(CXXFLAGS) -o $@ tutorial04.o matrices.o files.o glutils.o $(LIBRARIES)
	
tutorial05: tutorial05.o matrices.o files.o glutils.o utils.h
	$(CXX) $(CXXFLAGS) -o $@ tutorial05.o matrices.o files.o glutils.o $(LIBRARIES)
	
tutorial06: tutorial06.o matrices.o files.o glutils.o utils.h torus.o torus.h
	$(CXX) $(CXXFLAGS) -o $@ tutorial06.o matrices.o files.o glutils.o torus.o $(LIBRARIES)

clean:
	-rm *.o
	-rm gmon.out
	-rm $(EXECUTABLES)
