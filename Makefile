CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
LDFLAGS = -lsqlite3 -pthread

SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:.cpp=.o)
TARGET = fulfillx

all: $(TARGET)

$(TARGET): $(OBJS)
	@if [ -z "$(SRCS)" ]; then \
		echo "No source files found in src/. Nothing to build."; \
	else \
		$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS); \
	fi

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
