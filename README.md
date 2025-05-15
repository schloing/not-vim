# not-vim
## usage
```sh
./bin/nv [file]...
```
## running
```sh
cmake .
make
./bin/nv [file]...
```
### debugging with valgrind (unix)
```sh
sh run_valgrind.sh [file]...
```
### debugging with gdb (unix)
```sh
sh run_gdb.sh [file]...
```
### libraries
inspired by
https://github.com/adsr/mle

termbox2 for tui
https://github.com/termbox/termbox2

vector
https://github.com/eteran/c-vector/blob/master/cvector.h

### read
https://www.codingwiththomas.com/blog/a-lua-c-api-cheat-sheet
