@SETLOCAL


@PUSHD build
	@REM @CALL emcc -c ../elf.c -Wall -Isrc -Os -DPLATFORM_WEB -D_DEBUG
	@REM @CALL emar rcs elf.a
	@REM -Os -g -S
	@CALL emcc -o elf.html ../elf.c -O0 -Wall -Isrc -s USE_GLFW=3 -DPLATFORM_WEB --preload-file ../code/tests
@POPD

@ENDLOCAL