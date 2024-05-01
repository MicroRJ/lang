/*
** See Copyright Notice In elf.h
** elf-aux.h
** Auxiliary Functions
*/


void elf_debugger(char *message);



void elf_register(elf_Runtime *R, char *name, lBinding fn);
void elf_registerint(elf_Runtime *R, char *name, int val);



elf_int elf_clocktime();
elf_num elf_timediffs(elf_int begin);


void elf_throw(elf_Runtime *R, lbyteid id, char *error);


int elf_fndlinefile(elf_Module *md, llineid line);
void elf_getlinelocinfo(char *q, char *loc, int *linenum, char **lineloc);


void elf_tabmfld(elf_Runtime *R, elf_Table *obj, char *name, lBinding b);
void elf_register(elf_Runtime *, char *, lBinding fn);






