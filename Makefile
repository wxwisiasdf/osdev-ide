CXX=g++
CXXFLAGS=-Wall -Wextra -g -std=c++2a $(shell fltk-config --cxxflags)

LDFLAGS=$(shell fltk-config --ldflags)

all: osdev-ide

clean:
	$(RM) *.o

osdev-ide: buffer.o device_man.o editor.o find.o main.o project.o qemu.o tab.o ui.o syntax/asm.cpp syntax/c.cpp syntax/rust.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.cpp %.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

syntax/%.o: syntax/%.cpp syntax/%.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

syntax/%.o: syntax/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all clean
