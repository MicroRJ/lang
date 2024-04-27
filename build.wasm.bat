@SETLOCAL


@PUSHD build
	@CALL emcc -c ../elf.c -Wall -Isrc -Os -DPLATFORM_WEB -D_DEBUG
@POPD

@ENDLOCAL