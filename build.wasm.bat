@REM You should have emsdk installed
@REM already, this script should be
@REM run from cmd with admin privs,
@REM I think? EMSDK is in system vars so...
@REM
@REM clone from git @ emcripten-code/emsdk
@REM cd emsdk
@REM git pull
@REM emsdk install latest
@REM emsdk activate latest --system --permanent
@REM
@REM
@IF "%ELFBUILDWEB_ONCE%"==""  (
   @CALL %EMSDK%/emsdk activate latest
)
@SET ELFBUILDWEB_ONCE="YES"
@SETLOCAL
@PUSHD build
	@REM -Os -g -S
	@CALL emcc -o elf.html ../elf.c -O0 -Wall -Isrc -s USE_GLFW=3 -DPLATFORM_WEB --preload-file ../code/tests
@POPD
@ENDLOCAL