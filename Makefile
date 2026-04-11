CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
LDFLAGS = -pthread

TARGET = obd2_monitor
SOURCES = main.cpp connection.cpp pids.cpp dtc.cpp logger.cpp
OBJECTS = $(SOURCES:.cpp=.o)
HEADERS = connection.h pids.h dtc.h logger.h

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET) obd_log.csv

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
