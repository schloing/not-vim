# not-vim
## usage
```sh
./bin/nv [file]...
```
## running
### Makefile
```sh
cmake .
make
./bin/nv [file]...
```
### faster compilations
```sh
cmake -G Ninja .
ninja
./bin/nv [file]...
```
## debugging
### debugging with valgrind (unix)
```sh
sh run_valgrind.sh [file]...
```
### callgrind
```sh
valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes ./bin/nv main.c
```
### debugging with gdb (unix)
```sh
sh run_gdb.sh [file]...
```
### todo
- [ ] actually write changes to the file
- [ ] remappable keybinds
- [ ] make the lua actually do shit
- [ ] write plugload
- [ ] per window status line
- [ ] window padding
- [ ] syntax highlighting
- better cursor
    - [ ] emacs jump
    - [ ] rectangular select, multiple cursors, macros
- [ ] save editor state (open windows)
- [ ] shell integration. maybe even primarily work as a terminal multiplexer with a shit ton of shit
- [ ] make the code less ass
### libraries
inspired by
https://github.com/adsr/mle

termbox2 for tui
https://github.com/termbox/termbox2

vector
https://github.com/eteran/c-vector/blob/master/cvector.h

### read
https://www.codingwiththomas.com/blog/a-lua-c-api-cheat-sheet
