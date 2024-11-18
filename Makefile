CC := gcc
CFLAGS := -g -Wall -Wextra -ldl
LDFLAGS := -L/usr/lib/ -ltermbox2
SOURCES := editor.c main.c vec.c buffer.c
OBJECTS := $(SOURCES:.c=.o)
EXECUTABLE := nv

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ $(LDFLAGS) -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

run: $(EXECUTABLE)
	./$(EXECUTABLE) main.c

.PHONY: all clean run
