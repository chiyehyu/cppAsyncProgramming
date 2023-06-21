# Variables
CXX = g++
CXXFLAGS = -std=c++14 -g
SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)
TARGET = myprogram

# Default target
all: $(TARGET)

# Linking Rule
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compilation Rule
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Clean Rule
clean:
	rm -f $(OBJ) $(TARGET)