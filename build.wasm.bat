@REM -- You should have emsdk installed
@REM -- already, ensure EMSDK path is set,
@REM -- if you've installed em and the path
@REM -- hasn't updated, restart your machine.
@REM -- Getting emsdk:
@REM clone from git @ emcripten-code/emsdk
@REM cd emsdk
@REM git pull
@REM emsdk install latest
@REM emsdk activate latest --system --permanent
@REM -- to run, on a separate cmd do:
@REM emrun index.html
@REM
@IF "%ELFBUILDWEB_ONCE%"==""  (
   @CALL %EMSDK%/emsdk activate latest
)
@SET ELFBUILDWEB_ONCE="YES"
@SETLOCAL
@SET MYSHELL=shell.html
@REM @SET MYSHELL=shell2.html
@SET MYDATA=--preload-file code/ray.elf --preload-file code/examples --preload-file code/tests
@REM USE_GLFW=3
@REM -Os -g -S
@REM -s EXPORTED_FUNCTIONS="['elf_loadfile', 'elf_loadexpr']"

@REM Build raylib in web mode, this generates a module
@REM file .wasm, which we'll rename to dll, this can
@REM then be loaded using em's dlopen function
@CALL emcc elf-ray/raylib.c -o build/web/raylib.dll elf-ray/libraylib.a -O0 -Wall -I. -Ielf-ray/raylib/src -DPLATFORM_WEB -sSIDE_MODULE=1 -sUSE_GLFW=3
@REM Build the web-compiler / api
@CALL emcc -o build\web\index.html elf-web.c -O0 -Wall -Isrc -DPLATFORM_WEB --shell-file %MYSHELL% %MYDATA% -s EXPORTED_RUNTIME_METHODS="['ccall','cwrap','wasmExports']" -sMAIN_MODULE -sASYNCIFY -sUSE_GLFW=3



@ENDLOCAL