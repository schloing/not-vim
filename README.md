# not-vim
## cloning
```sh
git clone --recursive https://github.com/schloing/not-vim.git
```
## building
### requirements:
- cmake >3.14
- pkg-config / pkgconf
- gcc
- luajit (optional)
- libuv
- nng

libuv and nng are automatically installed by the build script

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
## running
```sh
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
### dependencies & libraries:
all functionally required dependencies are statically linked by default, so no additional installation is required
- [cvector](https://github.com/eteran/c-vector/tree/master)
- [termbox2](https://github.com/termbox/termbox2)
- [nvtree](https://github.com/schloing/nvtree)
- [libuv](https://github.com/libuv/libuv)
- [nng](https://github.com/nanomsg/nng)

### read
https://www.codingwiththomas.com/blog/a-lua-c-api-cheat-sheet
