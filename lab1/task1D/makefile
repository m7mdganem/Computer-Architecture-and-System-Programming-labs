all: encoder

encoder.o: encoder.c
	gcc -g -m32 -c -o encoder.o encoder.c

encoder: encoder.o
	gcc -g -m32  encoder.o -o encoder
	rm encoder.o

clean:
	rm -rf ./*.o encoder
