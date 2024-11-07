all:
	gcc -g *.c -o nv

run: ./nv
	./nv main.c
