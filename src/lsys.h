/*
** See Copyright Notice In lang.h
** lsys.h
** System Tools
*/


lapi void *sys_valloc(llong length);

lapi llong sys_clockhz();
lapi llong sys_clocktime();

lapi int sys_getmyname(int length, char *buffer);

lapi int sys_workdir(int length, char *buffer);
lapi int sys_setworkdir(char *buffer);

lapi void sys_fclose(Handle file);
lapi Handle sys_fopen(char *name, char *flags);


lapi Handle sys_loadlib(char const *name);
lapi void *sys_libfn(Handle lib, char const *name);

lapi Error sys_loadfilebytes(Alloc *allocator, void **lppOut, char const *fileName);
lapi Error sys_savefilebytes(char const *buffer, llong length, char const *fileName);

lapi int sys_getlasterror();
lapi void sys_geterrormsg(int error, char *buff, int len);
