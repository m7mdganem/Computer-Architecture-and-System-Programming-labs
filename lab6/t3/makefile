all: exec

exec: LineParser.o task3.o
	gcc -g -m32 LineParser.o task3.o -o myshell
	rm LineParser.o task3.o

LineParser.o: LineParser.c
	gcc -g -m32 -c LineParser.c -o LineParser.o

task3.o: task3.c
	gcc -g -m32 -c task3.c -o task3.o

clean:
	rm -rf ./*.o myshell