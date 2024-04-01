/*
** See Copyright Notice In lang.h
** ljit.c
** JIT Experiments
*/


#include <Windows.h>
#include <stdio.h>
#include <lang.h>


#define pf printf

typedef int (*jit_fn)(int);
unsigned char *jit_mem;
int jit_cur;


#include <ljit.h>
#include <x64.h>

typedef uintptr_t jit_llu;


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


// void dectozero(int n) {
// 	if (n <= 1) return n;
// 	return dectozero(n - 1);
// }
void jitdectozero() {
	DO_PUSH_RBP();
	DO_MOV64_REG_REG(REG_RBP,REG_RSP);
	// sub rsp, #16
	DO_SUB64_IMM(REG_RSP,16);
	// mov DWORD PTR[rbp+4], ecx
	DO_MOV32_MEM_REG_8DISP(REG_RBP,4,REG_ECX);
	// cmp ecx, #0
	DO_CMP32_IMM(REG_ECX,0);
	// jne
	DO_JNE_NEAR32(5+2);
	DO_MOV32_REG_REG(REG_EAX,REG_ECX);
	// DO_JMP_NEAR32();
	DO_MOV32_REG_MEM_8DISP(REG_EAX,4,REG_ECX);
	DO_LEAVE();
	// DO_POP_RBP();
	DO_RET();
}


void jitter(char *p) {
	char m[] = {REG_ECX,REG_EAX,REG_EDI};
	char x,y;
	while (*p) switch (*p++) {
		case '~': {
			x = m[*p++ - 'a'];
			DO_ADD64_IMM(REG_RSP,4);
			DO_MOV32_REG_MEM_8DISP(0,REG_RSP,0);
		} break;
		case '^': {
			x = m[*p++ - 'a'];
			DO_SUB64_IMM(REG_RSP,4);
			DO_MOV32_MEM_REG_8DISP(REG_RSP,4,x);
		} break;
		case '=': {
			x = m[*p++ - 'a'];
			if (*p == '0' || *p == '1') {
				y = *p ++ - '0';
				DO_MOV64_REG_IMM(x,y);
			} else {
				y = m[*p++ - 'a'];
				DO_MOV32_REG_REG(x,y);
			}
		} break;
		case '-': {
			x = m[*p++ - 'a'];
			y = m[*p++ - 'a'];
			DO_SUB32_REG(x,y);
		} break;
		case '+': {
			x = m[*p++ - 'a'];
			y = m[*p++ - 'a'];
			DO_ADD32_REG(x,y);
		} break;
	}
}



void main() {
	pf("%lli/%lli\n", sizeof(uintptr_t),sizeof(int));

	jit_mem = jit_alloc(4096);
	jit_fn f = (jit_fn) jit_mem;



	// DO_PUSH_RBP();
	// DO_MOV64_REG_REG(REG_RBP,REG_RSP);
	// jitter("^a+ab=ba~a");
	// DO_LEAVE();
	// DO_RET();
	// jitdectozero();
	jittestall();
	// pf("disassembly:\n");
	// jit_dasm(jit_mem);


	pf("f(777) => %i\n", f(777));
	// pf("f(888) => %i\n", f(888));
	// pf("f(999) => %i\n", f(999));
	// pf("\n\nHello, World!\n\n\n");
}