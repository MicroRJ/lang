/*
** See Copyright Notice In elf.h
** elf-sys.h
** System Tools
*/


elf_api void *sys_valloc(elf_int length);


elf_api void sys_consolelog(int type, char *message);


/* triggers the debugger for this program,
returns whether a debugger was successfully
attached */
elf_api elf_bool sys_debugger();

elf_api elf_int sys_clockhz();
elf_api elf_int sys_clocktime();

elf_api int sys_getmyname(int length, char *buffer);
elf_api int sys_getmypid();

elf_api int sys_pwd(int length, char *buffer);
elf_api int sys_setpwd(char *buffer);

elf_api elf_Handle sys_loadlib(char const *name);
elf_api void *sys_libfn(elf_Handle lib, char const *name);

elf_api Error sys_loadfilebytes(Alloc *allocator, void **lppOut, char const *fileName);
elf_api Error sys_savefilebytes(char const *buffer, elf_int length, char const *fileName);

elf_api int sys_getlasterror();
elf_api void sys_geterrormsg(int error, char *buff, int len);
