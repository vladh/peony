TARGET = peony
BINPATH = build/
CC = g++
CFLAGS = -I${HOME}/opt/include -std=c++2a -g -Wall -Wno-deprecated-volatile
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

# Things we used earlier and should remember:
# LDFLAGS = -L${HOME}/opt/lib -lm ...
# DYLD_LIBRARY_PATH=${HOME}/opt/lib $(BINPATH)$(TARGET)
