CC := gcc
CFLAGS := -ggdb -O0 -Wall -Wextra -Werror -Wno-unused-parameter -D_DEFAULT_SOURCE
LDFLAGS := -L/usr/local/lib/ -ltermbox2 -ldl
SRC := editor.c main.c buffer.c nvlua.c cursor.c window.c
OBJ := $(SRC:.c=.o)
EXEC := nv
DEPS_MK := deps.mk

CLANG_TIDY_CHECKS := bugprone-*,readability-*,modernize-*,performance-*,portability-*,clang-analyzer-*,cert-*,cppcoreguidelines-*
CLANG_FORMAT_STYLE := WebKit
VALGRIND_ARGS := -s --track-origins=yes --leak-check=full --show-leak-kinds=all --log-file="valgrind"

# modifies LDFLAGS
-include $(DEPS_MK)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ $(LDFLAGS) -o $@

.PHONY: debug
debug:
	# FIXME: -ex "target remote $(GDBSERVER_PORT)" 
	gdb --args ./$(EXEC) main.c install_deps.sh Makefile

.PHONY: valgrind
valgrind:
	valgrind $(VALGRIND_ARGS) ./$(EXEC) main.c install_deps.sh Makefile

.PHONY: run
run: $(EXEC)
	./$(EXEC) main.c install_deps.sh Makefile

.PHONY: tidy
tidy:
	clang-tidy $(SRC) --checks="$(CLANG_TIDY_CHECKS)" --fix --fix-errors -- -I.

.PHONY: format
format:
	clang-format -i --style=$(CLANG_FORMAT_STYLE) $(SRC) *.h

lint: tidy format
	echo "lint + format"

.PHONY: clean
clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: deps
deps:
	rm -f $(DEPS_MK)
	bash install_deps.sh

.PHONY: all
all: $(EXEC)
