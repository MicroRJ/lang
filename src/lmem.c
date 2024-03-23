/*
** See Copyright Notice In lang.h
** lmem.c
** Memory Tools
*/



void *langM_clear(void *target, Integer length) {
#if defined(_WIN32)
	ZeroMemory(target,length);
#else
	memset(target,0,length);
#endif
	return target;
}


void *langM_copy(void *target, void const *source, Integer length) {
#if defined(_WIN32)
	CopyMemory(target,source,length);
#else
	memcpy(target,source,length);
#endif
	return target;
}


void langM_dealloc_(Alloc *c, const void *memory, Debugloc loca) {

	Error error = c->fn(c,0,0,0,(void **)&memory,loca);
	LASSERT(LPASSED(error));
}


void *langM_alloc_(Alloc *c, Integer length, Debugloc loca) {
	void *memory = 0;
	Error error = c->fn(c,0,0,length,&memory,loca);
	LASSERT(LPASSED(error));
	return memory;
}


void *langM_realloc_(Alloc *c, Integer length, void *memory, Debugloc loca) {

	Error error = c->fn(c,0,0,length,&memory,loca);
	LASSERT(LPASSED(error));
	return memory;
}


void *langM_clearalloc_(Alloc *c, Integer size, Debugloc loca) {
	return langM_clear(langM_alloc_(c,size,loca),size);
}

#if defined(_DEBUG)
MemBlock *M_free;
Integer M_nfree;
Integer M_length;
Integer M_cursor;
unsigned char *M_memory;
#endif

#if defined(_DEBUG)
void langM_initmemory() {

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF|_CRTDBG_REPORT_FLAG);
	_CrtSetReportMode(_CRT_ERROR,_CRTDBG_MODE_DEBUG);

	M_length = GIGABYTES(4);
	M_memory = sys_valloc(M_length);
}
#else
	#define langM_initmemory()
#endif


void langM_checkmemptr(void *mem) {
	if (mem == 0) return;

	MemBlock *file = (MemBlock *) mem - 1;

	#if 0
	MemBlock *then;
	for (then = M_free; then != 0; then = then->then) {
		if (then == file) {
			Debugloc loca = file->freeloca;
			lang_logerror("%s %s(), %i: memory file was already freed here"
			, loca.fileName,loca.func,loca.lineNumber);
			break;
		}
	}
	#endif

	if (file->headtrap != FLYTRAP || file->foottrap != FLYTRAP) {
		Debugloc loca = file->loca;
		lang_logerror("%s %s(), %i: invalid file, %x, %x"
		, loca.fileName,loca.func,loca.lineNumber
		, file->headtrap, file->foottrap);
	}

	Integer contentssize = file->contentssize;
	Integer chunkcatedsize = CHUNKCATE(contentssize,CHUNKSIZE);
	for (int i = contentssize; i < chunkcatedsize; ++i) {
		LASSERT(((unsigned char *)mem)[i] == (FLYTRAP & 0xFF));
	}

}


void langM_debugdealloc(void *mem, Debugloc loca) {
	if (mem == 0) return;
	langM_checkmemptr(mem);

	MemBlock *file = (MemBlock *) mem - 1;
	file->freeloca = loca;
	file->then = M_free;
	M_free = file;
	++ M_nfree;
}


void *langM_debugalloc(Integer contentssize, Debugloc loca) {
	#if 0
	MemBlock *then;
	for (then = M_free; then != 0; then = then->then) {
		if (then->size >= size) {
			then->loca = loca;
			return then+1;
		}
	}
	#endif

	Integer chunkcatedsize = CHUNKCATE(contentssize+256,CHUNKSIZE);
	LASSERT(chunkcatedsize >= 512);
	// lang_loginfo("alloc %lli",contentssize);

	Integer totalsize = sizeof(MemBlock)+chunkcatedsize;

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


void *langM_debugrealloc(void *mem, Integer contentssize, Debugloc loca) {
	if (mem == 0) return langM_debugalloc(contentssize,loca);

	MemBlock *file = (MemBlock *) mem - 1;
	langM_debugdealloc(mem,loca);
	void *newmem = langM_debugalloc(contentssize,loca);
	memcpy(newmem,mem,file->contentssize);

	// lang_loginfo("realloc: %lli -> %lli",file->size,contentssize);

	return newmem;
}



ALLOCFN(langM_defgloballocfn) {
	if (oldAndNewMemory == 0) {
		return Error_InvalidArguments;
	}
	if (newSize == 0) {
		_aligned_free(*oldAndNewMemory);
		// langM_debugdealloc(*oldAndNewMemory,loca);
	} else {
		// *oldAndNewMemory = langM_debugrealloc(oldAndNewMemory ? *oldAndNewMemory : 0,newSize,loca);

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


ALLOCFN(langM_deftlsallocfn) {
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
		lglobal char memory[0x10000];
		lglobal char *cursor = memory;

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