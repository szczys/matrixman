test:
	make build;
	make run;

build:
	gcc -std=c99 -c matrixman.c;
	gcc -std=c99 -c display.c;
	gcc matrixman.o display.o -lSDL2 -o matrixman;

run:
	./matrixman;

clean:
	-rm matrixman;
	-rm *.o;
	-rm *~
