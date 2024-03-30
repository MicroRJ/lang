

#include <Windows.h>
#include <stdio.h>


#define pf printf

typedef int (*jit_fn)(int);


unsigned char *jit_mem;
int jit_cur;


#define OP_MOV  0x89
#define OP_RET  0xC3
#define OP_CALL 0xE8
/* jump relative to rip, rel32 operand */
#define OP_JMP  0xE9
#define OP_PSH  0x50

enum {
	REG_EAX = 0,
	REG_ECX,
	REG_EDX,
	REG_EBX,
	REG_ESP,
	REG_EBP,
	REG_ESI,
	REG_EDI,
};


char *REG_NAME[] = {
	"eax","ecx","edx","ebx","esp","ebp","esi","edi",
};



#define p1(x) (jit_mem[jit_cur++]=x)
#define p2(x,y) (p1(x),p1(y))
#define p3(x,y,z) (p2(x,y),p1(z))


#define DO_CALL(x) p2(OP_CALL,x)
#define DO_MOV_REG_REG(x,y) p2(OP_MOV,0xC0|(x)|((y)<<3))

#define DO_MOV_EAX_ECX DO_MOV_REG_REG(REG_EAX,REG_ECX)

#define DO_RET p1(0xc3)



char *jit_alloc(int length) {
	return VirtualAlloc(NULL,length,MEM_RESERVE|MEM_COMMIT,PAGE_EXECUTE_READWRITE);
}



void jit_dasm(unsigned char *mem) {
	unsigned char *cur = mem;
	while (*cur != 0) {
		switch (*cur ++) {
			case OP_CALL: {
				printf("call");
			} break;
			case OP_RET: {
				printf("ret");
			} break;
			case OP_MOV: {
				printf("mov ");
				unsigned char mrm = *cur ++;
				if ((mrm >> 6) == 0b01) {
					printf("mem, reg");
				} else
				if ((mrm >> 6) == 0b11) {
					int dst = (mrm & 0b00000111) >> 0;
					int src = (mrm & 0b00111000) >> 3;
					printf("%s, %s",REG_NAME[dst],REG_NAME[src]);
				}
			} break;
		}
		printf("\n");
	}
}




void main() {
	pf("%lli/%lli\n", sizeof(long),sizeof(int));

	jit_mem = jit_alloc(4096);
	jit_fn f = (jit_fn) jit_mem;

	DO_MOV_EAX_ECX;
	DO_RET;

	pf("disassembly:\n");
	jit_dasm(jit_mem);

	pf("f(777) => %i\n", f(777));
	pf("f(888) => %i\n", f(888));
	pf("f(999) => %i\n", f(999));
	// pf("\n\nHello, World!\n\n\n");
}