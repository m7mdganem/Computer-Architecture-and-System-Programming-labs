all: menu_map

menu_map.o: menu_map.c
	gcc -g -m32 -c -o menu_map.o menu_map.c

menu_map: menu_map.o
	gcc -g -m32  menu_map.o -o menu

clean:
	rm -rf ./*.o menu
