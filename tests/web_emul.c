
#define ELF_NOMAIN
#include "elf-web.c"

int main(int c, char **v) {
	elfweb_ini();
	elfweb_loadcode("test",
	"let x = 0 \
	let lib = fopen(\"build/raylib.dll\",\"r\")\
	pf(lib) ");
}