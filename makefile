munguShell.out : main.o shell.o
	gcc -o munguShell.out main.o shell.o -lpthread && make clean-o

shell.o : shell.c
	gcc -c -o shell.o shell.c

main.o : main.c
	gcc -c -o main.o main.c

clean :
	rm *.o munguShell.out

clean-o :
	rm *.o
