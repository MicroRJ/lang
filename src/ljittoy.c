/*
** See Copyright Notice In lang.h
** ljittoy.c
** JIT Experiments
*/

#define pf printf

typedef uintptr_t jit_llu;
typedef int (*jit_fn)(int);
unsigned char *jit_mem;
int jit_cur;


/* -- todo: this is a security risk, instead allocate
regular memory, then after all code has been generated,
make readonly and executable */
char *jit_alloc(int length) {
	return VirtualAlloc(NULL,length,MEM_RESERVE|MEM_COMMIT,PAGE_EXECUTE_READWRITE);
}



void p3(unsigned char x, unsigned char y, unsigned char z) {
	pf("%02x %02x %02x",x,y,z);
	jit_mem[jit_cur ++] = x;
	jit_mem[jit_cur ++] = y;
	jit_mem[jit_cur ++] = z;
}


void p2(unsigned char x, unsigned char y) {
	pf("%02x %02x",x,y);
	jit_mem[jit_cur ++] = x;
	jit_mem[jit_cur ++] = y;
}


void p1(unsigned char x) {
	pf("%02x",x);
	jit_mem[jit_cur ++] = x;
}


#define pu64(x) puX(x,8)
#define pu32(x) puX(x,4)
void puX(jit_llu x, int n) {
	while (n --) {
		pf("%02x ",(unsigned char)(x));
		jit_mem[jit_cur ++] = x & 0xff;
		x >>= 8;
	}
}


void foo() {
	printf("this function was called, but who did it?\n");
}


void jittestall() {
	DO_PUSH_RBP();
	DO_MOV64_REG_REG(REG_RBP,REG_RSP);
	DO_MOV64_REG_REG(REG_EDI,REG_ECX);
	DO_MOV32_MEM_IMM_8DISP(REG_EBP,+4,1);
	DO_MOV32_MEM_REG_8DISP(REG_EBP,+4,REG_ECX);
	DO_MOV32_REG_MEM_8DISP(REG_ECX,REG_EBP,-4);
	DO_MOV64_REG_IMM(REG_EAX,1);
	DO_CMP32_IMM(REG_EAX,2);
	DO_JL_NEAR32(/*self*/5+1);
	DO_SUB32_IMM(REG_EAX,777);
	DO_SUB32_REG(REG_EAX,REG_ECX);
	DO_ADD32_REG(REG_EAX,REG_ECX);
	DO_MOV64_REG_IMM(REG_EAX,(jit_llu)&foo);
	DO_CALL_REG64(REG_EAX);
	DO_MOV32_REG_REG(REG_EAX,REG_EDI);
	DO_POP_RBP();
	DO_RET();
}

#if 0
void do_add(elf_val *x, elf_val *y) {
	if (x->tag == TAG_NUM) {
		x->n = elf_iton(* x) + elf_iton(* y);
	} else
	if (x->tag == TAG_INT) {
		x->i = elf_ntoi(* x) + elf_ntoi(* y);
	}
}
#endif


elf_int do_add(elf_int x, elf_int y) {
	pf("do add %lli, %lli\n", x, y);
	return x + y;
}


void emit86_mov(ljValue x, ljValue y) {
	if (x.type == JIT_GPR) {
		if (y.type == JIT_MEM) {
			DO_MOV64_REG_MEM_8DISP(x.base,y.base,y.disp);
		} else if (y.type == JIT_IMM) {
			DO_MOV64_REG_IMM(x.base,y.immediate);
		} else LNOBRANCH;
	} else
	if (x.type == JIT_MEM) {
		if (y.type == JIT_MEM) {
			emit86_mov(REG(RAX),y);
			y = REG(RAX);
		} else LASSERT(y.type == JIT_GPR);
		DO_MOV64_MEM_REG_8DISP(x.base,x.disp,y.base);
	} else LNOBRANCH;
}


void emit86_logxor(ljValue x, ljValue y) {
	ljValue yy = y;
	if (yy.type == JIT_MEM) emit86_mov(y = REG(RAX),yy);

	if (x.type == JIT_GPR && y.type == JIT_GPR) {
		LINE((p2(0x31,MODRM_RR(x.base,y.base))));
	} else
	if (x.type == JIT_MEM && y.type == JIT_GPR) {
		LINE((p2(0x31,MODRM_8DISP(x.base,y.base)), NEXT, p1(x.disp)));
	} else LNOBRANCH;

	if (yy.type == JIT_MEM) emit86_mov(yy,y);
}


void emit86_shift(lbyteop type, ljValue x, ljValue y) {
	ljValue xx = x;
	if (xx.type == JIT_MEM) emit86_mov(x = REG(RAX),xx);
	if (x.type == JIT_GPR && y.type == JIT_IMM) {
		int v = type == BC_SHL ? 4 : 5;
		LINE((p2(0xC1,MODRM_RR(x.base,v)), NEXT, p1(y.immediate)));
	} else LNOBRANCH;
	if (xx.type == JIT_MEM) emit86_mov(xx,x);
}


lBinding jit(elf_Module *md, elf_Proto fn) {
	pf("jitting fn\n");
	int loc = ((fn.nlocals*8+15)/16)*16;
	DO_PUSH_RBP();
	DO_MOV64_REG_REG(REG_RBP,REG_RSP);
	/* todo: instead should use, sub 64-bit reg 8 bit operand */
	DO_SUB64_IMM(REG_RSP,loc);

	DO_MOV32_MEM_REG_8DISP(REG_RBP,-8,REG_ECX);

	lBytecode *bytes = md->bytes + fn.bytes;
	lbyteid nbytes = fn.nbytes;

	for (lbyteid i = 0; i < nbytes; ++i) {
		#if 0
		switch (byte.k) {
			case BC_INT: {
				stack[top].type = JIT_IMM;
				stack[top].immediate = byte.i;
				++ top;
			} break;
			case BC_RELOAD: {
				for (int y = 0; y < byte.y; ++ y) {
					stack[top].type = JIT_MEM;
					stack[top].base = RBP;
					stack[top].disp = - byte.x * 8 + y * 8 - 8;
					++ top;
				}
			} break;
			case BC_RELOAD: {
				ljValue y = stack[top-1];
				-- top;
				emit86_mov(MEM(RBP,- byte.i * 8 - 8), y);
			} break;
			case BC_ADD: {
				emit86_mov(REG(RCX),stack[top-2]);
				emit86_mov(REG(RDX),stack[top-1]);
				top -= 1;
				emit86_mov(REG(RAX),PTR(&do_add));
				DO_CALL_REG64(REG_EAX);
				stack[top++] = REG(RAX);
			} break;
			case BC_SHR:
			case BC_SHL: {
				ljValue x = stack[top-2];
				ljValue y = stack[top-1];
				-- top;
				emit86_shift(byte.k,x,y);
			} break;
			case BC_XOR: {
				ljValue x = stack[top-2];
				ljValue y = stack[top-1];
				-- top;
				emit86_logxor(x,y);
			} break;
			case BC_YIELD: {
				emit86_mov(REG(RAX),stack[--top]);
			} break;
			case BC_LEAVE:
			break;
			default: LNOBRANCH;
		}
		(void) byte;
		#endif
	}

	DO_LEAVE();
	DO_RET();
	pf("ended\n");

	typedef int (*xorfn)(int);
	int result = ((xorfn)jit_mem)(5282);
	pf("result: %i\n",result);
	return (lBinding) jit_mem;
}


void jittest() {
	jit_mem = jit_alloc(4096);
	jit_fn f = (jit_fn) jit_mem;

	// DO_PUSH_RBP();
	// DO_MOV64_REG_REG(REG_RBP,REG_RSP);
	// jitter("^a+ab=ba~a");
	// DO_LEAVE();
	// DO_RET();
	// jitdectozero();
	// jittestall();
	// pf("disassembly:\n");
	// jit_dasm(jit_mem);
	// pf("f(777) => %i\n", f(777));
	// pf("f(888) => %i\n", f(888));
	// pf("f(999) => %i\n", f(999));
	// pf("\n\nHello, World!\n\n\n");
}