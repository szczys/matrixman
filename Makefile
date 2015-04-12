test:
	make build;
	make run;

build:
	gcc -c hello.c;
	gcc hello.o -lSDL2 -o hello;

run:
	./hello;

clean:
	-rm hello;
	-rm hello.o;
