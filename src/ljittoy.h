/*
** See Copyright Notice In elf.h
** ljittoy.h
** JIT Experiments (x86_64)
*/

/* 4.2.24
	-- DEAR DIARY,  ----------------------------------
	----------------------------
	--	Generating machine code from this language
	will probably take me a little while...
	this being a dynamic programming language does
	not seem to help expedite the process and well
	of course, this is my first time toying around
	with this sort of stuff.
	So yes, this was probably not the best way to
	get started.
	Either way, I'll use this project to just play
	around, and attempt to get good feel for it,
	after all this tends to be how I learn things,
	and I therefore trust the process.
	Eventually I do want focus on actually making
	a JIT compiler that's good, like LuaJIT, even
	though those are big shoes to fill, it seems
	plausible...
	Even if we don't actually do jitting for this
	language, along the way we can still reuse
	what we've learned and apply it to some other
	simple type checked programming language and
	generate machine code directly, which should
	be simpler, we can even use someone else's
	front end... Seems like I just keep stalling,
	by writing comments like this.

	-- Road Map:
	- Some sort of albeit simple (IR) that we can
	use to apply optimizations passes and generate
	machine instructions for multiple backends.
	- Implied would be, a good register allocator,
	and some simple optimization passes.

	-- Further research is necessitated in the
	following fields:
	- optimal register allocation
	- instruction encoding
	- ABI's, CC's and all that stuff
	- CPU cache lines?
	- CPU stack alignment
*/


#define NO_NODE 0


/* -- Yet another extremely simple graph for the IR,
- is quite counter intuitive to have to generate
- another graph from the bytecode, but this seems
- to be a fundamental step in generating semi-decent
- code...
- That's at-least what I see is done everywhere,
- either way I think is easier this way too, despite
- the redundancies, if anything we can always reuse
- this for some other project and generate
- instructions directly.
- Will point out that this whole thing is being
- modeled after my own understanding of these
- concepts and of course in reference to the
- great LuaJIT project by Mike Pall...
- */
typedef enum {
	NODE_NONE = 0,
	NODE_DATA,
	NODE_LOAD,
	NODE_LOCAL,
	NODE_STORE,
} ljNodeOp;


typedef enum {
	DATA_VOID = 0,
	DATA_INT64,
	DATA_FLT64,
	DATA_PTR,
} ljDataTy;



typedef int ljnodeid;



typedef struct ljNode {
	ljNodeOp  op;
	ljDataTy  ty;
	ljnodeid x,y;
	elf_val    lv;
	elf_int  li;
} ljNode;



typedef struct ljState {
	ljNode *nodes;
	int nnodes;
} ljState;


typedef enum ljValType {
	JIT_NON = 0,
	JIT_IMM,
	JIT_MEM,
	JIT_GPR,
} ljValType;



typedef struct ljValue {
	ljValType type;
	int base;
	int disp;
	int index, scale;
	elf_int immediate;
	ljnodeid node;
} ljValue;




ljnodeid alloc_node(ljState *lj, ljNodeOp op, ljDataTy ty, ljnodeid x, ljnodeid y) {
	ljnodeid id = lj->nnodes ++;
	lj->nodes[id].op = op;
	lj->nodes[id].ty = ty;
	lj->nodes[id].x = x;
	lj->nodes[id].y = y;
	return id;
}


ljnodeid emit_irlongint(ljState *lj, elf_int li) {
	ljnodeid id = alloc_node(lj,NODE_DATA,DATA_INT64,NO_NODE,NO_NODE);
	lj->nodes[id].li = li;
	return id;
}


ljnodeid emit_irloadlocal(ljState *lj, ljDataTy ty, ljnodeid x, ljnodeid y) {
	return alloc_node(lj,NODE_LOAD,ty,x,y);
}


ljnodeid emit_irloadintolocal(ljState *lj, ljDataTy ty, ljnodeid x, ljnodeid y) {
	return alloc_node(lj,NODE_STORE,ty,x,y);
}




/* -- ISA STUFF (x86_64) */

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


#define REG_RSP REG_ESP
#define REG_RBP REG_EBP


typedef enum {
	GPR_NONE = -1,
	RAX, RCX, RDX, RBX, RSP, RBP, RSI, RDI,
	R8,  R9,  R10, R11, R12, R13, R14, R15,
} lj86Reg;


elf_globaldecl const char* GPR_NAMES[] = {
	"RAX", "RCX", "RDX", "RBX", "RSP", "RBP", "RSI", "RDI"
	,"R8",  "R9", "R10", "R11", "R12", "R13", "R14", "R15"
};





#define REG(gpr) ((ljValue) {JIT_GPR,gpr})
#define MEM(gpr,off) ((ljValue) {JIT_MEM,gpr,off})
#define PTR(ptr) ((ljValue) {JIT_IMM,0,0,0,0,(elf_int)(ptr)})




/*
** Escape opcode prefixes are used for opcodes
** that are more than one byte in length.
** 0x0F is mandatory for 2 byte opcodes.
*/
#define ESC2 0x0F


#define REX   0x40


/*
** REC.W (|(0x08)) Means 64-Bit Operand Size
*/
#define REX_W (REX|0x08)


/*
**
** Mod(R/M) Bit Layout:
**
** [[0,0][000][000]]
**   7 6  5 3  2 0
**
** Bits   Usage
** 7, 6   Mod (Addressing mode)
** 5..3   Reg or Opcode extension
** 2..0   Reg or Memory
**
*/


typedef enum {
	MODRM_REG 		= 0xC0,
	MODRM_MEM8OFF 	= 0x40,
	MODRM_MEM16OFF = 0x80,
} lj86Mode;


#define MODRM(z,x,y) ((z)|(x)|(y)<<3)
#define MODRM_RR(x,y) MODRM(MODRM_REG,x,y)
#define MODRM_8DISP(x,y) MODRM(MODRM_MEM8OFF,x,y)


/*
** SIB.Scale 2bits
** SIB.Index 3bits (register)
** SIB.Base  3bits (register)
*/
#define SIB(x,y,z) (((x) << 6)|((y) << 3)|((z) << 0))






#define OP_RET  	0xC3


/* jump relative to rip, rel32 operand */
#define OP_JMP  0xE9
#define OP_PSH  0x50
#define OP_POP  0x58

#define OP_CALL_REL32 0xE8
#define OP_CALL_REG 	 0xFF




#define NEXT (pf(", "))

#define LINE(x) (x, pf("\n"))



/*
** Call variant /2, which calls function pointer
** on register.
*/
#define MODRM_RR_CALL2(x) MODRM_RR(x,0x02)



// #define DO_NOP()

#define DO_PUSH_REG(x) LINE(p1(OP_PSH + (x)))
#define DO_PUSH_RBP() DO_PUSH_REG(REG_RBP)

#define DO_POP_REG(x) LINE(p1(OP_POP + (x)))
#define DO_POP_RBP() DO_POP_REG(REG_RBP)

#define DO_RET() LINE(p1(OP_RET))
#define DO_LEAVE() LINE(p1(0xC9))

#define DO_CALL_REL32(x) LINE(p2(OP_CALL_REL32),pu32(x))
#define DO_CALL_REG64(x) LINE(p2(OP_CALL_REG,MODRM_RR_CALL2(x)))



#define DO_MOV32_REG_REG(x,y) LINE(p2(0x89,MODRM_RR(x,y)))
/*
** Example:
** mov rax, ecx
*/
#define DO_MOV64_REG_REG(x,y) LINE(p3(REX_W,0x89,MODRM_RR(x,y)))
/*
**
** x: base register
** y: offset
** z: source register
**
**
** Example:
** mov DWORD PTR [rbp-4], rcx
**
** DO_MOV32_MEM_REG_8DISP(REG_RBP,-4,REG_RCX);
**
*/
#define DO_MOV32_MEM_REG_8DISP(x,y,z) \
LINE((p2(0x89,MODRM_8DISP(x,z)), NEXT, p1(y)))
#define DO_MOV64_MEM_REG_8DISP(x,y,z) \
LINE((p3(REX_W,0x89,MODRM_8DISP(x,z)), NEXT, p1(y)))
/*
**
*/
#define DO_MOV32_REG_MEM_8DISP(z,x,y) \
LINE((p2(0x8B,MODRM_8DISP(x,z)), NEXT, p1(y)))
#define DO_MOV64_REG_MEM_8DISP(z,x,y) \
LINE((p3(REX_W,0x8B,MODRM_8DISP(x,z)), NEXT, p1(y)))

/*
** Example:
** mov DWORD PTR [rbp-4], 1
**
** x is base register
** y is offset
** z is value
*/
#define DO_MOV32_MEM_IMM_8DISP(x,y,z) LINE((\
/* */p2(0xC7,MODRM_8DISP(x,0)), NEXT, p1(y), NEXT, pu32(z)))

#define DO_MOV64_MEM_IMM32(x,y) LINE((\
/* */p3(REX_W,0xC7,MODRM_8DISP(x,0)), NEXT, p1(y), NEXT, pu32(y)))


/*
** Where x is the target register and y is
** the 64-bit immediate operand.
** Requires REX_W prefix, lower 3 bits of
** opcode are used to to encode target.
*/
#define DO_MOV64_REG_IMM(x,y) LINE((p2(REX_W,0xB8|(x)), NEXT, pu64(y)))



/*
**
** Example:
** | sub eax,309h
**
** Description:
** Subtracts 32bit immediate operand (y)
** from register (x).
**
** It evaluates the result for both signed
** and unsigned integer operands and sets
** the OF and CF flags to indicate an overflow
** in the signed or unsigned result, respectively.
** The SF flag indicates the sign of the signed
** result.
**
** Encoding:
** 0x81 opcode uses reg/opcode field of
** Mod(R/M) byte to encode opcode variant 5.
** R/M field contains target register.
**
*/
#define DO_SUB32_IMM(x,y) LINE((\
/* */p2(0x81,MODRM_RR(x,0x05)), NEXT, pu32(y)))

#define DO_SUB64_IMM(x,y) LINE((\
/* */p3(REX_W,0x81,MODRM_RR(x,0x05)), NEXT, pu32(y)))

#define DO_CMP32_IMM(x,y) LINE((\
/* */p2(0x81,MODRM_RR(x,0x07)), NEXT, pu32(y)))


/*
**
** Example:
** | sub eax,ecx
**
** Description:
** Subtracts 32bit register (y) from register (x).
**
*/
#define DO_SUB32_REG(x,y) LINE((\
/* */p2(0x2B,MODRM_RR(x,y))))

#define DO_ADD32_REG(x,y) LINE((\
/* */p2(0x03,MODRM_RR(x,y))))

#define DO_ADD64_IMM(x,y) LINE((\
/* */p3(REX_W,0x81,MODRM_RR(x,0x00)), NEXT, pu32(y)))



#define DO_JNE_NEAR32(x) LINE((\
/* */p2(ESC2,0x85), NEXT, pu32(x)))

#define DO_JL_NEAR32(x) LINE((\
/* */p2(ESC2,0x8C), NEXT, pu32(x)))

