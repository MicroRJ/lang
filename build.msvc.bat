@IF "%INCLUDE%"==""  (
   @CALL vcvars64
)
@SETLOCAL
@SET myCompilerOptions=/options:strict /nologo /TC /Z7 /WX /W4
@SET myInclude=/I. /I.. /I../stb /I../lgi
@SET myCommon=%myCompilerOptions% %myInclude%
@SET myLinkerOptions=/INCREMENTAL:NO

@SET myDebugFlags=/Od /D_DEBUG /MTd
@SET myReleaseFlags=/O2
@SET myGenFlags=%myDebugFlags%
@PUSHD build
@CALL cl %myCommon% %myGenFlags% ../elf.c /link %myLinkerOptions% /SUBSYSTEM:CONSOLE
@CALL cl %myCommon% %myGenFlags% ../lgilib.c /link %myLinkerOptions% /DLL
@POPD
@ENDLOCAL