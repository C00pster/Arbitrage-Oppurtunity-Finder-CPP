CXX = g++
CXXFLAGS = -g -Werror -Wall -Wextra -Iinclude -O3
LDFLAGS = -lcurl

TARGET = arbitrage

SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:src/%.cpp=obj/%.o)

# The build target
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)

# Rule for object files
obj/%.o: src/%.cpp
	mkdir -p obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)
	rm -rf obj/
	rm -rf bin/