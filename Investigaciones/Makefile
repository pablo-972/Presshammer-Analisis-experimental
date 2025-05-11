CXX := g++
CFLAGS := -std=c++11 -g -O3
LFLAGS :=
LIBS :=

MAINSOURCES := main.cpp Mapping.cpp permissions.cpp content.cpp elf_header.cpp presshammer.cpp
MAINOBJS := $(MAINSOURCES:.cpp=.o)

TARGET := main
all: $(TARGET)

$(TARGET): $(MAINOBJS)
	$(CXX) $(CFLAGS) -o investigaciones $(MAINOBJS) $(LFLAGS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

clean: 
	rm -f *.o $(TARGET)