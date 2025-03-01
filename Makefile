CC := gcc
CFLAGS := -ggdb -O0 -Wall -Wextra -ldl -Werror -Wno-unused-parameter
PROD_CFLAGS := -ggdb -O0 -Wall -Wextra -ldl
LDFLAGS := -L/usr/local/lib -ltermbox2
SOURCES := editor.c main.c buffer.c cursor.c window.c
OBJECTS := $(SOURCES:.c=.o)
EXECUTABLE := nv
EXECUTABLE_ARGS := main.c editor.c
VALGRIND_ARGS := -s --track-origins=yes --leak-check=full --show-leak-kinds=all --log-file="valgrind" 
CLANG_TIDY_CHECKS := bugprone-*,readability-*,modernize-*,performance-*,portability-*,clang-analyzer-*
CLANG_FORMAT_STYLE := WebKit

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ $(LDFLAGS) -o $@

.PHONY: debug
debug:
	# FIXME: -ex "target remote $(GDBSERVER_PORT)" 
	gdb --args ./$(EXECUTABLE) $(EXECUTABLE_ARGS)

.PHONY: valgrind
valgrind:
	valgrind $(VALGRIND_ARGS) ./$(EXECUTABLE) $(EXECUTABLE_ARGS)

.PHONY: run
run: $(EXECUTABLE)
	./$(EXECUTABLE) $(EXECUTABLE_ARGS)

.PHONY: tidy
tidy:
	clang-tidy $(SOURCES) --checks="$(CLANG_TIDY_CHECKS)" --fix --fix-errors -- -I.

.PHONY: format
format:
	clang-format -i --style=$(CLANG_FORMAT_STYLE) $(SOURCES) *.h

lint: tidy format
	echo "lint + format"

.PHONY: clean
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

.PHONY: all
all: $(EXECUTABLE)
