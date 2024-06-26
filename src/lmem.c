/*
** See Copyright Notice In lang.h
** lmem.c
** Memory Tools
*/


/* todo: can we do this some other way? */
lglobaldecl Alloc langM_tlocalalloc = {"default-temp-allocator",langM_deftlocalallocfn};
lglobaldecl Alloc langM_globalalloc = {"default-heap-allocator",langM_defglobalallocfn};



/* todo: should prob migrate to using something
like stb leak */
void *langM_clear(void *target, llong length) {
#if defined(_WIN32)
	ZeroMemory(target,length);
#else
	memset(target,0,length);
#endif
	return target;
}


void *langM_copy(void *target, void const *source, llong length) {
#if defined(_WIN32)
	CopyMemory(target,source,length);
#else
	memcpy(target,source,length);
#endif
	return target;
}


void langM_dealloc_(Alloc *c, const void *memory, ldebugloc loca) {

	Error error = c->fn(c,0,0,0,(void **)&memory,loca);
	LASSERT(LPASSED(error));
}


void *langM_alloc_(Alloc *c, llong length, ldebugloc loca) {
	void *memory = 0;
	Error error = c->fn(c,0,0,length,&memory,loca);
	LASSERT(LPASSED(error));
	return memory;
}


void *langM_realloc_(Alloc *c, llong length, void *memory, ldebugloc loca) {

	Error error = c->fn(c,0,0,length,&memory,loca);
	LASSERT(LPASSED(error));
	return memory;
}


void *langM_clearalloc_(Alloc *c, llong size, ldebugloc loca) {
	return langM_clear(langM_alloc_(c,size,loca),size);
}


#if 0
#if defined(_DEBUG)
MemBlock *M_free;
llong M_nfree;
llong M_length;
llong M_cursor;
unsigned char *M_memory;
#endif
#endif

#if defined(_DEBUG)
void langM_initmemory() {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF|_CRTDBG_REPORT_FLAG);
	_CrtSetReportMode(_CRT_ERROR,_CRTDBG_MODE_DEBUG);
#if 0
	M_length = GIGABYTES(4);
	M_memory = sys_valloc(M_length);
#endif
}
#else
	#define langM_initmemory()
#endif


ALLOCFN(langM_defglobalallocfn) {
	if (oldAndNewMemory == 0) {
		return Error_InvalidArguments;
	}
	if (newSize == 0) {
		_aligned_free(*oldAndNewMemory);
	} else {
		if (*oldAndNewMemory == 0) {
			*oldAndNewMemory = _aligned_malloc(newSize,0x10);
		} else {
			*oldAndNewMemory = _aligned_realloc(*oldAndNewMemory,newSize,0x10);
		}

		if (*oldAndNewMemory == 0) {
			return Error_OutOfMemory;
		}
	}
	return Error_None;
}


ALLOCFN(langM_deftlocalallocfn) {
	if (oldAndNewMemory == 0) {
		return Error_InvalidArguments;
	}
	if (newSize == 0) {
		return Error_InvalidArguments;
	} else {
		/* reallocation is not permitted */
		if (*oldAndNewMemory != 0) {
			return Error_InvalidArguments;
		}

		// TODO:
		lthreaddecl char memory[0x10000];
		lthreaddecl char *cursor = 0;
		if (cursor == 0) cursor = memory;

		if (newSize > sizeof(memory)) {
			return Error_OutOfMemory;
		}

		if((cursor - memory) + newSize > sizeof(memory)) {
			cursor = memory;
		}

		*oldAndNewMemory = cursor;
		cursor += newSize;
	}
	return Error_None;
}



#if 0
void langM_checkmemptr(void *mem) {
	if (mem == 0) return;

	MemBlock *file = (MemBlock *) mem - 1;

	MemBlock *then;
	for (then = M_free; then != 0; then = then->then) {
		if (then == file) {
			ldebugloc loca = file->freeloca;
			lang_logerror("%s %s(), %i: memory file was already freed here"
			, loca.fileName,loca.func,loca.lineNumber);
			break;
		}
	}

	if (file->headtrap != FLYTRAP || file->foottrap != FLYTRAP) {
		ldebugloc loca = file->loca;
		lang_logerror("%s %s(), %i: invalid file, %x, %x"
		, loca.fileName,loca.func,loca.lineNumber
		, file->headtrap, file->foottrap);
	}

	llong contentssize = file->contentssize;
	llong chunkcatedsize = CHUNKCATE(contentssize,CHUNKSIZE);
	for (int i = contentssize; i < chunkcatedsize; ++i) {
		LASSERT(((unsigned char *)mem)[i] == (FLYTRAP & 0xFF));
	}

}


void langM_debugdealloc(void *mem, ldebugloc loca) {
	if (mem == 0) return;
	langM_checkmemptr(mem);

	MemBlock *file = (MemBlock *) mem - 1;
	file->freeloca = loca;
	file->then = M_free;
	M_free = file;
	++ M_nfree;
}


void *langM_debugalloc(llong contentssize, ldebugloc loca) {
	#if 0
	MemBlock *then;
	for (then = M_free; then != 0; then = then->then) {
		if (then->size >= size) {
			then->loca = loca;
			return then+1;
		}
	}
	#endif

	llong chunkcatedsize = CHUNKCATE(contentssize+256,CHUNKSIZE);
	LASSERT(chunkcatedsize >= 512);
	// lang_loginfo("alloc %lli",contentssize);

	llong totalsize = sizeof(MemBlock)+chunkcatedsize;

	LASSERT(M_cursor + totalsize <= M_length);

	MemBlock *block = (MemBlock *) (M_memory + M_cursor);

	unsigned char *mem = (unsigned char *) (block + 1);
	for (int i = contentssize; i < chunkcatedsize; ++i) {
		mem[i] = FLYTRAP & 0xFF;
	}

	block->headtrap = FLYTRAP;
	block->foottrap = FLYTRAP;
	block->contentssize = contentssize;
	block->then = 0;
	block->loca = loca;
	block->freeloca = loca;

	M_cursor += totalsize;

	return block+1;
}


void *langM_debugrealloc(void *mem, llong contentssize, ldebugloc loca) {
	if (mem == 0) return langM_debugalloc(contentssize,loca);

	MemBlock *file = (MemBlock *) mem - 1;
	langM_debugdealloc(mem,loca);
	void *newmem = langM_debugalloc(contentssize,loca);
	memcpy(newmem,mem,file->contentssize);

	// lang_loginfo("realloc: %lli -> %lli",file->size,contentssize);

	return newmem;
}

#endif
