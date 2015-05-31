test:
	make build;
	make run;

build:
	gcc -std=c99 -c matrixman.c matrixman-gamefile.c;
	gcc matrixman.o matrixman-gamefile.o -lSDL2 -o matrixman;

run:
	./matrixman;

clean:
	-rm matrixman;
	-rm *.o;
	-rm *~
