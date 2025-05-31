#pragma once
#include <emu/common.h>

struct bus;

struct instruction;

typedef struct {
    u8 a;
    u8 x;
    u8 y;
    u8 stkp;
    u16 pc;
    u8 status;

    u8 fetched;
    u8 temp;
    u16 addr_abs;
    u16 addr_rel;
    u8 opcode;
    u8 cycles;
    u32 clock_count;

    struct bus* b;
    struct instruction* lookup;
} nes6502;

typedef struct instruction {
    const char* name;
    u8 (*operate)(nes6502* nes);
    u8 (*addrmode)(nes6502* nes);
    u8 cycles;
} instruction;

nes6502 nes6502_init();
void nes6502_destroy(nes6502* nes);

void nes6502_reset(nes6502* nes);
void nes6502_irq(nes6502* nes);
void nes6502_nmi(nes6502* nes);
void nes6502_clock(nes6502* nes);

u8 nes6502_complete(nes6502* nes);

void nes6502_connect_bus(nes6502* nes, struct bus* b);

void nes6502_disassemble(nes6502* nes, u16 start, u16 stop);

typedef enum {
    FLAGS_6502_C = (1 << 0),
    FLAGS_6502_Z = (1 << 1),
    FLAGS_6502_I = (1 << 2),
    FLAGS_6502_D = (1 << 3),
    FLAGS_6502_B = (1 << 4),
    FLAGS_6502_U = (1 << 5),
    FLAGS_6502_V = (1 << 6),
    FLAGS_6502_N = (1 << 7)
} flags_6502;

u8 nes6502_get_flag(nes6502* nes, flags_6502 f);
void nes6502_set_flag(nes6502* nes, flags_6502 f, u8 v);

u8 nes6502_read(nes6502* nes, u16 a);
void nes6502_write(nes6502* nes, u16 a, u8 d);

u8 nes6502_fetch(nes6502* nes);

// -- ADDRESSING MODES --

u8 IMP(nes6502* nes);
u8 IMM(nes6502* nes);	
u8 ZP0(nes6502* nes);
u8 ZPX(nes6502* nes);	
u8 ZPY(nes6502* nes);
u8 REL(nes6502* nes);
u8 ABS(nes6502* nes);
u8 ABX(nes6502* nes);	
u8 ABY(nes6502* nes);
u8 IND(nes6502* nes);	
u8 IZX(nes6502* nes);
u8 IZY(nes6502* nes);

// -- ADDRESSING MODES --

// -- OPCODES --

u8 ADC(nes6502* nes);
u8 BCS(nes6502* nes);
u8 BNE(nes6502* nes);
u8 BVS(nes6502* nes);
u8 CLV(nes6502* nes);
u8 DEC(nes6502* nes);
u8 INC(nes6502* nes);
u8 JSR(nes6502* nes);
u8 LSR(nes6502* nes);
u8 PHP(nes6502* nes);
u8 ROR(nes6502* nes);
u8 SEC(nes6502* nes);
u8 STX(nes6502* nes);
u8 TSX(nes6502* nes);
u8 AND(nes6502* nes);
u8 BEQ(nes6502* nes);
u8 BPL(nes6502* nes);
u8 CLC(nes6502* nes);
u8 CMP(nes6502* nes);
u8 DEX(nes6502* nes);
u8 INX(nes6502* nes);
u8 LDA(nes6502* nes);
u8 NOP(nes6502* nes);
u8 PLA(nes6502* nes);
u8 RTI(nes6502* nes);
u8 SED(nes6502* nes);
u8 STY(nes6502* nes);
u8 TXA(nes6502* nes);
u8 ASL(nes6502* nes);
u8 BIT(nes6502* nes);
u8 BRK(nes6502* nes);
u8 CLD(nes6502* nes);
u8 CPX(nes6502* nes);
u8 DEY(nes6502* nes);
u8 INY(nes6502* nes);
u8 LDX(nes6502* nes);
u8 ORA(nes6502* nes);
u8 PLP(nes6502* nes);
u8 RTS(nes6502* nes);
u8 SEI(nes6502* nes);
u8 TAX(nes6502* nes);
u8 TXS(nes6502* nes);
u8 BCC(nes6502* nes);
u8 BMI(nes6502* nes);
u8 BVC(nes6502* nes);
u8 CLI(nes6502* nes);
u8 CPY(nes6502* nes);
u8 EOR(nes6502* nes);
u8 JMP(nes6502* nes);
u8 LDY(nes6502* nes);
u8 PHA(nes6502* nes);
u8 ROL(nes6502* nes);
u8 SBC(nes6502* nes);
u8 STA(nes6502* nes);
u8 TAY(nes6502* nes);
u8 TYA(nes6502* nes);

// All "unofficial" opcodes, basically just a NOP
u8 XXX(nes6502* nes);

// -- OPCODES --