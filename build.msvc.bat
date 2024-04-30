@IF "%INCLUDE%"==""  (
   @CALL vcvars64
)
@SETLOCAL
@SET myCompilerOptions=/options:strict /nologo /TC /Z7 /WX /W4
@SET myInclude=/I. /I.. /I../stb /I../elf-lgi/lgi /I../elf-ray/raylib/src
@SET myCommon=%myCompilerOptions% %myInclude%
@SET myLinkerOptions=/INCREMENTAL:NO

@SET myDebugFlags=/Od /D_DEBUG /MTd
@SET myReleaseFlags=/O2
@SET myGenFlags=%myDebugFlags%
@PUSHD build

@REM @CALL gcc -shared ../elf-ray/raylib.c -o raylib.dll ../elf-ray/libraylib.a -I../elf-ray/raylib/src -I.. -lgdi32 -lwinmm
@CALL cl %myCommon% %myGenFlags% ../elf.c /link %myLinkerOptions% /SUBSYSTEM:CONSOLE
@CALL cl %myCommon% %myGenFlags% ../elf-lgi/lgilib.c /link %myLinkerOptions% /DLL
@REM @CALL cl %myCommon% %myGenFlags% ../elf-ray/raylib.c /link %myLinkerOptions% /DLL
@REM @CALL ../elf-ray/libraylib.a -I../elf-ray/raylib/src -I.. -lgdi32 -lwinmm

@POPD
@ENDLOCAL