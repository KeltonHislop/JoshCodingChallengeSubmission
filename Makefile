CXX := g++
CXXFLAGS := -std=c++14 -Wall -Wextra -Wpedantic -O2
CPPFLAGS := -Ilibraries -D_WIN32_WINNT=0x0A00
LDFLAGS := -static
LDLIBS := -lws2_32

TARGET := bin/light_monitor.exe
SRC := src/main.cpp src/LightClient.cpp src/LightMonitor.cpp
HDR := src/Light.h src/LightClient.h src/LightMonitor.h

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC) $(HDR)
	if not exist bin mkdir bin
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS) $(LDLIBS)

clean:
	if exist $(TARGET) del /Q $(subst /,\,$(TARGET))