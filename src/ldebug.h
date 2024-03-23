/*
** See Copyright Notice In lang.h
** ldebug.h
** Debug Tools
*/


typedef struct Debugloc {
	char const *fileName;
	int lineNumber;
	char const *func;
	char const *lineStart;
	char const *fileStart;
} Debugloc;


void lang_setasserthook(int (*hook)(Debugloc));
void lang_assertfn(Debugloc ind, char const *name, Bool expr);


#define LHERE (Debugloc){__FILE__,__LINE__,__func__}


#define LASSERTALWAYS(xx) lang_assertfn(LHERE,XSTRINGIFY(xx),xx)


#if defined(_DEBUG)
	#define LASSERT(xx) LASSERTALWAYS(xx)
#else
	#define LASSERT(xx)
#endif


#if defined(_DEBUG)
	#define LDODEBUG(xx) do { xx; } while(0)
#else
	#define LDODEBUG(xx)
#endif


#if !defined(LNOCHANCE)
	#define LNOCHANCE LASSERTALWAYS(!"You've Hit A Roadblock")
#endif


#if defined(_DEBUG)
	#define LCHECKPRINTF(FORMAT,...) ((False)?(sprintf_s(Null,0,FORMAT,__VA_ARGS__),Null):Null)
#else
	#define LCHECKPRINTF(FORMAT,...) 0
#endif



