/*
** See Copyright Notice In lang.h
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

LAPI void lang_log_(int type, Debugloc source, char const *fmt, ...);

#define lang_log(TYPE,FORMAT,...) (LCHECKPRINTF(FORMAT,__VA_ARGS__),lang_log_(TYPE,LHERE,FORMAT,__VA_ARGS__))
#define lang_loginfo(yyy,...) lang_log(LOG_INFO,yyy,__VA_ARGS__)
#define lang_logdebug(yyy,...) lang_log(LOG_DEBUG,yyy,__VA_ARGS__)
#define lang_logwarning(yyy,...) lang_log(LOG_WARNING,yyy,__VA_ARGS__)
#define lang_logerror(yyy,...) lang_log(LOG_ERROR,yyy,__VA_ARGS__)
#define lang_logfatal(yyy,...) lang_log(LOG_FATAL,yyy,__VA_ARGS__)
