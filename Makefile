TARGET = peony
BINPATH = build/
CC = g++
CFLAGS = -I${HOME}/opt/include -std=c++2a -Wall -g
# LDFLAGS = -L${HOME}/opt/lib -lm -lglfw -lassimp
LDFLAGS = -lm -lglfw -lassimp -lfreetype

OBJECTS = src/peony.cpp

.PHONY: test default all clean
default: $(TARGET)
all: default

$(TARGET):
	$(CC) $(OBJECTS) $(CFLAGS) $(LDFLAGS) -o $(BINPATH)$@

run: $(TARGET)
	$(BINPATH)$(TARGET)

clean:
	rm -f $(BINPATH)$(TARGET)

# DYLD_LIBRARY_PATH=${HOME}/opt/lib $(BINPATH)$(TARGET)
