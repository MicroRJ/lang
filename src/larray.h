/*
** See Copyright Notice In elf.h
** larray.h
** (A) Array elf_Object and Array Tools
*/


typedef struct Array {
	elf_Object  obj;
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
** elf_varadd(items,(T){});
**
** int i = elf_varaddi(items,5);
** items[i..i+5] == (T){}
**
** T *slot = elf_varaddn(items,5);
** slot[i..i+5] == (T){}
**
**
*/
#define langA_vararr(var) ((Array*)(var))[-1]
#define langA_varfor(T,N,A) for (T N = A; N < A + elf_varlen(A); N += 1)
#define langA_varjfor(A) for (elf_int j = 0; j < elf_varlen(A); ++ j)
#define elf_arrfori(A) for (elf_int i = 0; i < elf_varlen(A); ++ i)
#define elf_delvar(var) ((var != 0) ? elf_delmem(lHEAP,(Array*)(var)-1),0 : 0)
#define langA_varmax(var) ((var != 0) ? ((Array*)(var))[-1].max : 0)
#define langA_varmin(var) ((var != 0) ? ((Array*)(var))[-1].min : 0)

#define elf_arrdecmin(var) ( (var) != lnil ? (-- ((Array*)(var))[-1].min) : 0 )

#define elf_varaddx(var,res,com) ((var) + elf_varaddxx((void**)&(var),sizeof(*var),res,com))
#define elf_varaddi(var,num) (elf_varaddxx((void**)&(var),sizeof(*var),num,num))
#define elf_varaddn(var,num) ((var) + elf_varaddi(var,num))
#define elf_varlen langA_varmin

#if 0
/* seems that only msvc compiles this properly */
#define elf_varadd(var,t) ((void)(elf_varaddn(var,1)[0] = t))
#else
#define elf_varadd(var,val) do {\
	int ___i___ = elf_varaddi(var,1);\
	var[___i___] = val;\
} while(0)
#endif



/*
** Returns the last index of the array
** that can be written to
*/
elf_int elf_varaddxx(void **var
, elf_int per, elf_int res, elf_int com);
