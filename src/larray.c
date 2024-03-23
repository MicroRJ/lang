/*
** See Copyright Notice In lang.h
** larray.c
** Array Object and Array Tools
*/

#if 0
/* todo?: will we have arrays or not? */
Array *langA_new(Module *fs, Integer n) {
	// static MetaFunc _m[] = {
		// {"length",langA_length_},
		// {"slice",langA_slice_},
		// {"clone",langA_clone_},
	// };

	Array *array = langGC_allocobj(fs,OBJ_ARRAY,sizeof(Array) + sizeof(Value) * n);
	// array->obj._m = _m;
	// array->obj._n = _countof(_m);
	return array;
}
#endif


Integer langA_varadd_(void **var, Integer per, Integer res, Integer com) {
	Integer max = 0;
	Integer min = 0;
	Array *arr = 0;
	if (*var != 0) {
		arr = ((Array*)(*var)) - 1;
		max = arr->max;
		min = arr->min;
	}
   /* increment reserve if we attempt to commit
   more than we've got reserved */
	if (max + res < com) {
		res += (com - (max + res));
	}
	if (min + res > max) {
		max <<= 1;
		if(min + res > max) {
			max = min + res;
		}
		arr = langM_realloc(elHEAP,sizeof(Array)+per*max,arr);
	}
	if (arr != 0) {
		arr->max = max;
		arr->min = min + com;
	}
	*var = arr + 1;
	return min;
}
