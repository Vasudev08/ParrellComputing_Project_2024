# Makefile for Sample Sort Test Program

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -g

# Source files
SRCS = sample_sort.cpp

# Output executable name
TARGET = sample_sort_test

# Default target
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

# Clean up the build files
clean:
	rm -f $(TARGET)

# Run the program with a given input file
run: $(TARGET)
	@echo "Running the program..."
	@./$(TARGET) input_10.txt

.PHONY: all clean run
