/*
** See Copyright Notice In elf.h
** ldebug.h
** Debug Tools
*/


typedef struct ldebugloc {
	char const *fileName;
	int lineNumber;
	char const *func;
	char const *lineStart;
	char const *fileStart;
} ldebugloc;


void lang_setasserthook(int (*hook)(ldebugloc));
void lang_assertfn(ldebugloc ind, char const *name, elf_bool expr);


#define LHERE (ldebugloc){__FILE__,__LINE__,__func__}


#define LASSERTALWAYS(xx) lang_assertfn(LHERE,XSTRINGIFY(xx),xx)


#if defined(_DEBUG)
	#define elf_assert(xx) LASSERTALWAYS(xx)
#else
	#define elf_assert(xx)
#endif


#if defined(_DEBUG)
	#define LDODEBUG(xx) do { xx; } while(0)
#else
	#define LDODEBUG(xx)
#endif


#if !defined(LNOBRANCH)
	#define LNOBRANCH do {\
		printf("%s[%i] %s(): You've Hit A Roadblock\n",__FILE__,__LINE__,__func__);\
		__debugbreak();\
	} while (0)
#endif


#if defined(_DEBUG)
	#define LCHECKPRINTF(FORMAT,...) ((lfalse)?(sprintf_s(lnil,0,FORMAT,__VA_ARGS__),lnil):lnil)
#else
	#define LCHECKPRINTF(FORMAT,...) 0
#endif



