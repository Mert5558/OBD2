CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude -Wno-unused-result
LDFLAGS = -pthread

TARGET = obd2_monitor
SOURCES = src/main.cpp src/connection.cpp src/pids.cpp src/dtc.cpp src/logger.cpp
OBJECTS = $(SOURCES:.cpp=.o)
HEADERS = include/connection.h include/pids.h include/dtc.h include/logger.h

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	rm -f $(OBJECTS)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) obd_log.csv

fclean: clean
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean fclean run
