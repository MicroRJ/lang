/*
** See Copyright Notice In elf.h
** llog.h
** Simple Logging Tools
*/


#define LLOGGING


#define ELF_LOGDBUG 	0
#define ELF_LOGINFO 	1
#define ELF_LOGWARN 	2
#define ELF_LOGERROR 3
#define ELF_LOGFATAL 4


char *elf_logtostr(int);



elf_api void elf_log_(int type, ldebugloc loc, char const *fmt, ...);


#define elf_log(TYPE,FORMAT,...) (LCHECKPRINTF(FORMAT,##__VA_ARGS__),elf_log_(TYPE,LHERE,FORMAT,##__VA_ARGS__))
#define elf_loginfo(yyy,...)    elf_log(ELF_LOGINFO,yyy,##__VA_ARGS__)
#define elf_logdebug(yyy,...)   elf_log(ELF_LOGDBUG,yyy,##__VA_ARGS__)
#define elf_logwarning(yyy,...) elf_log(ELF_LOGWARN,yyy,##__VA_ARGS__)
#define elf_logerror(yyy,...)   elf_log(ELF_LOGERROR,yyy,##__VA_ARGS__)
#define elf_logfatal(yyy,...)   elf_log(ELF_LOGFATAL,yyy,##__VA_ARGS__)
