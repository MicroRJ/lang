/*
** See Copyright Notice In elf.h
** elf-aux.h
** Auxiliary Functions
*/


void elf_debugger(char *message);



void elf_register(elf_ThreadState *R, char *name, lBinding fn);
void elf_registerint(elf_ThreadState *R, char *name, int val);



elf_int elf_clocktime();
elf_num elf_timediffs(elf_int begin);


void elf_throw(elf_ThreadState *R, lbyteid id, char *error);


int elf_fndfilebyline(elf_Module *md, llineid line);
void elf_getlinelocinfo(char *q, char *loc, int *linenum, char **lineloc);


void elf_tabmfld(elf_ThreadState *R, elf_Table *obj, char *name, lBinding b);






