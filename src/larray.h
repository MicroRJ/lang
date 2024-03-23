/*
** See Copyright Notice In lang.h
** larray.h
** (A) Array Object and Array Tools
*/


typedef struct Array {
	Object  obj;
	Integer max;
	Integer min;
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
#define langA_varfor(T,N,A) for (T N = A; N < A + langA_varlen(A); N += 1)
#define langA_varifor(A) for (Integer i = 0; i < langA_varlen(A); ++ i)
#define langA_vardel(var) ((var != 0) ? langM_dealloc(lHEAP,(Array*)(var)-1),0 : 0)
#define langA_varmax(var) ((var != 0) ? ((Array*)(var))[-1].max : 0)
#define langA_varmin(var) ((var != 0) ? ((Array*)(var))[-1].min : 0)
#define langA_varnadd2(var,res,com) ((var) + langA_varadd_((void**)&(var),sizeof(*var),res,com))
#define langA_variadd(var,num) (langA_varadd_((void**)&(var),sizeof(*var),num,num))
#define langA_varnadd(var,num) ((var) + langA_variadd(var,num))
#define langA_varadd(var,t) (langA_varnadd(var,1)[0] = t)
#define langA_varlen langA_varmin


/*
** Returns the last index of the array
** that can be written to
*/
Integer langA_varadd_(void **var
, Integer per, Integer res, Integer com);
