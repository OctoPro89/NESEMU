#include "nes6502.h"
#include <emu/bus.h>
#include <stdlib.h>
#include <stdio.h>

nes6502 nes6502_init() {
    nes6502 nes = { 0 };
    nes.lookup = (instruction*)malloc(sizeof(instruction) * 256);
    
    u8 i = 0;
	nes.lookup[i] = (instruction){ "BRK", BRK, IMM, 7 }; ++i;
    nes.lookup[i] = (instruction){ "ORA", ORA, IZX, 6 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 8 }; ++i;
    nes.lookup[i] = (instruction){ "???", NOP, IMP, 3 }; ++i;
    nes.lookup[i] = (instruction){ "ORA", ORA, ZP0, 3 }; ++i;
    nes.lookup[i] = (instruction){ "ASL", ASL, ZP0, 5 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 5 }; ++i;
    nes.lookup[i] = (instruction){ "PHP", PHP, IMP, 3 }; ++i;
    nes.lookup[i] = (instruction){ "ORA", ORA, IMM, 2 }; ++i;
    nes.lookup[i] = (instruction){ "ASL", ASL, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", NOP, IMP, 4 }; ++i;
    nes.lookup[i] = (instruction){ "ORA", ORA, ABS, 4 }; ++i;
    nes.lookup[i] = (instruction){ "ASL", ASL, ABS, 6 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 6 }; ++i;
	nes.lookup[i] = (instruction){ "BPL", BPL, REL, 2 }; ++i;
    nes.lookup[i] = (instruction){ "ORA", ORA, IZY, 5 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 8 }; ++i;
    nes.lookup[i] = (instruction){ "???", NOP, IMP, 4 }; ++i;
    nes.lookup[i] = (instruction){ "ORA", ORA, ZPX, 4 }; ++i;
    nes.lookup[i] = (instruction){ "ASL", ASL, ZPX, 6 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 6 }; ++i;
    nes.lookup[i] = (instruction){ "CLC", CLC, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "ORA", ORA, ABY, 4 }; ++i;
    nes.lookup[i] = (instruction){ "???", NOP, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 7 }; ++i;
    nes.lookup[i] = (instruction){ "???", NOP, IMP, 4 }; ++i;
    nes.lookup[i] = (instruction){ "ORA", ORA, ABX, 4 }; ++i;
    nes.lookup[i] = (instruction){ "ASL", ASL, ABX, 7 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 7 }; ++i;
	nes.lookup[i] = (instruction){ "JSR", JSR, ABS, 6 }; ++i;
    nes.lookup[i] = (instruction){ "AND", AND, IZX, 6 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 8 }; ++i;
    nes.lookup[i] = (instruction){ "BIT", BIT, ZP0, 3 }; ++i;
    nes.lookup[i] = (instruction){ "AND", AND, ZP0, 3 }; ++i;
    nes.lookup[i] = (instruction){ "ROL", ROL, ZP0, 5 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 5 }; ++i;
    nes.lookup[i] = (instruction){ "PLP", PLP, IMP, 4 }; ++i;
    nes.lookup[i] = (instruction){ "AND", AND, IMM, 2 }; ++i;
    nes.lookup[i] = (instruction){ "ROL", ROL, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "BIT", BIT, ABS, 4 }; ++i;
    nes.lookup[i] = (instruction){ "AND", AND, ABS, 4 }; ++i;
    nes.lookup[i] = (instruction){ "ROL", ROL, ABS, 6 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 6 }; ++i;
	nes.lookup[i] = (instruction){ "BMI", BMI, REL, 2 }; ++i;
    nes.lookup[i] = (instruction){ "AND", AND, IZY, 5 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 8 }; ++i;
    nes.lookup[i] = (instruction){ "???", NOP, IMP, 4 }; ++i;
    nes.lookup[i] = (instruction){ "AND", AND, ZPX, 4 }; ++i;
    nes.lookup[i] = (instruction){ "ROL", ROL, ZPX, 6 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 6 }; ++i;
    nes.lookup[i] = (instruction){ "SEC", SEC, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "AND", AND, ABY, 4 }; ++i;
    nes.lookup[i] = (instruction){ "???", NOP, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 7 }; ++i;
    nes.lookup[i] = (instruction){ "???", NOP, IMP, 4 }; ++i;
    nes.lookup[i] = (instruction){ "AND", AND, ABX, 4 }; ++i;
    nes.lookup[i] = (instruction){ "ROL", ROL, ABX, 7 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 7 }; ++i;
	nes.lookup[i] = (instruction){ "RTI", RTI, IMP, 6 }; ++i;
    nes.lookup[i] = (instruction){ "EOR", EOR, IZX, 6 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 8 }; ++i;
    nes.lookup[i] = (instruction){ "???", NOP, IMP, 3 }; ++i;
    nes.lookup[i] = (instruction){ "EOR", EOR, ZP0, 3 }; ++i;
    nes.lookup[i] = (instruction){ "LSR", LSR, ZP0, 5 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 5 }; ++i;
    nes.lookup[i] = (instruction){ "PHA", PHA, IMP, 3 }; ++i;
    nes.lookup[i] = (instruction){ "EOR", EOR, IMM, 2 }; ++i;
    nes.lookup[i] = (instruction){ "LSR", LSR, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "JMP", JMP, ABS, 3 }; ++i;
    nes.lookup[i] = (instruction){ "EOR", EOR, ABS, 4 }; ++i;
    nes.lookup[i] = (instruction){ "LSR", LSR, ABS, 6 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 6 }; ++i;
	nes.lookup[i] = (instruction){ "BVC", BVC, REL, 2 }; ++i;
    nes.lookup[i] = (instruction){ "EOR", EOR, IZY, 5 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 8 }; ++i;
    nes.lookup[i] = (instruction){ "???", NOP, IMP, 4 }; ++i;
    nes.lookup[i] = (instruction){ "EOR", EOR, ZPX, 4 }; ++i;
    nes.lookup[i] = (instruction){ "LSR", LSR, ZPX, 6 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 6 }; ++i;
    nes.lookup[i] = (instruction){ "CLI", CLI, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "EOR", EOR, ABY, 4 }; ++i;
    nes.lookup[i] = (instruction){ "???", NOP, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 7 }; ++i;
    nes.lookup[i] = (instruction){ "???", NOP, IMP, 4 }; ++i;
    nes.lookup[i] = (instruction){ "EOR", EOR, ABX, 4 }; ++i;
    nes.lookup[i] = (instruction){ "LSR", LSR, ABX, 7 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 7 }; ++i;
	nes.lookup[i] = (instruction){ "RTS", RTS, IMP, 6 }; ++i;
    nes.lookup[i] = (instruction){ "ADC", ADC, IZX, 6 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 8 }; ++i;
    nes.lookup[i] = (instruction){ "???", NOP, IMP, 3 }; ++i;
    nes.lookup[i] = (instruction){ "ADC", ADC, ZP0, 3 }; ++i;
    nes.lookup[i] = (instruction){ "ROR", ROR, ZP0, 5 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 5 }; ++i;
    nes.lookup[i] = (instruction){ "PLA", PLA, IMP, 4 }; ++i;
    nes.lookup[i] = (instruction){ "ADC", ADC, IMM, 2 }; ++i;
    nes.lookup[i] = (instruction){ "ROR", ROR, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "JMP", JMP, IND, 5 }; ++i;
    nes.lookup[i] = (instruction){ "ADC", ADC, ABS, 4 }; ++i;
    nes.lookup[i] = (instruction){ "ROR", ROR, ABS, 6 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 6 }; ++i;
	nes.lookup[i] = (instruction){ "BVS", BVS, REL, 2 }; ++i;
    nes.lookup[i] = (instruction){ "ADC", ADC, IZY, 5 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 8 }; ++i;
    nes.lookup[i] = (instruction){ "???", NOP, IMP, 4 }; ++i;
    nes.lookup[i] = (instruction){ "ADC", ADC, ZPX, 4 }; ++i;
    nes.lookup[i] = (instruction){ "ROR", ROR, ZPX, 6 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 6 }; ++i;
    nes.lookup[i] = (instruction){ "SEI", SEI, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "ADC", ADC, ABY, 4 }; ++i;
    nes.lookup[i] = (instruction){ "???", NOP, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 7 }; ++i;
    nes.lookup[i] = (instruction){ "???", NOP, IMP, 4 }; ++i;
    nes.lookup[i] = (instruction){ "ADC", ADC, ABX, 4 }; ++i;
    nes.lookup[i] = (instruction){ "ROR", ROR, ABX, 7 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 7 }; ++i;
	nes.lookup[i] = (instruction){ "???", NOP, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "STA", STA, IZX, 6 }; ++i;
    nes.lookup[i] = (instruction){ "???", NOP, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 6 }; ++i;
    nes.lookup[i] = (instruction){ "STY", STY, ZP0, 3 }; ++i;
    nes.lookup[i] = (instruction){ "STA", STA, ZP0, 3 }; ++i;
    nes.lookup[i] = (instruction){ "STX", STX, ZP0, 3 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 3 }; ++i;
    nes.lookup[i] = (instruction){ "DEY", DEY, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", NOP, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "TXA", TXA, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "STY", STY, ABS, 4 }; ++i;
    nes.lookup[i] = (instruction){ "STA", STA, ABS, 4 }; ++i;
    nes.lookup[i] = (instruction){ "STX", STX, ABS, 4 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 4 }; ++i;
	nes.lookup[i] = (instruction){ "BCC", BCC, REL, 2 }; ++i;
    nes.lookup[i] = (instruction){ "STA", STA, IZY, 6 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 6 }; ++i;
    nes.lookup[i] = (instruction){ "STY", STY, ZPX, 4 }; ++i;
    nes.lookup[i] = (instruction){ "STA", STA, ZPX, 4 }; ++i;
    nes.lookup[i] = (instruction){ "STX", STX, ZPY, 4 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 4 }; ++i;
    nes.lookup[i] = (instruction){ "TYA", TYA, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "STA", STA, ABY, 5 }; ++i;
    nes.lookup[i] = (instruction){ "TXS", TXS, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 5 }; ++i;
    nes.lookup[i] = (instruction){ "???", NOP, IMP, 5 }; ++i;
    nes.lookup[i] = (instruction){ "STA", STA, ABX, 5 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 5 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 5 }; ++i;
	nes.lookup[i] = (instruction){ "LDY", LDY, IMM, 2 }; ++i;
    nes.lookup[i] = (instruction){ "LDA", LDA, IZX, 6 }; ++i;
    nes.lookup[i] = (instruction){ "LDX", LDX, IMM, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 6 }; ++i;
    nes.lookup[i] = (instruction){ "LDY", LDY, ZP0, 3 }; ++i;
    nes.lookup[i] = (instruction){ "LDA", LDA, ZP0, 3 }; ++i;
    nes.lookup[i] = (instruction){ "LDX", LDX, ZP0, 3 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 3 }; ++i;
    nes.lookup[i] = (instruction){ "TAY", TAY, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "LDA", LDA, IMM, 2 }; ++i;
    nes.lookup[i] = (instruction){ "TAX", TAX, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "LDY", LDY, ABS, 4 }; ++i;
    nes.lookup[i] = (instruction){ "LDA", LDA, ABS, 4 }; ++i;
    nes.lookup[i] = (instruction){ "LDX", LDX, ABS, 4 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 4 }; ++i;
	nes.lookup[i] = (instruction){ "BCS", BCS, REL, 2 }; ++i;
    nes.lookup[i] = (instruction){ "LDA", LDA, IZY, 5 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 5 }; ++i;
    nes.lookup[i] = (instruction){ "LDY", LDY, ZPX, 4 }; ++i;
    nes.lookup[i] = (instruction){ "LDA", LDA, ZPX, 4 }; ++i;
    nes.lookup[i] = (instruction){ "LDX", LDX, ZPY, 4 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 4 }; ++i;
    nes.lookup[i] = (instruction){ "CLV", CLV, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "LDA", LDA, ABY, 4 }; ++i;
    nes.lookup[i] = (instruction){ "TSX", TSX, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 4 }; ++i;
    nes.lookup[i] = (instruction){ "LDY", LDY, ABX, 4 }; ++i;
    nes.lookup[i] = (instruction){ "LDA", LDA, ABX, 4 }; ++i;
    nes.lookup[i] = (instruction){ "LDX", LDX, ABY, 4 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 4 }; ++i;
	nes.lookup[i] = (instruction){ "CPY", CPY, IMM, 2 }; ++i;
    nes.lookup[i] = (instruction){ "CMP", CMP, IZX, 6 }; ++i;
    nes.lookup[i] = (instruction){ "???", NOP, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 8 }; ++i;
    nes.lookup[i] = (instruction){ "CPY", CPY, ZP0, 3 }; ++i;
    nes.lookup[i] = (instruction){ "CMP", CMP, ZP0, 3 }; ++i;
    nes.lookup[i] = (instruction){ "DEC", DEC, ZP0, 5 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 5 }; ++i;
    nes.lookup[i] = (instruction){ "INY", INY, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "CMP", CMP, IMM, 2 }; ++i;
    nes.lookup[i] = (instruction){ "DEX", DEX, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "CPY", CPY, ABS, 4 }; ++i;
    nes.lookup[i] = (instruction){ "CMP", CMP, ABS, 4 }; ++i;
    nes.lookup[i] = (instruction){ "DEC", DEC, ABS, 6 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 6 }; ++i;
	nes.lookup[i] = (instruction){ "BNE", BNE, REL, 2 }; ++i;
    nes.lookup[i] = (instruction){ "CMP", CMP, IZY, 5 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 8 }; ++i;
    nes.lookup[i] = (instruction){ "???", NOP, IMP, 4 }; ++i;
    nes.lookup[i] = (instruction){ "CMP", CMP, ZPX, 4 }; ++i;
    nes.lookup[i] = (instruction){ "DEC", DEC, ZPX, 6 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 6 }; ++i;
    nes.lookup[i] = (instruction){ "CLD", CLD, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "CMP", CMP, ABY, 4 }; ++i;
    nes.lookup[i] = (instruction){ "NOP", NOP, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 7 }; ++i;
    nes.lookup[i] = (instruction){ "???", NOP, IMP, 4 }; ++i;
    nes.lookup[i] = (instruction){ "CMP", CMP, ABX, 4 }; ++i;
    nes.lookup[i] = (instruction){ "DEC", DEC, ABX, 7 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 7 }; ++i;
	nes.lookup[i] = (instruction){ "CPX", CPX, IMM, 2 }; ++i;
    nes.lookup[i] = (instruction){ "SBC", SBC, IZX, 6 }; ++i;
    nes.lookup[i] = (instruction){ "???", NOP, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 8 }; ++i;
    nes.lookup[i] = (instruction){ "CPX", CPX, ZP0, 3 }; ++i;
    nes.lookup[i] = (instruction){ "SBC", SBC, ZP0, 3 }; ++i;
    nes.lookup[i] = (instruction){ "INC", INC, ZP0, 5 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 5 }; ++i;
    nes.lookup[i] = (instruction){ "INX", INX, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "SBC", SBC, IMM, 2 }; ++i;
    nes.lookup[i] = (instruction){ "NOP", NOP, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", SBC, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "CPX", CPX, ABS, 4 }; ++i;
    nes.lookup[i] = (instruction){ "SBC", SBC, ABS, 4 }; ++i;
    nes.lookup[i] = (instruction){ "INC", INC, ABS, 6 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 6 }; ++i;
	nes.lookup[i] = (instruction){ "BEQ", BEQ, REL, 2 }; ++i;
    nes.lookup[i] = (instruction){ "SBC", SBC, IZY, 5 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 8 }; ++i;
    nes.lookup[i] = (instruction){ "???", NOP, IMP, 4 }; ++i;
    nes.lookup[i] = (instruction){ "SBC", SBC, ZPX, 4 }; ++i;
    nes.lookup[i] = (instruction){ "INC", INC, ZPX, 6 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 6 }; ++i;
    nes.lookup[i] = (instruction){ "SED", SED, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "SBC", SBC, ABY, 4 }; ++i;
    nes.lookup[i] = (instruction){ "NOP", NOP, IMP, 2 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 7 }; ++i;
    nes.lookup[i] = (instruction){ "???", NOP, IMP, 4 }; ++i;
    nes.lookup[i] = (instruction){ "SBC", SBC, ABX, 4 }; ++i;
    nes.lookup[i] = (instruction){ "INC", INC, ABX, 7 }; ++i;
    nes.lookup[i] = (instruction){ "???", XXX, IMP, 7 };

    return nes;
}

void nes6502_destroy(nes6502* nes) {
    free(nes->lookup);
}

void nes6502_reset(nes6502* nes) {
    nes->addr_abs = 0xFFFC;
	u16 lo = nes6502_read(nes, nes->addr_abs);
	u16 hi = nes6502_read(nes, nes->addr_abs + 1);

	nes->pc = (hi << 8) | lo;

	nes->a = 0;
	nes->x = 0;
	nes->y = 0;
	nes->stkp = 0xFD;
	nes->status = 0x00 | FLAGS_6502_U;

	nes->addr_rel = 0x0000;
	nes->addr_abs = 0x0000;
	nes->fetched = 0x00;

	nes->cycles = 8;
}

void nes6502_irq(nes6502* nes) {
    if (nes6502_get_flag(nes, FLAGS_6502_I) == 0)
	{
		nes6502_write(nes, 0x0100 + nes->stkp, (nes->pc >> 8) & 0x00FF);
		nes->stkp--;
		nes6502_write(nes, 0x0100 + nes->stkp, nes->pc & 0x00FF);
		nes->stkp--;

		nes6502_set_flag(nes, FLAGS_6502_B, 0);
		nes6502_set_flag(nes, FLAGS_6502_U, 1);
		nes6502_set_flag(nes, FLAGS_6502_I, 1);
		nes6502_write(nes, 0x0100 + nes->stkp, nes->status);
		nes->stkp--;

		nes->addr_abs = 0xFFFE;
		u16 lo = nes6502_read(nes, nes->addr_abs);
		u16 hi = nes6502_read(nes, nes->addr_abs + 1);
		nes->pc = (hi << 8) | lo;

		nes->cycles = 7;
	}
}

void nes6502_nmi(nes6502* nes) {
	nes6502_write(nes, 0x0100 + nes->stkp, (nes->pc >> 8) & 0x00FF);
	nes->stkp--;
	nes6502_write(nes, 0x0100 + nes->stkp, nes->pc & 0x00FF);
	nes->stkp--;

	nes6502_set_flag(nes, FLAGS_6502_B, 0);
	nes6502_set_flag(nes, FLAGS_6502_U, 1);
	nes6502_set_flag(nes, FLAGS_6502_I, 1);
	nes6502_write(nes, 0x0100 + nes->stkp, nes->status);
	nes->stkp--;

	nes->addr_abs = 0xFFFA;
	u16 lo = nes6502_read(nes, nes->addr_abs);
	u16 hi = nes6502_read(nes, nes->addr_abs + 1);
	nes->pc = (hi << 8) | lo;

	nes->cycles = 8;
}

void nes6502_clock(nes6502* nes) {
    if (nes->cycles == 0) {
        nes->opcode = nes6502_read(nes, nes->pc);

        nes6502_set_flag(nes, FLAGS_6502_U, true);

        ++nes->pc;

        nes->cycles = nes->lookup[nes->opcode].cycles;

        u8 additional_cycle1 = nes->lookup[nes->opcode].addrmode(nes);
        u8 additional_cycle2 = nes->lookup[nes->opcode].operate(nes);

        nes->cycles += (additional_cycle1 & additional_cycle2);

        nes6502_set_flag(nes, FLAGS_6502_U, true);
    }

    ++nes->clock_count;

    --nes->cycles;
}

u8 nes6502_complete(nes6502* nes) {
    return nes->cycles == 0;
}

void nes6502_connect_bus(nes6502* nes, struct bus* b) {
    nes->b = b;
}

void nes6502_disassemble(nes6502* nes, u16 start, u16 stop) {
    u16 addr = start;

    while (addr <= stop) {
        u16 line_addr = addr;

        printf("$%04X: ", line_addr);

        u8 opcode = nes6502_read(nes, addr++);
        const instruction* inst = &nes->lookup[opcode];

        printf("%s ", inst->name);

        u8 lo = 0, hi = 0, value = 0;
        u16 abs_addr = 0;
        if (inst->addrmode == IMP) {
            printf("{IMP}");
        }

        if (inst->addrmode == IMM) {
            value = nes6502_read(nes, addr++);
            printf("#$%02X {IMM}", value);
        }

        if (inst->addrmode == ZP0) {
            lo = nes6502_read(nes, addr++);
            printf("$%02X {ZP0}", lo);
        }

        if (inst->addrmode == ZPX) {
            lo = nes6502_read(nes, addr++);
            printf("$%02X, X {ZPX}", lo);
        }

        if (inst->addrmode == ZPY) {
            lo = nes6502_read(nes, addr++);
            printf("$%02X, Y {ZPY}", lo);
        }

        if (inst->addrmode == IZX) {
            lo = nes6502_read(nes, addr++);
            printf("($%02X, X) {IZX}", lo);
        }

        if (inst->addrmode == IZY) {
            lo = nes6502_read(nes, addr++);
            printf("($%02X), Y {IZY}", lo);
        }

        if (inst->addrmode == ABS) {
            lo = nes6502_read(nes, addr++);
            hi = nes6502_read(nes, addr++);
            abs_addr = (hi << 8) | lo;
            printf("$%04X {ABS}", abs_addr);
        }

        if (inst->addrmode == ABX) {
            lo = nes6502_read(nes, addr++);
            hi = nes6502_read(nes, addr++);
            abs_addr = (hi << 8) | lo;
            printf("$%04X, X {ABX}", abs_addr);
        }

        if (inst->addrmode == ABY) {
            lo = nes6502_read(nes, addr++);
            hi = nes6502_read(nes, addr++);
            abs_addr = (hi << 8) | lo;
            printf("$%04X, Y {ABY}", abs_addr);
        }

        if (inst->addrmode == IND) {
            lo = nes6502_read(nes, addr++);
            hi = nes6502_read(nes, addr++);
            abs_addr = (hi << 8) | lo;
            printf("($%04X) {IND}", abs_addr);
        }

        if (inst->addrmode == REL) {
            value = nes6502_read(nes, addr++);
            abs_addr = addr + (int8_t)value;
            printf("$%02X [$%04X] {REL}", value, abs_addr);
        }
    }
}

u8 nes6502_get_flag(nes6502* nes, flags_6502 f) {
    return ((nes->status & f) > 0) ? 1 : 0;
}

void nes6502_set_flag(nes6502* nes, flags_6502 f, u8 v) {
    if (v) {
        nes->status |= f;
    } else {
        nes->status &= ~f;
    }
}

u8 nes6502_read(nes6502* nes, u16 a) {
    return bus_cpu_read(nes->b, a, false);
}

void nes6502_write(nes6502* nes, u16 a, u8 d) {
    bus_cpu_write(nes->b, a, d);
}

u8 nes6502_fetch(nes6502* nes) {
    if (nes->lookup[nes->opcode].addrmode != IMP) {
        nes->fetched = nes6502_read(nes, nes->addr_abs);
    }

    return nes->fetched;
}

// -- ADDRESSING MODES --

u8 IMP(nes6502* nes) {
    nes->fetched = nes->a;
    return 0;
}

u8 IMM(nes6502* nes) {
    nes->addr_abs = nes->pc++;
    return 0;
}

u8 ZP0(nes6502* nes) {
    nes->addr_abs = nes6502_read(nes, nes->pc++);
    nes->addr_abs &= 0x00FF;
    return 0;
}

u8 ZPX(nes6502* nes) {
    nes->addr_abs = (nes6502_read(nes, nes->pc++) + nes->x) & 0x00FF;
    return 0;
}

u8 ZPY(nes6502* nes) {
    nes->addr_abs = (nes6502_read(nes, nes->pc++) + nes->y) & 0x00FF;
    return 0;
}

u8 REL(nes6502* nes) {
    nes->addr_rel = nes6502_read(nes, nes->pc++);
    if (nes->addr_rel & 0x80)
        nes->addr_rel |= 0xFF00;
    return 0;
}

u8 ABS(nes6502* nes) {
    u16 lo = nes6502_read(nes, nes->pc++);
    u16 hi = nes6502_read(nes, nes->pc++);
    nes->addr_abs = (hi << 8) | lo;
    return 0;
}

u8 ABX(nes6502* nes) {
    u16 lo = nes6502_read(nes, nes->pc++);
    u16 hi = nes6502_read(nes, nes->pc++);
    nes->addr_abs = ((hi << 8) | lo) + nes->x;
    return ((nes->addr_abs & 0xFF00) != (hi << 8));
}

u8 ABY(nes6502* nes) {
    u16 lo = nes6502_read(nes, nes->pc++);
    u16 hi = nes6502_read(nes, nes->pc++);
    nes->addr_abs = ((hi << 8) | lo) + nes->y;
    return ((nes->addr_abs & 0xFF00) != (hi << 8));
}

u8 IND(nes6502* nes) {
    u16 ptr_lo = nes6502_read(nes, nes->pc++);
    u16 ptr_hi = nes6502_read(nes, nes->pc++);
    u16 ptr = (ptr_hi << 8) | ptr_lo;

    if (ptr_lo == 0x00FF) {
        nes->addr_abs = (nes6502_read(nes, ptr & 0xFF00) << 8) | nes6502_read(nes, ptr);
    } else {
        nes->addr_abs = (nes6502_read(nes, ptr + 1) << 8) | nes6502_read(nes, ptr);
    }
    return 0;
}

u8 IZX(nes6502* nes) {
    u16 t = nes6502_read(nes, nes->pc++);
    u16 lo = nes6502_read(nes, (u8)(t + nes->x) & 0x00FF);
    u16 hi = nes6502_read(nes, (u8)(t + nes->x + 1) & 0x00FF);
    nes->addr_abs = (hi << 8) | lo;
    return 0;
}

u8 IZY(nes6502* nes) {
    u16 t = nes6502_read(nes, nes->pc++);
    u16 lo = nes6502_read(nes, t & 0x00FF);
    u16 hi = nes6502_read(nes, (t + 1) & 0x00FF);
    nes->addr_abs = ((hi << 8) | lo) + nes->y;
    return ((nes->addr_abs & 0xFF00) != (hi << 8));
}

// -- ADDRESSING MODES --

// -- OPCODES --

u8 ADC(nes6502* nes) {
    nes6502_fetch(nes);
    nes->temp = (u16)nes->a + (u16)nes->fetched + (u16)nes6502_get_flag(nes, FLAGS_6502_C);
    nes6502_set_flag(nes, FLAGS_6502_C, nes->temp > 255);
    nes6502_set_flag(nes, FLAGS_6502_Z, (nes->temp & 0x00FF) == 0);
    nes6502_set_flag(nes, FLAGS_6502_V, (~((u16)nes->a ^ (u16)nes->fetched) & ((u16)nes->a ^ (u16)nes->temp)) & 0x0080);
    nes6502_set_flag(nes, FLAGS_6502_N, nes->temp & 0x80);
    nes->a = nes->temp & 0x00FF;
    return 1;
}

u8 SBC(nes6502* nes) {
    nes6502_fetch(nes);
    u16 value = ((u16)nes->fetched) ^ 0x00FF;
    nes->temp = (u16)nes->a + value + (u16)nes6502_get_flag(nes, FLAGS_6502_C);
    nes6502_set_flag(nes, FLAGS_6502_C, nes->temp & 0xFF00);
    nes6502_set_flag(nes, FLAGS_6502_Z, ((nes->temp & 0x00FF) == 0));
    nes6502_set_flag(nes, FLAGS_6502_V, (nes->temp ^ (u16)nes->a) & (nes->temp ^ value) & 0x0080);
    nes6502_set_flag(nes, FLAGS_6502_N, nes->temp & 0x0080);
    nes->a = nes->temp & 0x00FF;
    return 1;
}

u8 AND(nes6502* nes) {
    nes6502_fetch(nes);
    nes->a = nes->a & nes->fetched;
    nes6502_set_flag(nes, FLAGS_6502_Z, nes->a == 0x00);
    nes6502_set_flag(nes, FLAGS_6502_N, nes->a & 0x80);
    return 1;
}

u8 ASL(nes6502* nes) {
    nes6502_fetch(nes);
    nes->temp = (u16)nes->fetched << 1;
    nes6502_set_flag(nes, FLAGS_6502_C, (nes->temp & 0xFF00) > 0);
    nes6502_set_flag(nes, FLAGS_6502_Z, (nes->temp & 0x00FF) == 0x00);
    nes6502_set_flag(nes, FLAGS_6502_N, nes->temp & 0x80);
    if (nes->lookup[nes->opcode].addrmode == IMP)
        nes->a = nes->temp & 0x00FF;
    else
        nes6502_write(nes, nes->addr_abs, nes->temp & 0x00FF);
    return 0;
}

u8 BCC(nes6502* nes) {
    if (nes6502_get_flag(nes, FLAGS_6502_C) == 0) {
        nes->cycles++;
        nes->addr_abs = nes->pc + nes->addr_rel;
        if ((nes->addr_abs & 0xFF00) != (nes->pc & 0xFF00))
            nes->cycles++;
        nes->pc = nes->addr_abs;
    }
    return 0;
}

u8 BCS(nes6502* nes) {
    if (nes6502_get_flag(nes, FLAGS_6502_C) == 1) {
        nes->cycles++;
        nes->addr_abs = nes->pc + nes->addr_rel;
        if ((nes->addr_abs & 0xFF00) != (nes->pc & 0xFF00))
            nes->cycles++;
        nes->pc = nes->addr_abs;
    }
    return 0;
}

u8 BEQ(nes6502* nes) {
    if (nes6502_get_flag(nes, FLAGS_6502_Z) == 1) {
        nes->cycles++;
        nes->addr_abs = nes->pc + nes->addr_rel;
        if ((nes->addr_abs & 0xFF00) != (nes->pc & 0xFF00))
            nes->cycles++;
        nes->pc = nes->addr_abs;
    }
    return 0;
}

u8 BIT(nes6502* nes) {
    nes6502_fetch(nes);
    nes->temp = nes->a & nes->fetched;
    nes6502_set_flag(nes, FLAGS_6502_Z, (nes->temp & 0x00FF) == 0x00);
    nes6502_set_flag(nes, FLAGS_6502_N, nes->fetched & (1 << 7));
    nes6502_set_flag(nes, FLAGS_6502_V, nes->fetched & (1 << 6));
    return 0;
}

u8 BMI(nes6502* nes) {
    if (nes6502_get_flag(nes, FLAGS_6502_N) == 1) {
        nes->cycles++;
        nes->addr_abs = nes->pc + nes->addr_rel;
        if ((nes->addr_abs & 0xFF00) != (nes->pc & 0xFF00))
            nes->cycles++;
        nes->pc = nes->addr_abs;
    }
    return 0;
}

u8 BNE(nes6502* nes) {
    if (nes6502_get_flag(nes, FLAGS_6502_Z) == 0) {
        nes->cycles++;
        nes->addr_abs = nes->pc + nes->addr_rel;
        if ((nes->addr_abs & 0xFF00) != (nes->pc & 0xFF00))
            nes->cycles++;
        nes->pc = nes->addr_abs;
    }
    return 0;
}

u8 BPL(nes6502* nes) {
    if (nes6502_get_flag(nes, FLAGS_6502_N) == 0) {
        nes->cycles++;
        nes->addr_abs = nes->pc + nes->addr_rel;
        if ((nes->addr_abs & 0xFF00) != (nes->pc & 0xFF00))
            nes->cycles++;
        nes->pc = nes->addr_abs;
    }
    return 0;
}

u8 BRK(nes6502* nes) {
    nes->pc++;
    nes6502_set_flag(nes, FLAGS_6502_I, 1);
    nes6502_write(nes, 0x0100 + nes->stkp--, (nes->pc >> 8) & 0x00FF);
    nes6502_write(nes, 0x0100 + nes->stkp--, nes->pc & 0x00FF);
    nes6502_set_flag(nes, FLAGS_6502_B, 1);
    nes6502_write(nes, 0x0100 + nes->stkp--, nes->status);
    nes6502_set_flag(nes, FLAGS_6502_B, 0);
    u16 lo = nes6502_read(nes, 0xFFFE);
    u16 hi = nes6502_read(nes, 0xFFFF);
    nes->pc = (hi << 8) | lo;
    return 0;
}

u8 BVC(nes6502* nes) {
    if (nes6502_get_flag(nes, FLAGS_6502_V) == 0) {
        nes->cycles++;
        nes->addr_abs = nes->pc + nes->addr_rel;
        if ((nes->addr_abs & 0xFF00) != (nes->pc & 0xFF00)) nes->cycles++;
        nes->pc = nes->addr_abs;
    }
    return 0;
}

u8 BVS(nes6502* nes) {
    if (nes6502_get_flag(nes, FLAGS_6502_V) == 1) {
        nes->cycles++;
        nes->addr_abs = nes->pc + nes->addr_rel;
        if ((nes->addr_abs & 0xFF00) != (nes->pc & 0xFF00)) nes->cycles++;
        nes->pc = nes->addr_abs;
    }
    return 0;
}

u8 CLC(nes6502* nes) { nes6502_set_flag(nes, FLAGS_6502_C, false); return 0; }
u8 CLD(nes6502* nes) { nes6502_set_flag(nes, FLAGS_6502_D, false); return 0; }
u8 CLI(nes6502* nes) { nes6502_set_flag(nes, FLAGS_6502_I, false); return 0; }
u8 CLV(nes6502* nes) { nes6502_set_flag(nes, FLAGS_6502_V, false); return 0; }

u8 CMP(nes6502* nes) {
    nes6502_fetch(nes);
    nes->temp = (u16)nes->a - (u16)nes->fetched;
    nes6502_set_flag(nes, FLAGS_6502_C, nes->a >= nes->fetched);
    nes6502_set_flag(nes, FLAGS_6502_Z, (nes->temp & 0x00FF) == 0x00);
    nes6502_set_flag(nes, FLAGS_6502_N, nes->temp & 0x80);
    return 1;
}

u8 CPX(nes6502* nes) {
    nes6502_fetch(nes);
    nes->temp = (u16)nes->x - (u16)nes->fetched;
    nes6502_set_flag(nes, FLAGS_6502_C, nes->x >= nes->fetched);
    nes6502_set_flag(nes, FLAGS_6502_Z, (nes->temp & 0x00FF) == 0x00);
    nes6502_set_flag(nes, FLAGS_6502_N, nes->temp & 0x80);
    return 0;
}

u8 CPY(nes6502* nes) {
    nes6502_fetch(nes);
    nes->temp = (u16)nes->y - (u16)nes->fetched;
    nes6502_set_flag(nes, FLAGS_6502_C, nes->y >= nes->fetched);
    nes6502_set_flag(nes, FLAGS_6502_Z, (nes->temp & 0x00FF) == 0x00);
    nes6502_set_flag(nes, FLAGS_6502_N, nes->temp & 0x80);
    return 0;
}

u8 DEC(nes6502* nes) {
    nes6502_fetch(nes);
    nes->temp = nes->fetched - 1;
    nes6502_write(nes, nes->addr_abs, nes->temp & 0x00FF);
    nes6502_set_flag(nes, FLAGS_6502_Z, (nes->temp & 0x00FF) == 0x00);
    nes6502_set_flag(nes, FLAGS_6502_N, nes->temp & 0x80);
    return 0;
}

u8 DEX(nes6502* nes) {
    nes->x--;
    nes6502_set_flag(nes, FLAGS_6502_Z, nes->x == 0x00);
    nes6502_set_flag(nes, FLAGS_6502_N, nes->x & 0x80);
    return 0;
}

u8 DEY(nes6502* nes) {
    nes->y--;
    nes6502_set_flag(nes, FLAGS_6502_Z, nes->y == 0x00);
    nes6502_set_flag(nes, FLAGS_6502_N, nes->y & 0x80);
    return 0;
}

u8 EOR(nes6502* nes) {
    nes6502_fetch(nes);
    nes->a = nes->a ^ nes->fetched;
    nes6502_set_flag(nes, FLAGS_6502_Z, nes->a == 0x00);
    nes6502_set_flag(nes, FLAGS_6502_N, nes->a & 0x80);
    return 1;
}

u8 INC(nes6502* nes) {
    nes6502_fetch(nes);
    nes->temp = nes->fetched + 1;
    nes6502_write(nes, nes->addr_abs, nes->temp & 0x00FF);
    nes6502_set_flag(nes, FLAGS_6502_Z, (nes->temp & 0x00FF) == 0x00);
    nes6502_set_flag(nes, FLAGS_6502_N, nes->temp & 0x80);
    return 0;
}

u8 INX(nes6502* nes) {
    nes->x++;
    nes6502_set_flag(nes, FLAGS_6502_Z, nes->x == 0x00);
    nes6502_set_flag(nes, FLAGS_6502_N, nes->x & 0x80);
    return 0;
}

u8 INY(nes6502* nes) {
    nes->y++;
    nes6502_set_flag(nes, FLAGS_6502_Z, nes->y == 0x00);
    nes6502_set_flag(nes, FLAGS_6502_N, nes->y & 0x80);
    return 0;
}

u8 JMP(nes6502* nes) {
    nes->pc = nes->addr_abs;
    return 0;
}

u8 JSR(nes6502* nes) {
    nes->pc--;
    nes6502_write(nes, 0x0100 + nes->stkp--, (nes->pc >> 8) & 0x00FF);
    nes6502_write(nes, 0x0100 + nes->stkp--, nes->pc & 0x00FF);
    nes->pc = nes->addr_abs;
    return 0;
}

u8 LDA(nes6502* nes) {
    nes6502_fetch(nes);
    nes->a = nes->fetched;
    nes6502_set_flag(nes, FLAGS_6502_Z, nes->a == 0x00);
    nes6502_set_flag(nes, FLAGS_6502_N, nes->a & 0x80);
    return 1;
}

u8 LDX(nes6502* nes) {
    nes6502_fetch(nes);
    nes->x = nes->fetched;
    nes6502_set_flag(nes, FLAGS_6502_Z, nes->x == 0x00);
    nes6502_set_flag(nes, FLAGS_6502_N, nes->x & 0x80);
    return 1;
}

u8 LDY(nes6502* nes) {
    nes6502_fetch(nes);
    nes->y = nes->fetched;
    nes6502_set_flag(nes, FLAGS_6502_Z, nes->y == 0x00);
    nes6502_set_flag(nes, FLAGS_6502_N, nes->y & 0x80);
    return 1;
}

u8 LSR(nes6502* nes) {
    nes6502_fetch(nes);
    nes6502_set_flag(nes, FLAGS_6502_C, nes->fetched & 0x01);
    nes->temp = nes->fetched >> 1;
    nes6502_set_flag(nes, FLAGS_6502_Z, (nes->temp & 0x00FF) == 0x00);
    nes6502_set_flag(nes, FLAGS_6502_N, nes->temp & 0x80);
    if (nes->lookup[nes->opcode].addrmode == IMP)
        nes->a = nes->temp & 0x00FF;
    else
        nes6502_write(nes, nes->addr_abs, nes->temp & 0x00FF);
    return 0;
}

u8 NOP(nes6502* nes) {
    switch (nes->opcode) {
        case 0x1C: case 0x3C: case 0x5C:
        case 0x7C: case 0xDC: case 0xFC:
            return 1;
    }
    return 0;
}

u8 ORA(nes6502* nes) {
    nes6502_fetch(nes);
    nes->a = nes->a | nes->fetched;
    nes6502_set_flag(nes, FLAGS_6502_Z, nes->a == 0x00);
    nes6502_set_flag(nes, FLAGS_6502_N, nes->a & 0x80);
    return 1;
}

u8 PHA(nes6502* nes) {
    nes6502_write(nes, 0x0100 + nes->stkp--, nes->a);
    return 0;
}

u8 PHP(nes6502* nes) {
    nes6502_write(nes, 0x0100 + nes->stkp--, nes->status | FLAGS_6502_B | FLAGS_6502_U);
    nes6502_set_flag(nes, FLAGS_6502_B, 0);
    nes6502_set_flag(nes, FLAGS_6502_U, 0);
    return 0;
}

u8 PLA(nes6502* nes) {
    nes->stkp++;
    nes->a = nes6502_read(nes, 0x0100 + nes->stkp);
    nes6502_set_flag(nes, FLAGS_6502_Z, nes->a == 0x00);
    nes6502_set_flag(nes, FLAGS_6502_N, nes->a & 0x80);
    return 0;
}

u8 PLP(nes6502* nes) {
    nes->stkp++;
    nes->status = nes6502_read(nes, 0x0100 + nes->stkp);
    nes6502_set_flag(nes, FLAGS_6502_U, 1);
    return 0;
}

u8 ROL(nes6502* nes) {
    nes6502_fetch(nes);
    nes->temp = ((u16)nes->fetched << 1) | nes6502_get_flag(nes, FLAGS_6502_C);
    nes6502_set_flag(nes, FLAGS_6502_C, nes->temp & 0xFF00);
    nes6502_set_flag(nes, FLAGS_6502_Z, (nes->temp & 0x00FF) == 0);
    nes6502_set_flag(nes, FLAGS_6502_N, nes->temp & 0x80);
    if (nes->lookup[nes->opcode].addrmode == IMP)
        nes->a = nes->temp & 0x00FF;
    else
        nes6502_write(nes, nes->addr_abs, nes->temp & 0x00FF);
    return 0;
}

u8 ROR(nes6502* nes) {
    nes6502_fetch(nes);
    nes->temp = (nes6502_get_flag(nes, FLAGS_6502_C) << 7) | (nes->fetched >> 1);
    nes6502_set_flag(nes, FLAGS_6502_C, nes->fetched & 0x01);
    nes6502_set_flag(nes, FLAGS_6502_Z, (nes->temp & 0x00FF) == 0);
    nes6502_set_flag(nes, FLAGS_6502_N, nes->temp & 0x80);
    if (nes->lookup[nes->opcode].addrmode == IMP)
        nes->a = nes->temp & 0x00FF;
    else
        nes6502_write(nes, nes->addr_abs, nes->temp & 0x00FF);
    return 0;
}

u8 RTI(nes6502* nes) {
    nes->stkp++;
    nes->status = nes6502_read(nes, 0x0100 + nes->stkp);
    nes->status &= ~FLAGS_6502_B;
    nes->status &= ~FLAGS_6502_U;

    nes->stkp++;
    nes->pc = (u16)nes6502_read(nes, 0x0100 + nes->stkp);
    nes->stkp++;
    nes->pc |= ((u16)nes6502_read(nes, 0x0100 + nes->stkp) << 8);
    return 0;
}

u8 RTS(nes6502* nes) {
    nes->stkp++;
    nes->pc = (u16)nes6502_read(nes, 0x0100 + nes->stkp);
    nes->stkp++;
    nes->pc |= ((u16)nes6502_read(nes, 0x0100 + nes->stkp) << 8);
    nes->pc++;
    return 0;
}

u8 SEC(nes6502* nes) { nes6502_set_flag(nes, FLAGS_6502_C, 1); return 0; }
u8 SED(nes6502* nes) { nes6502_set_flag(nes, FLAGS_6502_D, 1); return 0; }
u8 SEI(nes6502* nes) { nes6502_set_flag(nes, FLAGS_6502_I, 1); return 0; }

u8 STA(nes6502* nes) {
    nes6502_write(nes, nes->addr_abs, nes->a);
    return 0;
}

u8 STX(nes6502* nes) {
    nes6502_write(nes, nes->addr_abs, nes->x);
    return 0;
}

u8 STY(nes6502* nes) {
    nes6502_write(nes, nes->addr_abs, nes->y);
    return 0;
}

u8 TAX(nes6502* nes) {
    nes->x = nes->a;
    nes6502_set_flag(nes, FLAGS_6502_Z, nes->x == 0);
    nes6502_set_flag(nes, FLAGS_6502_N, nes->x & 0x80);
    return 0;
}

u8 TAY(nes6502* nes) {
    nes->y = nes->a;
    nes6502_set_flag(nes, FLAGS_6502_Z, nes->y == 0);
    nes6502_set_flag(nes, FLAGS_6502_N, nes->y & 0x80);
    return 0;
}

u8 TSX(nes6502* nes) {
    nes->x = nes->stkp;
    nes6502_set_flag(nes, FLAGS_6502_Z, nes->x == 0);
    nes6502_set_flag(nes, FLAGS_6502_N, nes->x & 0x80);
    return 0;
}

u8 TXA(nes6502* nes) {
    nes->a = nes->x;
    nes6502_set_flag(nes, FLAGS_6502_Z, nes->a == 0);
    nes6502_set_flag(nes, FLAGS_6502_N, nes->a & 0x80);
    return 0;
}

u8 TXS(nes6502* nes) {
    nes->stkp = nes->x;
    return 0;
}

u8 TYA(nes6502* nes) {
    nes->a = nes->y;
    nes6502_set_flag(nes, FLAGS_6502_Z, nes->a == 0);
    nes6502_set_flag(nes, FLAGS_6502_N, nes->a & 0x80);
    return 0;
}

u8 XXX(nes6502* nes) {
    printf("XXX / ILLEGAL OPCODE RUN!\n");
    return 0;
}

// -- OPCODES --