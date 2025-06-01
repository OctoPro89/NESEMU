const FLAGS_6502 = {
    C : (1 << 0),
    Z : (1 << 1),
    I : (1 << 2),
    D : (1 << 3),
    B : (1 << 4),
    U : (1 << 5),
    V : (1 << 6),
    N : (1 << 7)
};

class instruction {
    constructor(name, operate, addrmode, cycles) {
        this.name = name;
        this.operate = operate;
        this.addrmode = addrmode;
        this.cycles = cycles;
    }
}

class nes6502 {
    nes6502_reset() {
        this.addr_abs = 0xFFFC;
        let lo = nes6502_read(nes, this.addr_abs);
        let hi = nes6502_read(nes, this.addr_abs + 1);

        this.pc = (hi << 8) | lo;

        this.a = 0;
        this.x = 0;
        this.y = 0;
        this.stkp = 0xFD;
        this.status = 0x00 | FLAGS_6502.U;

        this.addr_rel = 0x0000;
        this.addr_abs = 0x0000;
        this.fetched = 0x00;

        this.cycles = 8;
    }

    nes6502_irq() {
        if (get_flag(FLAGS_6502.I) == 0) {
            nes6502_write(nes, 0x0100 + this.stkp, (this.pc >> 8) & 0x00FF);
            this.stkp--;
            nes6502_write(nes, 0x0100 + this.stkp, this.pc & 0x00FF);
            this.stkp--;

            set_flag(FLAGS_6502.B, 0);
            set_flag(FLAGS_6502.U, 1);
            set_flag(FLAGS_6502.I, 1);
            nes6502_write(nes, 0x0100 + this.stkp, this.status);
            this.stkp--;

            this.addr_abs = 0xFFFE;
            let lo = nes6502_read(nes, this.addr_abs);
            let hi = nes6502_read(nes, this.addr_abs + 1);
            this.pc = (hi << 8) | lo;

            this.cycles = 7;
        }
    }

    nes6502_nmi() {
        nes6502_write(nes, 0x0100 + this.stkp, (this.pc >> 8) & 0x00FF);
        this.stkp--;
        nes6502_write(nes, 0x0100 + this.stkp, this.pc & 0x00FF);
        this.stkp--;

        set_flag(FLAGS_6502.B, 0);
        set_flag(FLAGS_6502.U, 1);
        set_flag(FLAGS_6502.I, 1);
        nes6502_write(nes, 0x0100 + this.stkp, this.status);
        this.stkp--;

        this.addr_abs = 0xFFFA;
        let lo = nes6502_read(nes, this.addr_abs);
        let hi = nes6502_read(nes, this.addr_abs + 1);
        this.pc = (hi << 8) | lo;

        this.cycles = 8;
    }

    nes6502_clock() {
        if (this.cycles == 0) {
            this.opcode = nes6502_read(nes, this.pc);

            set_flag(FLAGS_6502.U, true);

            ++this.pc;

            this.cycles = this.lookup[this.opcode].cycles;

            additional_cycle1 = this.lookup[this.opcode].addrmode(nes);
            additional_cycle2 = this.lookup[this.opcode].operate(nes);

            this.cycles += (additional_cycle1 & additional_cycle2);

            set_flag(FLAGS_6502.U, true);
        }

        ++this.clock_count;

        --this.cycles;
    }

    nes6502_complete() {
        return this.cycles == 0;
    }

    nes6502_connect_bus(b) {
        this.b = b;
    }

    nes6502_get_flag(f) {
        return ((this.status & f) > 0) ? 1 : 0;
    }

    nes6502_set_flag(f, v) {
        if (v) {
            this.status |= f;
        } else {
            this.status &= ~f;
        }
    }

    nes6502_read(a) {
        return bus_cpu_read(this.b, a, false);
    }

    nes6502_write(a, d) {
        bus_cpu_write(this.b, a, d);
    }

    nes6502_fetch() {
        if (this.lookup[this.opcode].addrmode != IMP) {
            this.fetched = nes6502_read(nes, this.addr_abs);
        }

        return this.fetched;
    }

    // -- ADDRESSING MODES --

    IMP() {
        this.fetched = this.a;
        return 0;
    }

    IMM() {
        this.addr_abs = this.pc++;
        return 0;
    }

    ZP0() {
        this.addr_abs = nes6502_read(nes, this.pc++);
        this.addr_abs &= 0x00FF;
        return 0;
    }

    ZPX() {
        this.addr_abs = (nes6502_read(nes, this.pc++) + this.x) & 0x00FF;
        return 0;
    }

    ZPY() {
        this.addr_abs = (nes6502_read(nes, this.pc++) + this.y) & 0x00FF;
        return 0;
    }

    REL() {
        this.addr_rel = nes6502_read(nes, this.pc++);
        if (this.addr_rel & 0x80)
            this.addr_rel |= 0xFF00;
        return 0;
    }

    ABS() {
        let lo = nes6502_read(nes, this.pc++);
        let hi = nes6502_read(nes, this.pc++);
        this.addr_abs = (hi << 8) | lo;
        return 0;
    }

    ABX() {
        let lo = nes6502_read(nes, this.pc++);
        let hi = nes6502_read(nes, this.pc++);
        this.addr_abs = ((hi << 8) | lo) + this.x;
        return ((this.addr_abs & 0xFF00) != (hi << 8));
    }

    ABY() {
        let lo = nes6502_read(nes, this.pc++);
        let hi = nes6502_read(nes, this.pc++);
        this.addr_abs = ((hi << 8) | lo) + this.y;
        return ((this.addr_abs & 0xFF00) != (hi << 8));
    }

    IND() {
        let ptr_lo = nes6502_read(nes, this.pc++);
        let ptr_hi = nes6502_read(nes, this.pc++);
        let ptr = (ptr_hi << 8) | ptr_lo;

        if (ptr_lo == 0x00FF) {
            this.addr_abs = (nes6502_read(nes, ptr & 0xFF00) << 8) | nes6502_read(nes, ptr);
        } else {
            this.addr_abs = (nes6502_read(nes, ptr + 1) << 8) | nes6502_read(nes, ptr);
        }
        return 0;
    }

    IZX() {
        let t = nes6502_read(nes, this.pc++);
        let lo = nes6502_read(nes, (u8)(t + this.x) & 0x00FF);
        let hi = nes6502_read(nes, (u8)(t + this.x + 1) & 0x00FF);
        this.addr_abs = (hi << 8) | lo;
        return 0;
    }

    IZY() {
        let t = nes6502_read(nes, this.pc++);
        let lo = nes6502_read(nes, t & 0x00FF);
        let hi = nes6502_read(nes, (t + 1) & 0x00FF);
        this.addr_abs = ((hi << 8) | lo) + this.y;
        return ((this.addr_abs & 0xFF00) != (hi << 8));
    }

    // -- ADDRESSING MODES --

    // -- OPCODES --

    ADC() {
        nes6502_fetch(nes);
        this.temp = this.a + this.fetched + get_flag(FLAGS_6502.C);
        set_flag(FLAGS_6502.C, this.temp > 255);
        set_flag(FLAGS_6502.Z, (this.temp & 0x00FF) == 0);
        set_flag(FLAGS_6502.V, (~(this.a ^ this.fetched) & (this.a ^ this.temp)) & 0x0080);
        set_flag(FLAGS_6502.N, this.temp & 0x80);
        this.a = this.temp & 0x00FF;
        return 1;
    }

    SBC() {
        nes6502_fetch(nes);
        let value = this.fetched ^ 0x00FF;
        this.temp = this.a + value + get_flag(FLAGS_6502.C);

        set_flag(FLAGS_6502.C, this.temp > 0xFF);
        set_flag(FLAGS_6502.Z, ((this.temp & 0x00FF) == 0));
        set_flag(FLAGS_6502.V, ((this.temp ^ this.a) & (this.temp ^ value) & 0x0080));
        set_flag(FLAGS_6502.N, this.temp & 0x0080);

        this.a = this.temp & 0x00FF;
        return 1;
    }


    AND() {
        nes6502_fetch(nes);
        this.a = this.a & this.fetched;
        set_flag(FLAGS_6502.Z, this.a == 0x00);
        set_flag(FLAGS_6502.N, this.a & 0x80);
        return 1;
    }

    ASL() {
        nes6502_fetch(nes);
        this.temp = (this.fetched << 1);

        set_flag(FLAGS_6502.C, this.fetched & 0x80);
        set_flag(FLAGS_6502.Z, (this.temp & 0x00FF) == 0x00);
        set_flag(FLAGS_6502.N, this.temp & 0x80);

        if (this.lookup[this.opcode].addrmode == IMP)
            this.a = this.temp & 0x00FF;
        else
            nes6502_write(nes, this.addr_abs, this.temp & 0x00FF);

        return 0;
    }


    BCC() {
        if (get_flag(FLAGS_6502.C) == 0) {
            this.cycles++;
            this.addr_abs = this.pc + this.addr_rel;
            if ((this.addr_abs & 0xFF00) != (this.pc & 0xFF00))
                this.cycles++;
            this.pc = this.addr_abs;
        }
        return 0;
    }

    BCS() {
        if (get_flag(FLAGS_6502.C) == 1) {
            this.cycles++;
            this.addr_abs = this.pc + this.addr_rel;
            if ((this.addr_abs & 0xFF00) != (this.pc & 0xFF00))
                this.cycles++;
            this.pc = this.addr_abs;
        }
        return 0;
    }

    BEQ() {
        if (get_flag(FLAGS_6502.Z) == 1) {
            this.cycles++;
            this.addr_abs = this.pc + this.addr_rel;
            if ((this.addr_abs & 0xFF00) != (this.pc & 0xFF00))
                this.cycles++;
            this.pc = this.addr_abs;
        }
        return 0;
    }

    BIT() {
        nes6502_fetch(nes);
        this.temp = this.a & this.fetched;
        set_flag(FLAGS_6502.Z, (this.temp & 0x00FF) == 0x00);
        set_flag(FLAGS_6502.N, this.fetched & (1 << 7));
        set_flag(FLAGS_6502.V, this.fetched & (1 << 6));
        return 0;
    }

    BMI() {
        if (get_flag(FLAGS_6502.N) == 1) {
            this.cycles++;
            this.addr_abs = this.pc + this.addr_rel;
            if ((this.addr_abs & 0xFF00) != (this.pc & 0xFF00))
                this.cycles++;
            this.pc = this.addr_abs;
        }
        return 0;
    }

    BNE() {
        if (get_flag(FLAGS_6502.Z) == 0) {
            this.cycles++;
            this.addr_abs = this.pc + this.addr_rel;
            if ((this.addr_abs & 0xFF00) != (this.pc & 0xFF00))
                this.cycles++;
            this.pc = this.addr_abs;
        }
        return 0;
    }

    BPL() {
        if (get_flag(FLAGS_6502.N) == 0) {
            this.cycles++;
            this.addr_abs = this.pc + this.addr_rel;
            if ((this.addr_abs & 0xFF00) != (this.pc & 0xFF00))
                this.cycles++;
            this.pc = this.addr_abs;
        }
        return 0;
    }

    BRK() {
        this.pc++;
        set_flag(FLAGS_6502.I, 1);
        nes6502_write(nes, 0x0100 + this.stkp, (this.pc >> 8) & 0x00FF);
        this.stkp--;
        nes6502_write(nes, 0x0100 + this.stkp, this.pc & 0x00FF);
        this.stkp--;
        set_flag(FLAGS_6502.B, 1);
        nes6502_write(nes, 0x0100 + this.stkp, this.status);
        this.stkp--;
        set_flag(FLAGS_6502.B, 0);
        let lo = nes6502_read(nes, 0xFFFE);
        let hi = nes6502_read(nes, 0xFFFF);
        this.pc = (hi << 8) | lo;
        return 0;
    }

    BVC() {
        if (get_flag(FLAGS_6502.V) == 0) {
            this.cycles++;
            this.addr_abs = this.pc + this.addr_rel;
            if ((this.addr_abs & 0xFF00) != (this.pc & 0xFF00)) this.cycles++;
            this.pc = this.addr_abs;
        }
        return 0;
    }

    BVS() {
        if (get_flag(FLAGS_6502.V) == 1) {
            this.cycles++;
            this.addr_abs = this.pc + this.addr_rel;
            if ((this.addr_abs & 0xFF00) != (this.pc & 0xFF00)) this.cycles++;
            this.pc = this.addr_abs;
        }
        return 0;
    }

    CLC() { set_flag(FLAGS_6502.C, false); return 0; }
    CLD() { set_flag(FLAGS_6502.D, false); return 0; }
    CLI() { set_flag(FLAGS_6502.I, false); return 0; }
    CLV() { set_flag(FLAGS_6502.V, false); return 0; }

    CMP() {
        nes6502_fetch(nes);
        this.temp = this.a - this.fetched;
        set_flag(FLAGS_6502.C, this.a >= this.fetched);
        set_flag(FLAGS_6502.Z, (this.temp & 0x00FF) == 0x00);
        set_flag(FLAGS_6502.N, this.temp & 0x80);
        return 1;
    }

    CPX() {
        nes6502_fetch(nes);
        this.temp = this.x - this.fetched;
        set_flag(FLAGS_6502.C, this.x >= this.fetched);
        set_flag(FLAGS_6502.Z, (this.temp & 0x00FF) == 0x00);
        set_flag(FLAGS_6502.N, this.temp & 0x80);
        return 0;
    }

    CPY() {
        nes6502_fetch(nes);
        this.temp = this.y - this.fetched;
        set_flag(FLAGS_6502.C, this.y >= this.fetched);
        set_flag(FLAGS_6502.Z, (this.temp & 0x00FF) == 0x00);
        set_flag(FLAGS_6502.N, this.temp & 0x80);
        return 0;
    }

    DEC() {
        nes6502_fetch(nes);
        this.temp = this.fetched - 1;
        nes6502_write(nes, this.addr_abs, this.temp & 0x00FF);
        set_flag(FLAGS_6502.Z, (this.temp & 0x00FF) == 0x00);
        set_flag(FLAGS_6502.N, this.temp & 0x80);
        return 0;
    }

    DEX() {
        this.x--;
        set_flag(FLAGS_6502.Z, this.x == 0x00);
        set_flag(FLAGS_6502.N, this.x & 0x80);
        return 0;
    }

    DEY() {
        this.y--;
        set_flag(FLAGS_6502.Z, this.y == 0x00);
        set_flag(FLAGS_6502.N, this.y & 0x80);
        return 0;
    }

    EOR() {
        nes6502_fetch(nes);
        this.a = this.a ^ this.fetched;
        set_flag(FLAGS_6502.Z, this.a == 0x00);
        set_flag(FLAGS_6502.N, this.a & 0x80);
        return 1;
    }

    INC() {
        nes6502_fetch(nes);
        this.temp = this.fetched + 1;
        nes6502_write(nes, this.addr_abs, this.temp & 0x00FF);
        set_flag(FLAGS_6502.Z, (this.temp & 0x00FF) == 0x00);
        set_flag(FLAGS_6502.N, this.temp & 0x80);
        return 0;
    }

    INX() {
        this.x++;
        set_flag(FLAGS_6502.Z, this.x == 0x00);
        set_flag(FLAGS_6502.N, this.x & 0x80);
        return 0;
    }

    INY() {
        this.y++;
        set_flag(FLAGS_6502.Z, this.y == 0x00);
        set_flag(FLAGS_6502.N, this.y & 0x80);
        return 0;
    }

    JMP() {
        this.pc = this.addr_abs;
        return 0;
    }

    JSR() {
        this.pc--;
        nes6502_write(nes, 0x0100 + this.stkp, (this.pc >> 8) & 0x00FF);
        this.stkp--;
        nes6502_write(nes, 0x0100 + this.stkp, this.pc & 0x00FF);
        this.stkp--;
        this.pc = this.addr_abs;
        return 0;
    }


    LDA() {
        nes6502_fetch(nes);
        this.a = this.fetched;
        set_flag(FLAGS_6502.Z, this.a == 0x00);
        set_flag(FLAGS_6502.N, this.a & 0x80);
        return 1;
    }

    LDX() {
        nes6502_fetch(nes);
        this.x = this.fetched;
        set_flag(FLAGS_6502.Z, this.x == 0x00);
        set_flag(FLAGS_6502.N, this.x & 0x80);
        return 1;
    }

    LDY() {
        nes6502_fetch(nes);
        this.y = this.fetched;
        set_flag(FLAGS_6502.Z, this.y == 0x00);
        set_flag(FLAGS_6502.N, this.y & 0x80);
        return 1;
    }

    LSR() {
        nes6502_fetch(nes);
        set_flag(FLAGS_6502.C, this.fetched & 0x01);
        this.temp = this.fetched >> 1;
        set_flag(FLAGS_6502.Z, (this.temp & 0x00FF) == 0x00);
        set_flag(FLAGS_6502.N, this.temp & 0x80);
        if (this.lookup[this.opcode].addrmode == IMP)
            this.a = this.temp & 0x00FF;
        else
            nes6502_write(nes, this.addr_abs, this.temp & 0x00FF);
        return 0;
    }

    NOP() {
        switch (this.opcode) {
            case 0x1C: case 0x3C: case 0x5C:
            case 0x7C: case 0xDC: case 0xFC:
                return 1;
        }
        return 0;
    }

    ORA() {
        nes6502_fetch(nes);
        this.a = this.a | this.fetched;
        set_flag(FLAGS_6502.Z, this.a == 0x00);
        set_flag(FLAGS_6502.N, this.a & 0x80);
        return 1;
    }

    PHA() {
        nes6502_write(nes, 0x0100 + this.stkp, this.a);
        this.stkp--;
        return 0;
    }

    PHP() {
        nes6502_write(nes, 0x0100 + this.stkp, this.status | FLAGS_6502.B | FLAGS_6502.U);
        set_flag(FLAGS_6502.B, 0);
        set_flag(FLAGS_6502.U, 0);
        this.stkp--;
        return 0;
    }

    PLA() {
        this.stkp++;
        this.a = nes6502_read(nes, 0x0100 + this.stkp);
        set_flag(FLAGS_6502.Z, this.a == 0x00);
        set_flag(FLAGS_6502.N, this.a & 0x80);
        return 0;
    }

    PLP() {
        this.stkp++;
        this.status = nes6502_read(nes, 0x0100 + this.stkp);
        set_flag(FLAGS_6502.U, 1);
        return 0;
    }

    ROL() {
        nes6502_fetch(nes);

        let carry_in = get_flag(FLAGS_6502.C);
        set_flag(FLAGS_6502.C, this.fetched & 0x80);  // bit 7 into carry

        this.temp = (this.fetched << 1) | carry_in;

        set_flag(FLAGS_6502.Z, (this.temp & 0x00FF) == 0);
        set_flag(FLAGS_6502.N, this.temp & 0x80);

        if (this.lookup[this.opcode].addrmode == IMP)
            this.a = this.temp & 0x00FF;
        else
            nes6502_write(nes, this.addr_abs, this.temp & 0x00FF);

        return 0;
    }

    ROR() {
        nes6502_fetch(nes);

        let carry_in = get_flag(FLAGS_6502.C) << 7;
        set_flag(FLAGS_6502.C, this.fetched & 0x01);  // bit 0 into carry

        this.temp = (this.fetched >> 1) | carry_in;

        set_flag(FLAGS_6502.Z, (this.temp & 0x00FF) == 0);
        set_flag(FLAGS_6502.N, this.temp & 0x80);

        if (this.lookup[this.opcode].addrmode == IMP)
            this.a = this.temp & 0x00FF;
        else
            nes6502_write(nes, this.addr_abs, this.temp & 0x00FF);

        return 0;
    }

    RTI() {
        this.stkp++;
        this.status = nes6502_read(nes, 0x0100 + this.stkp);
        this.status &= ~FLAGS_6502.B;
        this.status &= ~FLAGS_6502.U;

        this.stkp++;
        this.pc = nes6502_read(nes, 0x0100 + this.stkp);
        this.stkp++;
        this.pc |= (nes6502_read(nes, 0x0100 + this.stkp) << 8);
        return 0;
    }

    RTS() {
        this.stkp++;
        this.pc = nes6502_read(nes, 0x0100 + this.stkp);
        this.stkp++;
        this.pc |= (nes6502_read(nes, 0x0100 + this.stkp) << 8);
        this.pc++;
        return 0;
    }

    SEC() { set_flag(FLAGS_6502.C, 1); return 0; }
    SED() { set_flag(FLAGS_6502.D, 1); return 0; }
    SEI() { set_flag(FLAGS_6502.I, 1); return 0; }

    STA() {
        nes6502_write(nes, this.addr_abs, this.a);
        return 0;
    }

    STX() {
        nes6502_write(nes, this.addr_abs, this.x);
        return 0;
    }

    STY() {
        nes6502_write(nes, this.addr_abs, this.y);
        return 0;
    }

    TAX() {
        this.x = this.a;
        set_flag(FLAGS_6502.Z, this.x == 0);
        set_flag(FLAGS_6502.N, this.x & 0x80);
        return 0;
    }

    TAY() {
        this.y = this.a;
        set_flag(FLAGS_6502.Z, this.y == 0);
        set_flag(FLAGS_6502.N, this.y & 0x80);
        return 0;
    }

    TSX() {
        this.x = this.stkp;
        set_flag(FLAGS_6502.Z, this.x == 0);
        set_flag(FLAGS_6502.N, this.x & 0x80);
        return 0;
    }

    TXA() {
        this.a = this.x;
        set_flag(FLAGS_6502.Z, this.a == 0);
        set_flag(FLAGS_6502.N, this.a & 0x80);
        return 0;
    }

    TXS() {
        this.stkp = this.x;
        return 0;
    }

    TYA() {
        this.a = this.y;
        set_flag(FLAGS_6502.Z, this.a == 0);
        set_flag(FLAGS_6502.N, this.a & 0x80);
        return 0;
    }

    XXX() {
        printf("XXX / ILLEGAL OPCODE RUN!\n");
        return 0;
    }

    // -- OPCODES --

    constructor() {
        this.a = 0;
        this.x = 0;
        this.y = 0;
        this.stkp = 0;
        this.pc = 0;
        this.status = 0;
        this.fetched = 0;
        this.temp = 0;
        this.addr_abs = 0;
        this.addr_rel = 0;
        this.opcode = 0;
        this.cycles = 0;
        this.clock_count = 0;

        this.bus = null;
        this.lookup = new Array(256);
        let i = 0;
        this.lookup[i] = new instruction("BRK", BRK, IMM, 7 ); ++i;
        this.lookup[i] = new instruction("ORA", ORA, IZX, 6 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 8 ); ++i;
        this.lookup[i] = new instruction("???", NOP, IMP, 3 ); ++i;
        this.lookup[i] = new instruction("ORA", ORA, ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("ASL", ASL, ZP0, 5 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 5 ); ++i;
        this.lookup[i] = new instruction("PHP", PHP, IMP, 3 ); ++i;
        this.lookup[i] = new instruction("ORA", ORA, IMM, 2 ); ++i;
        this.lookup[i] = new instruction("ASL", ASL, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", NOP, IMP, 4 ); ++i;
        this.lookup[i] = new instruction("ORA", ORA, ABS, 4 ); ++i;
        this.lookup[i] = new instruction("ASL", ASL, ABS, 6 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 6 ); ++i;
        this.lookup[i] = new instruction("BPL", BPL, REL, 2 ); ++i;
        this.lookup[i] = new instruction("ORA", ORA, IZY, 5 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 8 ); ++i;
        this.lookup[i] = new instruction("???", NOP, IMP, 4 ); ++i;
        this.lookup[i] = new instruction("ORA", ORA, ZPX, 4 ); ++i;
        this.lookup[i] = new instruction("ASL", ASL, ZPX, 6 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 6 ); ++i;
        this.lookup[i] = new instruction("CLC", CLC, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("ORA", ORA, ABY, 4 ); ++i;
        this.lookup[i] = new instruction("???", NOP, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 7 ); ++i;
        this.lookup[i] = new instruction("???", NOP, IMP, 4 ); ++i;
        this.lookup[i] = new instruction("ORA", ORA, ABX, 4 ); ++i;
        this.lookup[i] = new instruction("ASL", ASL, ABX, 7 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 7 ); ++i;
        this.lookup[i] = new instruction("JSR", JSR, ABS, 6 ); ++i;
        this.lookup[i] = new instruction("AND", AND, IZX, 6 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 8 ); ++i;
        this.lookup[i] = new instruction("BIT", BIT, ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("AND", AND, ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("ROL", ROL, ZP0, 5 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 5 ); ++i;
        this.lookup[i] = new instruction("PLP", PLP, IMP, 4 ); ++i;
        this.lookup[i] = new instruction("AND", AND, IMM, 2 ); ++i;
        this.lookup[i] = new instruction("ROL", ROL, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("BIT", BIT, ABS, 4 ); ++i;
        this.lookup[i] = new instruction("AND", AND, ABS, 4 ); ++i;
        this.lookup[i] = new instruction("ROL", ROL, ABS, 6 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 6 ); ++i;
        this.lookup[i] = new instruction("BMI", BMI, REL, 2 ); ++i;
        this.lookup[i] = new instruction("AND", AND, IZY, 5 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 8 ); ++i;
        this.lookup[i] = new instruction("???", NOP, IMP, 4 ); ++i;
        this.lookup[i] = new instruction("AND", AND, ZPX, 4 ); ++i;
        this.lookup[i] = new instruction("ROL", ROL, ZPX, 6 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 6 ); ++i;
        this.lookup[i] = new instruction("SEC", SEC, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("AND", AND, ABY, 4 ); ++i;
        this.lookup[i] = new instruction("???", NOP, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 7 ); ++i;
        this.lookup[i] = new instruction("???", NOP, IMP, 4 ); ++i;
        this.lookup[i] = new instruction("AND", AND, ABX, 4 ); ++i;
        this.lookup[i] = new instruction("ROL", ROL, ABX, 7 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 7 ); ++i;
        this.lookup[i] = new instruction("RTI", RTI, IMP, 6 ); ++i;
        this.lookup[i] = new instruction("EOR", EOR, IZX, 6 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 8 ); ++i;
        this.lookup[i] = new instruction("???", NOP, IMP, 3 ); ++i;
        this.lookup[i] = new instruction("EOR", EOR, ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("LSR", LSR, ZP0, 5 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 5 ); ++i;
        this.lookup[i] = new instruction("PHA", PHA, IMP, 3 ); ++i;
        this.lookup[i] = new instruction("EOR", EOR, IMM, 2 ); ++i;
        this.lookup[i] = new instruction("LSR", LSR, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("JMP", JMP, ABS, 3 ); ++i;
        this.lookup[i] = new instruction("EOR", EOR, ABS, 4 ); ++i;
        this.lookup[i] = new instruction("LSR", LSR, ABS, 6 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 6 ); ++i;
        this.lookup[i] = new instruction("BVC", BVC, REL, 2 ); ++i;
        this.lookup[i] = new instruction("EOR", EOR, IZY, 5 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 8 ); ++i;
        this.lookup[i] = new instruction("???", NOP, IMP, 4 ); ++i;
        this.lookup[i] = new instruction("EOR", EOR, ZPX, 4 ); ++i;
        this.lookup[i] = new instruction("LSR", LSR, ZPX, 6 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 6 ); ++i;
        this.lookup[i] = new instruction("CLI", CLI, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("EOR", EOR, ABY, 4 ); ++i;
        this.lookup[i] = new instruction("???", NOP, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 7 ); ++i;
        this.lookup[i] = new instruction("???", NOP, IMP, 4 ); ++i;
        this.lookup[i] = new instruction("EOR", EOR, ABX, 4 ); ++i;
        this.lookup[i] = new instruction("LSR", LSR, ABX, 7 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 7 ); ++i;
        this.lookup[i] = new instruction("RTS", RTS, IMP, 6 ); ++i;
        this.lookup[i] = new instruction("ADC", ADC, IZX, 6 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 8 ); ++i;
        this.lookup[i] = new instruction("???", NOP, IMP, 3 ); ++i;
        this.lookup[i] = new instruction("ADC", ADC, ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("ROR", ROR, ZP0, 5 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 5 ); ++i;
        this.lookup[i] = new instruction("PLA", PLA, IMP, 4 ); ++i;
        this.lookup[i] = new instruction("ADC", ADC, IMM, 2 ); ++i;
        this.lookup[i] = new instruction("ROR", ROR, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("JMP", JMP, IND, 5 ); ++i;
        this.lookup[i] = new instruction("ADC", ADC, ABS, 4 ); ++i;
        this.lookup[i] = new instruction("ROR", ROR, ABS, 6 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 6 ); ++i;
        this.lookup[i] = new instruction("BVS", BVS, REL, 2 ); ++i;
        this.lookup[i] = new instruction("ADC", ADC, IZY, 5 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 8 ); ++i;
        this.lookup[i] = new instruction("???", NOP, IMP, 4 ); ++i;
        this.lookup[i] = new instruction("ADC", ADC, ZPX, 4 ); ++i;
        this.lookup[i] = new instruction("ROR", ROR, ZPX, 6 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 6 ); ++i;
        this.lookup[i] = new instruction("SEI", SEI, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("ADC", ADC, ABY, 4 ); ++i;
        this.lookup[i] = new instruction("???", NOP, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 7 ); ++i;
        this.lookup[i] = new instruction("???", NOP, IMP, 4 ); ++i;
        this.lookup[i] = new instruction("ADC", ADC, ABX, 4 ); ++i;
        this.lookup[i] = new instruction("ROR", ROR, ABX, 7 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 7 ); ++i;
        this.lookup[i] = new instruction("???", NOP, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("STA", STA, IZX, 6 ); ++i;
        this.lookup[i] = new instruction("???", NOP, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 6 ); ++i;
        this.lookup[i] = new instruction("STY", STY, ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("STA", STA, ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("STX", STX, ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 3 ); ++i;
        this.lookup[i] = new instruction("DEY", DEY, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", NOP, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("TXA", TXA, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("STY", STY, ABS, 4 ); ++i;
        this.lookup[i] = new instruction("STA", STA, ABS, 4 ); ++i;
        this.lookup[i] = new instruction("STX", STX, ABS, 4 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 4 ); ++i;
        this.lookup[i] = new instruction("BCC", BCC, REL, 2 ); ++i;
        this.lookup[i] = new instruction("STA", STA, IZY, 6 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 6 ); ++i;
        this.lookup[i] = new instruction("STY", STY, ZPX, 4 ); ++i;
        this.lookup[i] = new instruction("STA", STA, ZPX, 4 ); ++i;
        this.lookup[i] = new instruction("STX", STX, ZPY, 4 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 4 ); ++i;
        this.lookup[i] = new instruction("TYA", TYA, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("STA", STA, ABY, 5 ); ++i;
        this.lookup[i] = new instruction("TXS", TXS, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 5 ); ++i;
        this.lookup[i] = new instruction("???", NOP, IMP, 5 ); ++i;
        this.lookup[i] = new instruction("STA", STA, ABX, 5 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 5 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 5 ); ++i;
        this.lookup[i] = new instruction("LDY", LDY, IMM, 2 ); ++i;
        this.lookup[i] = new instruction("LDA", LDA, IZX, 6 ); ++i;
        this.lookup[i] = new instruction("LDX", LDX, IMM, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 6 ); ++i;
        this.lookup[i] = new instruction("LDY", LDY, ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("LDA", LDA, ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("LDX", LDX, ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 3 ); ++i;
        this.lookup[i] = new instruction("TAY", TAY, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("LDA", LDA, IMM, 2 ); ++i;
        this.lookup[i] = new instruction("TAX", TAX, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("LDY", LDY, ABS, 4 ); ++i;
        this.lookup[i] = new instruction("LDA", LDA, ABS, 4 ); ++i;
        this.lookup[i] = new instruction("LDX", LDX, ABS, 4 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 4 ); ++i;
        this.lookup[i] = new instruction("BCS", BCS, REL, 2 ); ++i;
        this.lookup[i] = new instruction("LDA", LDA, IZY, 5 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 5 ); ++i;
        this.lookup[i] = new instruction("LDY", LDY, ZPX, 4 ); ++i;
        this.lookup[i] = new instruction("LDA", LDA, ZPX, 4 ); ++i;
        this.lookup[i] = new instruction("LDX", LDX, ZPY, 4 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 4 ); ++i;
        this.lookup[i] = new instruction("CLV", CLV, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("LDA", LDA, ABY, 4 ); ++i;
        this.lookup[i] = new instruction("TSX", TSX, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 4 ); ++i;
        this.lookup[i] = new instruction("LDY", LDY, ABX, 4 ); ++i;
        this.lookup[i] = new instruction("LDA", LDA, ABX, 4 ); ++i;
        this.lookup[i] = new instruction("LDX", LDX, ABY, 4 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 4 ); ++i;
        this.lookup[i] = new instruction("CPY", CPY, IMM, 2 ); ++i;
        this.lookup[i] = new instruction("CMP", CMP, IZX, 6 ); ++i;
        this.lookup[i] = new instruction("???", NOP, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 8 ); ++i;
        this.lookup[i] = new instruction("CPY", CPY, ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("CMP", CMP, ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("DEC", DEC, ZP0, 5 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 5 ); ++i;
        this.lookup[i] = new instruction("INY", INY, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("CMP", CMP, IMM, 2 ); ++i;
        this.lookup[i] = new instruction("DEX", DEX, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("CPY", CPY, ABS, 4 ); ++i;
        this.lookup[i] = new instruction("CMP", CMP, ABS, 4 ); ++i;
        this.lookup[i] = new instruction("DEC", DEC, ABS, 6 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 6 ); ++i;
        this.lookup[i] = new instruction("BNE", BNE, REL, 2 ); ++i;
        this.lookup[i] = new instruction("CMP", CMP, IZY, 5 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 8 ); ++i;
        this.lookup[i] = new instruction("???", NOP, IMP, 4 ); ++i;
        this.lookup[i] = new instruction("CMP", CMP, ZPX, 4 ); ++i;
        this.lookup[i] = new instruction("DEC", DEC, ZPX, 6 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 6 ); ++i;
        this.lookup[i] = new instruction("CLD", CLD, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("CMP", CMP, ABY, 4 ); ++i;
        this.lookup[i] = new instruction("NOP", NOP, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 7 ); ++i;
        this.lookup[i] = new instruction("???", NOP, IMP, 4 ); ++i;
        this.lookup[i] = new instruction("CMP", CMP, ABX, 4 ); ++i;
        this.lookup[i] = new instruction("DEC", DEC, ABX, 7 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 7 ); ++i;
        this.lookup[i] = new instruction("CPX", CPX, IMM, 2 ); ++i;
        this.lookup[i] = new instruction("SBC", SBC, IZX, 6 ); ++i;
        this.lookup[i] = new instruction("???", NOP, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 8 ); ++i;
        this.lookup[i] = new instruction("CPX", CPX, ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("SBC", SBC, ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("INC", INC, ZP0, 5 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 5 ); ++i;
        this.lookup[i] = new instruction("INX", INX, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("SBC", SBC, IMM, 2 ); ++i;
        this.lookup[i] = new instruction("NOP", NOP, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", SBC, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("CPX", CPX, ABS, 4 ); ++i;
        this.lookup[i] = new instruction("SBC", SBC, ABS, 4 ); ++i;
        this.lookup[i] = new instruction("INC", INC, ABS, 6 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 6 ); ++i;
        this.lookup[i] = new instruction("BEQ", BEQ, REL, 2 ); ++i;
        this.lookup[i] = new instruction("SBC", SBC, IZY, 5 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 8 ); ++i;
        this.lookup[i] = new instruction("???", NOP, IMP, 4 ); ++i;
        this.lookup[i] = new instruction("SBC", SBC, ZPX, 4 ); ++i;
        this.lookup[i] = new instruction("INC", INC, ZPX, 6 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 6 ); ++i;
        this.lookup[i] = new instruction("SED", SED, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("SBC", SBC, ABY, 4 ); ++i;
        this.lookup[i] = new instruction("NOP", NOP, IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 7 ); ++i;
        this.lookup[i] = new instruction("???", NOP, IMP, 4 ); ++i;
        this.lookup[i] = new instruction("SBC", SBC, ABX, 4 ); ++i;
        this.lookup[i] = new instruction("INC", INC, ABX, 7 ); ++i;
        this.lookup[i] = new instruction("???", XXX, IMP, 7 );
    }
}