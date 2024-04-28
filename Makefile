# to use make for windows, get the toolkit from:
# git @ skeeto/w64devkit/releases
elf.o: elf.h elf.c
	cc -c elf.c -o build/elf.o -I.

