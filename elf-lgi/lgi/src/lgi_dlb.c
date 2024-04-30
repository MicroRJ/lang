/*
**
** Copyright(c) 2023 - Dayan Rodriguez - Dyr
**
** -+- dlb -+-
**
*/

#ifndef _DLB_VER
#define _DLB_VER 1

typedef struct {
	__int64 sze_max;
	__int64 sze_min;
} dlb_t;

#define ccdlb_(arr) ((dlb_t*)(arr)-1)
#define ccdlb(arr) ((arr)?ccdlb_(arr):(0))

#define arr_max_(arr) (ccdlb_(arr)->sze_max)
#define arr_min_(arr) (ccdlb_(arr)->sze_min)

#define arr_max(arr) ((arr)?arr_max_(arr):(0))
#define arr_min(arr) ((arr)?arr_min_(arr):(0))

#define arrdel(arr) free(ccdlb(arr))
#define arrmax arr_max
#define arrmin arr_min

#define arrlen arrmin

#define arrend_max(arr) ((arr)+arrmax(arr))
#define arrend_min(arr) ((arr)+arrmin(arr))
#define arrend arrend_min

#define arrresi(arr,num) ccdlbadd_((void**)(&arr),sizeof(*(arr)),num,0)
#define arraddi(arr,num) ccdlbadd_((void**)(&arr),sizeof(*(arr)),num,num)

#define arrres(arr,num) ((arr)+arrresi(arr,num))
#define arradd(arr,num) ((arr)+arraddi(arr,num))

#if defined(__cplusplus)
extern "C" {
#endif
	__int64
	ccdlbadd_(void **lpp, __int32 itemSize, __int64 append, __int64 commit) {

		__int64 sze_max = 0, sze_min = 0;
		dlb_t *dlb = NULL;

		if (*lpp) {
			dlb = ccdlb_(*lpp);
			sze_max  = dlb->sze_max;
			sze_min  = dlb->sze_min;
		}
#if defined(_DEBUG)
		if ((commit<=append+sze_max-sze_min) != 1) {
			__debugbreak();
		}
#endif
		if (sze_max < sze_min + append) {
			sze_max <<= 1;
			if (sze_max < sze_min + append) {
				sze_max = sze_min + append;
			}
			dlb = (dlb_t*) realloc(dlb,sizeof(*dlb)+sze_max*itemSize);
			*lpp = dlb + 1;

			dlb->sze_max = sze_max;
		}

		dlb->sze_min = sze_min + commit;
		return sze_min;
	}

#if defined(__cplusplus)
}
#endif

#endif