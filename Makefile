./nv:
	gcc -g *.c -o nv

clean:
	rm ./nv

run: nv
	./nv main.c
