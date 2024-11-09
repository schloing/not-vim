CC := gcc
CFLAGS := -g -Wall -Wextra
SOURCES := editor.c main.c vec.c
OBJECTS := $(SOURCES:.c=.o)
EXECUTABLE := nv

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

run: $(EXECUTABLE)
	./$(EXECUTABLE) main.c

.PHONY: all clean run
