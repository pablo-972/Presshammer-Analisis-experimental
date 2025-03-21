CXX := g++
CFLAGS := -std=c++11 -g -O3
MAINSOURCES := main.cpp Hist.cpp Mapping.cpp
MAINOBJS := $(MAINSOURCES:.cpp=.o)


all: demo-main

demo-main: $(MAINOBJS) 
	$(CXX) $(CFLAGS) -o demo-main $(MAINOBJS) $(LFLAGS) $(LIBS)

.cpp.o: $(CXX) $(CFLAGS) $(INCLUDES) -c $<

clean: rm -f *.o demo-main