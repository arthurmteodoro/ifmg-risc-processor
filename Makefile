all:
	gcc -Wall -c hash.c -o hash.o
	gcc -Wall hash.o montador.c -o montador.out
	gcc -Wall simulador.c -o simulador.out