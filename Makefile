CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -lSDL2

TARGET = chip8

all:
	$(CXX) $(CXXFLAGS) main.cpp -o $(TARGET)

clean:
	rm -f $(TARGET)
