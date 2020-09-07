TARGET = gl
BINPATH = build/
CC = g++
CFLAGS = -I${HOME}/opt/include -std=c++2a -Wall -g
LDFLAGS = -L${HOME}/opt/lib -lm -lglfw -lassimp

OBJECTS = $(patsubst %.cpp, %.o, $(wildcard src/*.cpp))
HEADERS = $(wildcard src/*.h)

.PHONY: test default all clean
default: $(TARGET)
all: default

src/%.o: src/%.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) $(LDFLAGS) -o $(BINPATH)$@

run: $(TARGET)
	DYLD_LIBRARY_PATH=${HOME}/opt/lib $(BINPATH)$(TARGET)

clean:
	rm -f $(BINPATH)$(TARGET)
