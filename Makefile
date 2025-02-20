CC := gcc
CFLAGS := -ggdb -O0 -Wall -Wextra -ldl -Werror -Wno-unused-parameter
PROD_CFLAGS := -ggdb -O0 -Wall -Wextra -ldl
LDFLAGS := -L/usr/local/lib -ltermbox2
SOURCES := editor.c main.c buffer.c cursor.c window.c
OBJECTS := $(SOURCES:.c=.o)
NVARGS := main.c editor.c
VALGRINDARGS := -s --track-origins=yes --leak-check=full --show-leak-kinds=all --log-file="valgrind" 
EXECUTABLE := nv
GDBSERVER_PORT := 1234

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ $(LDFLAGS) -o $@

debug:
	gdb -ex "target remote $(GDBSERVER_PORT)" --args ./$(EXECUTABLE) $(NVARGS)

gdbserver:
	gdbserver :$(GDBSERVER_PORT) ./$(EXECUTABLE) $(NVARGS)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

valgrind:
	valgrind $(VALGRINDARGS) ./$(EXECUTABLE) $(NVARGS)

run: $(EXECUTABLE)
	./$(EXECUTABLE) $(NVARGS)

.PHONY: all clean run valgrind
