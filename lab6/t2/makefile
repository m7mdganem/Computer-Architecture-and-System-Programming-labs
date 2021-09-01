all: exec

exec: LineParser.o task2.o
	gcc -g -m32 LineParser.o task2.o -o myshell
	rm LineParser.o task2.o

LineParser.o: LineParser.c
	gcc -g -m32 -c LineParser.c -o LineParser.o

task2.o: task2.c
	gcc -g -m32 -c task2.c -o task2.o

clean:
	rm -rf ./*.o myshell