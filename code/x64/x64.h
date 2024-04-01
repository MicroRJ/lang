/*
** See Copyright Notice In lang.h
** x64.h
** JIT Experiments
*/


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


/*
** No displacement register addressing mode
*/
#define MODRM_MOD_NODISP  0xC0
/*
** 8bit displacement addressing mode
*/
#define MODRM_MOD_8DISP  0x40
/*
** 8bit displacement addressing mode
*/
#define MODRM_MOD_16DISP 0x80

#define MODRM_RR(x,y) (MODRM_MOD_NODISP|(x)|((y)<<3))

#define MODRM_8DISP(x,y) (MODRM_MOD_8DISP|(x)|((y)<<3))


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


char *REG_NAME[] = {
	"eax","ecx","edx","ebx","esp","ebp","esi","edi",
};



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

/*
**
*/
#define DO_MOV32_REG_MEM_8DISP(z,x,y) \
LINE((p2(0x8B,MODRM_8DISP(x,z)), NEXT, p1(y)))
/*
** Example:
** mov DWORD PTR [rbp-4], 1
**
** x is base register
** y is offset
** z is value
*/
#define DO_MOV32_MEM_IMM_8DISP(x,y,z) LINE((p2(0xC7,MODRM_8DISP(x,0)), NEXT, p1(y), NEXT, pu32(z)))


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

