/*
** See Copyright Notice In elf.h
** elf-var.c
** variable array tool
*/


elf_int elf_varaddxx(void **var, elf_int per, elf_int res, elf_int com) {
	elf_int max = 0;
	elf_int min = 0;
	elf_var *arr = 0;
	if (*var != 0) {
		arr = ((elf_var*)(*var)) - 1;
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
		arr = langM_realloc(lHEAP,sizeof(elf_var)+per*max,arr);
	}
	if (arr != 0) {
		arr->max = max;
		arr->min = min + com;
	}
	*var = arr + 1;
	return min;
}
