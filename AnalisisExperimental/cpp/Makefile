CXX := g++
CFLAGS := -std=c++11 -g -O3
LFLAGS :=
LIBS :=


MAINSOURCES := main.cpp Mapping.cpp presshammer.cpp taggon.cpp
MAINOBJS := $(MAINSOURCES:.cpp=.o)


TARGET := main

all: $(TARGET)

$(TARGET): $(MAINOBJS)
	$(CXX) $(CFLAGS) -o analisis_experimental $(MAINOBJS) $(LFLAGS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

clean: 
	rm -f *.o $(TARGET)