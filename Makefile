# Makefile for virtual-led-volume - Linux build
# author: Stefano Di Patti
#

# name of the file
FILENAME = virtual-led-volume

CC = gcc
CXX = g++

# Include path
IDIR = external/include/

# Libraries path
LDIR = external/libs/linux

# compiler flags:
CXXFLAGS  = -g -O0 -x c++ -Wall -Wno-invalid-offsetof -std=c++17 -I$(IDIR) -Isrc

# linker flags:
LDFLAGS = -L$(LDIR) -lglfw3 -lassimp -lz -lminizip -lkubazip -lpoly2tri -lpolyclipping -ldraco -lpugixml

SOURCES = external/include/glad/glad.c src/main.cpp

# project headers: most of the code is in header files, so they are dependencies of the executable too
HEADERS = $(shell find src -name '*.h')

# CPU tests (no OpenGL context needed)
TEST_SOURCES = tests/test_offaxis.cpp
TEST_TARGET = test_offaxis.out

TARGET = $(FILENAME).out

.PHONY : all
all: $(TARGET)

.PHONY : clean
clean :
	-rm $(TARGET) -f
# the executable is a real file: rebuilt only if a source or a header is newer than it
$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SOURCES) $(LDFLAGS) -o $(TARGET)

.PHONY : run
run : all
	./$(TARGET)

.PHONY : test
test : $(TEST_TARGET)
	./$(TEST_TARGET)

# the test uses only GLM: no glad, no libraries to link
$(TEST_TARGET): $(TEST_SOURCES) src/render/offaxis.h
	$(CXX) $(CXXFLAGS) $(TEST_SOURCES) -o $(TEST_TARGET)
