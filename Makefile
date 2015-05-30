test:
	make build;
	make run;

build:
	gcc -std=c99 -c main.c matrixman.c;
	gcc main.o matrixman.o -lSDL2 -o main;

run:
	./main;

clean:
	-rm main;
	-rm *.o;
	-rm *~
