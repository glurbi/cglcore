EXECUTABLES = tutorial01\
			  tutorial02\
			  tutorial03\
			  tutorial04\
			  tutorial05\
			  tutorial06\
			  tutorial07\
			  tutorial08\
			  tutorial09

all: $(EXECUTABLES)

tutorial01: tutorial01.cpp
	g++ -Wall -g -std=c++0x -o tutorial01 tutorial01.cpp -lX11 -lGL -lGLEW

tutorial02: tutorial02.cpp
	g++ -Wall -g -std=c++0x -o tutorial02 tutorial02.cpp -lX11 -lGL -lGLEW
	
tutorial03: tutorial03.cpp
	g++ -Wall -g -std=c++0x -o tutorial03 tutorial03.cpp -lX11 -lGL -lGLEW -lSDL
	
tutorial04: tutorial04.cpp
	g++ -Wall -g -std=c++0x -o tutorial04 tutorial04.cpp -lX11 -lGL -lGLEW -lSDL
	
tutorial05: tutorial05.cpp
	g++ -Wall -g -std=c++0x $(shell pkg-config --cflags gtk+-2.0 gtkgl-2.0 gtkglext-1.0) \
	    -o tutorial05 tutorial05.cpp $(shell pkg-config --libs gtk+-2.0 gtkgl-2.0 gtkglext-1.0) -lGLEW
	
tutorial06: tutorial06.cpp
	g++ -Wall -g -std=c++0x $(shell pkg-config --cflags gtk+-2.0 gtkgl-2.0 gtkglext-1.0) \
	    -o tutorial06 tutorial06.cpp $(shell pkg-config --libs gtk+-2.0 gtkgl-2.0 gtkglext-1.0) -lGLEW

tutorial07: tutorial07.cpp
	g++ -Wall -g -std=c++0x -o tutorial07 tutorial07.cpp -lX11 -lGL -lGLEW -lSDL
	
tutorial08: tutorial08.cpp
	g++ -Wall -g -std=c++0x -o tutorial08 tutorial08.cpp -lX11 -lGL -lGLEW -lSDL
	
tutorial09: tutorial09.cpp
	g++ -Wall -g -std=c++0x -o tutorial09 tutorial09.cpp -lX11 -lGL -lGLEW -lSDL

clean:
	-rm $(EXECUTABLES)
