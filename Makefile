.PHONY: all clean

# to use make for windows, get the toolkit from:
# git @ skeeto/w64devkit/releases
# this makefile needs working...


MAKE ?= make
CC = clang-cl
PLATFORM ?= PLATFORM_DESKTOP

CFLAGS = -D$(PLATFORM)
OUT = build
ifeq ($(PLATFORM),PLATFORM_WEB)
	CC = emcc
	OUT = build/web
	CFLAGS += -O3 -Wall
else ifeq ($(PLATFORM),PLATFORM_DESKTOP)
	CFLAGS += -TC -Z7 -W4
	CFLAGS += -Od -MTd
	CFLAGS += -D_DEBUG
endif

SRC = $(wildcard src/*)

#  --preload-file code/examples --preload-file code/tests
ifeq ($(PLATFORM),PLATFORM_WEB)
MYSHELL = shell.html
MYDATA = --preload-file code
all: build/elf_web.exe build/raylib_web.dll
build/raylib_web.dll: $(SRC) elf-ray/raylib.c
	$(CC) $(CFLAGS) elf-ray/raylib.c -o $(OUT)/raylib.o.wasm -I. -Ielf-ray/raylib/src -Ielf-ray/raygui/src elf-ray/libraylib.a -sSIDE_MODULE=1 -sUSE_GLFW=3
build/elf_web.exe: elf-web.c $(SRC)
	$(CC) $(CFLAGS) -o $(OUT)/index.html elf-web.c -Isrc --shell-file $(MYSHELL) $(MYDATA) -sEXPORTED_RUNTIME_METHODS="['ccall','cwrap','wasmExports']" -sMAIN_MODULE -sASYNCIFY -sUSE_GLFW=3 -sALLOW_MEMORY_GROWTH
else
all: build/elf.exe build/lgilib.dll build/raylib.dll tests/web_emul.exe
build/elf.exe: elf.h elf.c $(SRC)
	$(CC) $(CFLAGS) elf.c -o build/elf.exe -I.
# winmm.lib
WINLIBS = shell32.lib Gdi32.lib winmm.lib
build/raylib.dll: elf.h $(SRC) elf-ray/raylib.c
	$(CC) $(CFLAGS) elf-ray/raylib.c -o $(OUT)/raylib.dll -I. -Ielf-ray/raylib/src -Ielf-ray/raygui/src /link /DLL $(WINLIBS) elf-ray/libraylib.lib
build/lgilib.dll: elf.h $(SRC)
	$(CC) $(CFLAGS) elf-lgi/lgilib.c -o build/lgilib.dll -I. -Ielf-lgi/lgi /link /DLL
tests/web_emul.exe: elf.h tests/web_emul.c elf-web.c $(SRC)
	$(CC) $(CFLAGS) tests/web_emul.c -o tests/web_emul.exe -I. -I..
endif
clean:
	del build/*.lib build/*.dll build/*.obj build/*.exe /s

# MYDATA=--preload-file code/ray.elf --preload-file code/examples --preload-file code/tests
# -s EXPORTED_FUNCTIONS="['elf_loadfile', 'elf_loadexpr']"
# Build raylib in web mode, this generates a module
# file .wasm, which we'll rename to dll, this can
# then be loaded using em's dlopen function
#  emcc elf-ray/raylib.c -o build/web/raylib.dll elf-ray/libraylib.a -I. -Ielf-ray/raylib/src -DPLATFORM_WEB -sSIDE_MODULE=1 -sUSE_GLFW=3
