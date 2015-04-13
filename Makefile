test:
	make build;
	make run;

build:
	gcc -std=c99 -c matrixman.c;
	gcc matrixman.o -lSDL2 -o matrixman;

run:
	./matrixman;

clean:
	-rm matrixman;
	-rm matrixman.o;
