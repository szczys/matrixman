test:
	make build;
	make run;

build:
	gcc -std=c99 -c matrixman.c display.c control.c;
	gcc matrixman.o display.o control.o -lSDL2 -o matrixman;

run:
	./matrixman;

clean:
	-rm matrixman;
	-rm *.o;
	-rm *~
