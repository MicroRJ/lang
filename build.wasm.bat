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
@REM to run:
@REM emrun index.html
@REM
@IF "%ELFBUILDWEB_ONCE%"==""  (
   @CALL %EMSDK%/emsdk activate latest
)
@SET ELFBUILDWEB_ONCE="YES"
@SETLOCAL
@SET MYSHELL=shell.html
@SET MYSHELL=shell2.html
@SET MYDATAP=code/tests

@REM -Os -g -S
@REM -s EXPORTED_FUNCTIONS="['elf_loadfile', 'elf_loadexpr']"
@CALL emcc -o build\web\index.html elf.c -O0 -Wall -Isrc -s USE_GLFW=3 -DPLATFORM_WEB --shell-file %MYSHELL% --preload-file %MYDATAP% -s EXPORTED_RUNTIME_METHODS="['cwrap','wasmExports']"

@ENDLOCAL