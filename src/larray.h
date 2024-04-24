/*
** See Copyright Notice In lang.h
** larray.h
** (A) Array elf_obj and Array Tools
*/


typedef struct Array {
	elf_obj  obj;
	elf_int max;
	elf_int min;
   /* contents are allocated past this point */
} Array;




/*
** The following are utilities for working
** with stb styled stretchy buffers.
** Usage is as follows:
**
**
** T *items = 0;
** langA_varadd(items,(T){});
**
** int i = langA_variadd(items,5);
** items[i..i+5] == (T){}
**
** T *slot = langA_varnadd(items,5);
** slot[i..i+5] == (T){}
**
**
*/
#define langA_vararr(var) ((Array*)(var))[-1]
#define langA_varfor(T,N,A) for (T N = A; N < A + elf_arrlen(A); N += 1)
#define langA_varjfor(A) for (elf_int j = 0; j < elf_arrlen(A); ++ j)
#define elf_forivar(A) for (elf_int i = 0; i < elf_arrlen(A); ++ i)
#define elf_delvar(var) ((var != 0) ? elf_delmem(lHEAP,(Array*)(var)-1),0 : 0)
#define langA_varmax(var) ((var != 0) ? ((Array*)(var))[-1].max : 0)
#define langA_varmin(var) ((var != 0) ? ((Array*)(var))[-1].min : 0)
#define langA_varnadd2(var,res,com) ((var) + langA_varadd_((void**)&(var),sizeof(*var),res,com))
#define langA_variadd(var,num) (langA_varadd_((void**)&(var),sizeof(*var),num,num))
#define langA_varnadd(var,num) ((var) + langA_variadd(var,num))
#define langA_varadd(var,t) ((void)(langA_varnadd(var,1)[0] = t))
#define elf_arrlen langA_varmin


/*
** Returns the last index of the array
** that can be written to
*/
elf_int langA_varadd_(void **var
, elf_int per, elf_int res, elf_int com);
