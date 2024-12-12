CC := gcc
CFLAGS := -ggdb -O0 -Wall -Wextra -ldl
PROD_CFLAGS := -ggdb -O0 -Wall -Wextra -ldl
LDFLAGS := -L/usr/local/lib -ltermbox2
SOURCES := editor.c main.c buffer.c
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
