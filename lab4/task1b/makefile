all: exec
exec: start.s util.c task1b.c
	nasm -f elf32 start.s -o start.o
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector task1b.c -o task1b.o
	ld -m elf_i386 start.o task1b.o util.o -o encoder
	rm task1b.o util.o start.o

clean:
	rm -rf ./*.o encoder