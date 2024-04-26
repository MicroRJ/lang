/*
** See Copyright Notice In elf.h
** llog.h
** Simple Logging Tools
*/


enum {
	LOG_FATAL = 0,
	LOG_ERROR,
	LOG_WARNING,
	LOG_INFO,
	LOG_DEBUG,
};

lapi void elf_log_(int type, ldebugloc loc, char const *fmt, ...);

#define elf_log(TYPE,FORMAT,...) (LCHECKPRINTF(FORMAT,__VA_ARGS__),elf_log_(TYPE,LHERE,FORMAT,__VA_ARGS__))
#define elf_loginfo(yyy,...) elf_log(LOG_INFO,yyy,__VA_ARGS__)
#define elf_logdebug(yyy,...) elf_log(LOG_DEBUG,yyy,__VA_ARGS__)
#define elf_logwarning(yyy,...) elf_log(LOG_WARNING,yyy,__VA_ARGS__)
#define elf_logerror(yyy,...) elf_log(LOG_ERROR,yyy,__VA_ARGS__)
#define elf_logfatal(yyy,...) elf_log(LOG_FATAL,yyy,__VA_ARGS__)
