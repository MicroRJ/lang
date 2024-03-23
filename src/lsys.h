/*
** See Copyright Notice In lang.h
** lsys.h
** System Tools
*/


LAPI void *sys_valloc(Integer length);

LAPI Integer sys_clockhz();
LAPI Integer sys_clocktime();

LAPI int sys_getmyname(int length, char *buffer);
LAPI int sys_pwd(int length, char *buffer);

LAPI Handle sys_loaddll(char const *name);
LAPI void *sys_finddllfn(Handle lib, char const *name);

LAPI Error sys_loadfilebytes(Alloc *allocator, void **lppOut, char const *fileName);
LAPI Error sys_savefilebytes(char const *buffer, Integer length, char const *fileName);

LAPI int sys_getlasterror();
LAPI void sys_geterrormsg(int error, char *buff, int len);
