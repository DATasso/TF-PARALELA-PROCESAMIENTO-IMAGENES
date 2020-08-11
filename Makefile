CXX = mpic++
CXXFLAGS = -std=gnu++14 -g
MKDIR = mkdir -p

LIBS = `pkg-config --cflags --libs opencv4`

directorios:
	$(MKDIR) build dist

algo.o: directorios algo.cpp
	$(CXX) $(CXXFLAGS) -c algo.cpp -o build/algo.o $(LIBS) 

all: clean algo.o
	$(CXX) $(CXXFLAGS) -o dist/programa build/algo.o  $(LIBS) 
	rm -fr build

clean:
	rm -fr *.o a.out programa dist build

.DEFAULT_GOAL := all
