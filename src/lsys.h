/*
** See Copyright Notice In elf.h
** lsys.h
** System Tools
*/


/* todo: define this elsewhere */
#if defined(_WIN32)
	#define PLATFORM_WIN32
#else
	#if !defined(PLATFORM_WEB)
		#define PLATFORM_WEB
	#endif
#endif


/* triggers the debugger for this program,
returns whether a debugger was successfully
attached */
lapi elf_bool sys_debugger();


lapi void *sys_valloc(elf_int length);

lapi elf_int sys_clockhz();
lapi elf_int sys_clocktime();

lapi int sys_getmyname(int length, char *buffer);
lapi int sys_getmypid();

lapi int sys_pwd(int length, char *buffer);
lapi int sys_setpwd(char *buffer);

lapi elf_Handle sys_loadlib(char const *name);
lapi void *sys_libfn(elf_Handle lib, char const *name);

lapi Error sys_loadfilebytes(Alloc *allocator, void **lppOut, char const *fileName);
lapi Error sys_savefilebytes(char const *buffer, elf_int length, char const *fileName);

lapi int sys_getlasterror();
lapi void sys_geterrormsg(int error, char *buff, int len);
