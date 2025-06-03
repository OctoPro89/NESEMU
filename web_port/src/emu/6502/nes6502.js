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
    reset() {
        this.addr_abs = 0xFFFC;
        let lo = this.read(this.addr_abs);
        let hi = this.read(this.addr_abs + 1);

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

    irq() {
        if (get_flag(FLAGS_6502.I) == 0) {
            this.write(0x0100 + this.stkp, (this.pc >> 8) & 0x00FF);
            this.stkp--;
            this.write(0x0100 + this.stkp, this.pc & 0x00FF);
            this.stkp--;

            this.set_flag(FLAGS_6502.B, 0);
            this.set_flag(FLAGS_6502.U, 1);
            this.set_flag(FLAGS_6502.I, 1);
            this.write(0x0100 + this.stkp, this.status);
            this.stkp--;

            this.addr_abs = 0xFFFE;
            let lo = this.read(this.addr_abs);
            let hi = this.read(this.addr_abs + 1);
            this.pc = (hi << 8) | lo;

            this.cycles = 7;
        }
    }

    nmi() {
        this.write(0x0100 + this.stkp, (this.pc >> 8) & 0x00FF);
        this.stkp--;
        this.write(0x0100 + this.stkp, this.pc & 0x00FF);
        this.stkp--;

        this.set_flag(FLAGS_6502.B, 0);
        this.set_flag(FLAGS_6502.U, 1);
        this.set_flag(FLAGS_6502.I, 1);
        this.write(0x0100 + this.stkp, this.status);
        this.stkp--;

        this.addr_abs = 0xFFFA;
        let lo = this.read(this.addr_abs);
        let hi = this.read(this.addr_abs + 1);
        this.pc = (hi << 8) | lo;

        this.cycles = 8;
    }

    clock() {
        if (this.cycles == 0) {
            this.opcode = this.read(this.pc);

            this.set_flag(FLAGS_6502.U, true);

            ++this.pc;

            this.cycles = this.lookup[this.opcode].cycles;

            additional_cycle1 = this.lookup[this.opcode].addrmode();
            additional_cycle2 = this.lookup[this.opcode].operate();

            this.cycles += (additional_cycle1 & additional_cycle2);

            this.set_flag(FLAGS_6502.U, true);
        }

        ++this.clock_count;

        --this.cycles;
    }

    complete() {
        return this.cycles == 0;
    }

    connect_bus(b) {
        this.b = b;
    }

    get_flag(f) {
        return ((this.status & f) > 0) ? 1 : 0;
    }

    set_flag(f, v) {
        if (v) {
            this.status |= f;
        } else {
            this.status &= ~f;
        }
    }

    read(a) {
        return bus_cpu_read(this.b, a, false);
    }

    write(a, d) {
        bus_cpu_write(this.b, a, d);
    }

    fetch() {
        if (this.lookup[this.opcode].addrmode != this.IMP) {
            this.fetched = this.read(this.addr_abs);
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
        this.addr_abs = this.read(this.pc++);
        this.addr_abs &= 0x00FF;
        return 0;
    }

    ZPX() {
        this.addr_abs = (this.read(this.pc++) + this.x) & 0x00FF;
        return 0;
    }

    ZPY() {
        this.addr_abs = (this.read(this.pc++) + this.y) & 0x00FF;
        return 0;
    }

    REL() {
        this.addr_rel = this.read(this.pc++);
        if (this.addr_rel & 0x80)
            this.addr_rel |= 0xFF00;
        return 0;
    }

    ABS() {
        let lo = this.read(this.pc++);
        let hi = this.read(this.pc++);
        this.addr_abs = (hi << 8) | lo;
        return 0;
    }

    ABX() {
        let lo = this.read(this.pc++);
        let hi = this.read(this.pc++);
        this.addr_abs = ((hi << 8) | lo) + this.x;
        return ((this.addr_abs & 0xFF00) != (hi << 8));
    }

    ABY() {
        let lo = this.read(this.pc++);
        let hi = this.read(this.pc++);
        this.addr_abs = ((hi << 8) | lo) + this.y;
        return ((this.addr_abs & 0xFF00) != (hi << 8));
    }

    IND() {
        let ptr_lo = this.read(this.pc++);
        let ptr_hi = this.read(this.pc++);
        let ptr = (ptr_hi << 8) | ptr_lo;

        if (ptr_lo == 0x00FF) {
            this.addr_abs = (this.read(ptr & 0xFF00) << 8) | this.read(ptr);
        } else {
            this.addr_abs = (this.read(ptr + 1) << 8) | this.read(ptr);
        }
        return 0;
    }

    IZX() {
        let t = this.read(this.pc++);
        let lo = this.read((u8)(t + this.x) & 0x00FF);
        let hi = this.read((u8)(t + this.x + 1) & 0x00FF);
        this.addr_abs = (hi << 8) | lo;
        return 0;
    }

    IZY() {
        let t = this.read(this.pc++);
        let lo = this.read(t & 0x00FF);
        let hi = this.read((t + 1) & 0x00FF);
        this.addr_abs = ((hi << 8) | lo) + this.y;
        return ((this.addr_abs & 0xFF00) != (hi << 8));
    }

    // -- ADDRESSING MODES --

    // -- OPCODES --

    ADC() {
        this.fetch()
        this.temp = this.a + this.fetched + get_flag(FLAGS_6502.C);
        this.set_flag(FLAGS_6502.C, this.temp > 255);
        this.set_flag(FLAGS_6502.Z, (this.temp & 0x00FF) == 0);
        this.set_flag(FLAGS_6502.V, (~(this.a ^ this.fetched) & (this.a ^ this.temp)) & 0x0080);
        this.set_flag(FLAGS_6502.N, this.temp & 0x80);
        this.a = this.temp & 0x00FF;
        return 1;
    }

    SBC() {
        this.fetch()
        let value = this.fetched ^ 0x00FF;
        this.temp = this.a + value + get_flag(FLAGS_6502.C);

        this.set_flag(FLAGS_6502.C, this.temp > 0xFF);
        this.set_flag(FLAGS_6502.Z, ((this.temp & 0x00FF) == 0));
        this.set_flag(FLAGS_6502.V, ((this.temp ^ this.a) & (this.temp ^ value) & 0x0080));
        this.set_flag(FLAGS_6502.N, this.temp & 0x0080);

        this.a = this.temp & 0x00FF;
        return 1;
    }


    AND() {
        this.fetch()
        this.a = this.a & this.fetched;
        this.set_flag(FLAGS_6502.Z, this.a == 0x00);
        this.set_flag(FLAGS_6502.N, this.a & 0x80);
        return 1;
    }

    ASL() {
        this.fetch()
        this.temp = (this.fetched << 1);

        this.set_flag(FLAGS_6502.C, this.fetched & 0x80);
        this.set_flag(FLAGS_6502.Z, (this.temp & 0x00FF) == 0x00);
        this.set_flag(FLAGS_6502.N, this.temp & 0x80);

        if (this.lookup[this.opcode].addrmode == IMP)
            this.a = this.temp & 0x00FF;
        else
            this.write(this.addr_abs, this.temp & 0x00FF);

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
        this.fetch()
        this.temp = this.a & this.fetched;
        this.set_flag(FLAGS_6502.Z, (this.temp & 0x00FF) == 0x00);
        this.set_flag(FLAGS_6502.N, this.fetched & (1 << 7));
        this.set_flag(FLAGS_6502.V, this.fetched & (1 << 6));
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
        this.set_flag(FLAGS_6502.I, 1);
        this.write(0x0100 + this.stkp, (this.pc >> 8) & 0x00FF);
        this.stkp--;
        this.write(0x0100 + this.stkp, this.pc & 0x00FF);
        this.stkp--;
        this.set_flag(FLAGS_6502.B, 1);
        this.write(0x0100 + this.stkp, this.status);
        this.stkp--;
        this.set_flag(FLAGS_6502.B, 0);
        let lo = this.read(0xFFFE);
        let hi = this.read(0xFFFF);
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

    CLC() { this.set_flag(FLAGS_6502.C, false); return 0; }
    CLD() { this.set_flag(FLAGS_6502.D, false); return 0; }
    CLI() { this.set_flag(FLAGS_6502.I, false); return 0; }
    CLV() { this.set_flag(FLAGS_6502.V, false); return 0; }

    CMP() {
        this.fetch()
        this.temp = this.a - this.fetched;
        this.set_flag(FLAGS_6502.C, this.a >= this.fetched);
        this.set_flag(FLAGS_6502.Z, (this.temp & 0x00FF) == 0x00);
        this.set_flag(FLAGS_6502.N, this.temp & 0x80);
        return 1;
    }

    CPX() {
        this.fetch()
        this.temp = this.x - this.fetched;
        this.set_flag(FLAGS_6502.C, this.x >= this.fetched);
        this.set_flag(FLAGS_6502.Z, (this.temp & 0x00FF) == 0x00);
        this.set_flag(FLAGS_6502.N, this.temp & 0x80);
        return 0;
    }

    CPY() {
        this.fetch()
        this.temp = this.y - this.fetched;
        this.set_flag(FLAGS_6502.C, this.y >= this.fetched);
        this.set_flag(FLAGS_6502.Z, (this.temp & 0x00FF) == 0x00);
        this.set_flag(FLAGS_6502.N, this.temp & 0x80);
        return 0;
    }

    DEC() {
        this.fetch()
        this.temp = this.fetched - 1;
        this.write(this.addr_abs, this.temp & 0x00FF);
        this.set_flag(FLAGS_6502.Z, (this.temp & 0x00FF) == 0x00);
        this.set_flag(FLAGS_6502.N, this.temp & 0x80);
        return 0;
    }

    DEX() {
        this.x--;
        this.set_flag(FLAGS_6502.Z, this.x == 0x00);
        this.set_flag(FLAGS_6502.N, this.x & 0x80);
        return 0;
    }

    DEY() {
        this.y--;
        this.set_flag(FLAGS_6502.Z, this.y == 0x00);
        this.set_flag(FLAGS_6502.N, this.y & 0x80);
        return 0;
    }

    EOR() {
        this.fetch()
        this.a = this.a ^ this.fetched;
        this.set_flag(FLAGS_6502.Z, this.a == 0x00);
        this.set_flag(FLAGS_6502.N, this.a & 0x80);
        return 1;
    }

    INC() {
        this.fetch()
        this.temp = this.fetched + 1;
        this.write(this.addr_abs, this.temp & 0x00FF);
        this.set_flag(FLAGS_6502.Z, (this.temp & 0x00FF) == 0x00);
        this.set_flag(FLAGS_6502.N, this.temp & 0x80);
        return 0;
    }

    INX() {
        this.x++;
        this.set_flag(FLAGS_6502.Z, this.x == 0x00);
        this.set_flag(FLAGS_6502.N, this.x & 0x80);
        return 0;
    }

    INY() {
        this.y++;
        this.set_flag(FLAGS_6502.Z, this.y == 0x00);
        this.set_flag(FLAGS_6502.N, this.y & 0x80);
        return 0;
    }

    JMP() {
        this.pc = this.addr_abs;
        return 0;
    }

    JSR() {
        this.pc--;
        this.write(0x0100 + this.stkp, (this.pc >> 8) & 0x00FF);
        this.stkp--;
        this.write(0x0100 + this.stkp, this.pc & 0x00FF);
        this.stkp--;
        this.pc = this.addr_abs;
        return 0;
    }


    LDA() {
        this.fetch()
        this.a = this.fetched;
        this.set_flag(FLAGS_6502.Z, this.a == 0x00);
        this.set_flag(FLAGS_6502.N, this.a & 0x80);
        return 1;
    }

    LDX() {
        this.fetch()
        this.x = this.fetched;
        this.set_flag(FLAGS_6502.Z, this.x == 0x00);
        this.set_flag(FLAGS_6502.N, this.x & 0x80);
        return 1;
    }

    LDY() {
        this.fetch()
        this.y = this.fetched;
        this.set_flag(FLAGS_6502.Z, this.y == 0x00);
        this.set_flag(FLAGS_6502.N, this.y & 0x80);
        return 1;
    }

    LSR() {
        this.fetch()
        this.set_flag(FLAGS_6502.C, this.fetched & 0x01);
        this.temp = this.fetched >> 1;
        this.set_flag(FLAGS_6502.Z, (this.temp & 0x00FF) == 0x00);
        this.set_flag(FLAGS_6502.N, this.temp & 0x80);
        if (this.lookup[this.opcode].addrmode == this.IMP)
            this.a = this.temp & 0x00FF;
        else
            this.write(this.addr_abs, this.temp & 0x00FF);
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
        this.fetch()
        this.a = this.a | this.fetched;
        this.set_flag(FLAGS_6502.Z, this.a == 0x00);
        this.set_flag(FLAGS_6502.N, this.a & 0x80);
        return 1;
    }

    PHA() {
        this.write(0x0100 + this.stkp, this.a);
        this.stkp--;
        return 0;
    }

    PHP() {
        this.write(0x0100 + this.stkp, this.status | FLAGS_6502.B | FLAGS_6502.U);
        this.set_flag(FLAGS_6502.B, 0);
        this.set_flag(FLAGS_6502.U, 0);
        this.stkp--;
        return 0;
    }

    PLA() {
        this.stkp++;
        this.a = this.read(0x0100 + this.stkp);
        this.set_flag(FLAGS_6502.Z, this.a == 0x00);
        this.set_flag(FLAGS_6502.N, this.a & 0x80);
        return 0;
    }

    PLP() {
        this.stkp++;
        this.status = this.read(0x0100 + this.stkp);
        this.set_flag(FLAGS_6502.U, 1);
        return 0;
    }

    ROL() {
        this.fetch()

        let carry_in = get_flag(FLAGS_6502.C);
        this.set_flag(FLAGS_6502.C, this.fetched & 0x80);  // bit 7 into carry

        this.temp = (this.fetched << 1) | carry_in;

        this.set_flag(FLAGS_6502.Z, (this.temp & 0x00FF) == 0);
        this.set_flag(FLAGS_6502.N, this.temp & 0x80);

        if (this.lookup[this.opcode].addrmode == this.IMP)
            this.a = this.temp & 0x00FF;
        else
            this.write(this.addr_abs, this.temp & 0x00FF);

        return 0;
    }

    ROR() {
        this.fetch()

        let carry_in = get_flag(FLAGS_6502.C) << 7;
        this.set_flag(FLAGS_6502.C, this.fetched & 0x01);  // bit 0 into carry

        this.temp = (this.fetched >> 1) | carry_in;

        this.set_flag(FLAGS_6502.Z, (this.temp & 0x00FF) == 0);
        this.set_flag(FLAGS_6502.N, this.temp & 0x80);

        if (this.lookup[this.opcode].addrmode == this.IMP)
            this.a = this.temp & 0x00FF;
        else
            this.write(this.addr_abs, this.temp & 0x00FF);

        return 0;
    }

    RTI() {
        this.stkp++;
        this.status = this.read(0x0100 + this.stkp);
        this.status &= ~FLAGS_6502.B;
        this.status &= ~FLAGS_6502.U;

        this.stkp++;
        this.pc = this.read(0x0100 + this.stkp);
        this.stkp++;
        this.pc |= (this.read(0x0100 + this.stkp) << 8);
        return 0;
    }

    RTS() {
        this.stkp++;
        this.pc = this.read(0x0100 + this.stkp);
        this.stkp++;
        this.pc |= (this.read(0x0100 + this.stkp) << 8);
        this.pc++;
        return 0;
    }

    SEC() { this.set_flag(FLAGS_6502.C, 1); return 0; }
    SED() { this.set_flag(FLAGS_6502.D, 1); return 0; }
    SEI() { this.set_flag(FLAGS_6502.I, 1); return 0; }

    STA() {
        this.write(this.addr_abs, this.a);
        return 0;
    }

    STX() {
        this.write(this.addr_abs, this.x);
        return 0;
    }

    STY() {
        this.write(this.addr_abs, this.y);
        return 0;
    }

    TAX() {
        this.x = this.a;
        this.set_flag(FLAGS_6502.Z, this.x == 0);
        this.set_flag(FLAGS_6502.N, this.x & 0x80);
        return 0;
    }

    TAY() {
        this.y = this.a;
        this.set_flag(FLAGS_6502.Z, this.y == 0);
        this.set_flag(FLAGS_6502.N, this.y & 0x80);
        return 0;
    }

    TSX() {
        this.x = this.stkp;
        this.set_flag(FLAGS_6502.Z, this.x == 0);
        this.set_flag(FLAGS_6502.N, this.x & 0x80);
        return 0;
    }

    TXA() {
        this.a = this.x;
        this.set_flag(FLAGS_6502.Z, this.a == 0);
        this.set_flag(FLAGS_6502.N, this.a & 0x80);
        return 0;
    }

    TXS() {
        this.stkp = this.x;
        return 0;
    }

    TYA() {
        this.a = this.y;
        this.set_flag(FLAGS_6502.Z, this.a == 0);
        this.set_flag(FLAGS_6502.N, this.a & 0x80);
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
        this.lookup[i] = new instruction("BRK", this.BRK, this.IMM, 7 ); ++i;
        this.lookup[i] = new instruction("ORA", this.ORA, this.IZX, 6 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 8 ); ++i;
        this.lookup[i] = new instruction("???", this.NOP, this.IMP, 3 ); ++i;
        this.lookup[i] = new instruction("ORA", this.ORA, this.ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("ASL", this.ASL, this.ZP0, 5 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 5 ); ++i;
        this.lookup[i] = new instruction("PHP", this.PHP, this.IMP, 3 ); ++i;
        this.lookup[i] = new instruction("ORA", this.ORA, this.IMM, 2 ); ++i;
        this.lookup[i] = new instruction("ASL", this.ASL, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.NOP, this.IMP, 4 ); ++i;
        this.lookup[i] = new instruction("ORA", this.ORA, this.ABS, 4 ); ++i;
        this.lookup[i] = new instruction("ASL", this.ASL, this.ABS, 6 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 6 ); ++i;
        this.lookup[i] = new instruction("BPL", this.BPL, this.REL, 2 ); ++i;
        this.lookup[i] = new instruction("ORA", this.ORA, this.IZY, 5 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 8 ); ++i;
        this.lookup[i] = new instruction("???", this.NOP, this.IMP, 4 ); ++i;
        this.lookup[i] = new instruction("ORA", this.ORA, this.ZPX, 4 ); ++i;
        this.lookup[i] = new instruction("ASL", this.ASL, this.ZPX, 6 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 6 ); ++i;
        this.lookup[i] = new instruction("CLC", this.CLC, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("ORA", this.ORA, this.ABY, 4 ); ++i;
        this.lookup[i] = new instruction("???", this.NOP, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 7 ); ++i;
        this.lookup[i] = new instruction("???", this.NOP, this.IMP, 4 ); ++i;
        this.lookup[i] = new instruction("ORA", this.ORA, this.ABX, 4 ); ++i;
        this.lookup[i] = new instruction("ASL", this.ASL, this.ABX, 7 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 7 ); ++i;
        this.lookup[i] = new instruction("JSR", this.JSR, this.ABS, 6 ); ++i;
        this.lookup[i] = new instruction("AND", this.AND, this.IZX, 6 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 8 ); ++i;
        this.lookup[i] = new instruction("BIT", this.BIT, this.ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("AND", this.AND, this.ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("ROL", this.ROL, this.ZP0, 5 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 5 ); ++i;
        this.lookup[i] = new instruction("PLP", this.PLP, this.IMP, 4 ); ++i;
        this.lookup[i] = new instruction("AND", this.AND, this.IMM, 2 ); ++i;
        this.lookup[i] = new instruction("ROL", this.ROL, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("BIT", this.BIT, this.ABS, 4 ); ++i;
        this.lookup[i] = new instruction("AND", this.AND, this.ABS, 4 ); ++i;
        this.lookup[i] = new instruction("ROL", this.ROL, this.ABS, 6 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 6 ); ++i;
        this.lookup[i] = new instruction("BMI", this.BMI, this.REL, 2 ); ++i;
        this.lookup[i] = new instruction("AND", this.AND, this.IZY, 5 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 8 ); ++i;
        this.lookup[i] = new instruction("???", this.NOP, this.IMP, 4 ); ++i;
        this.lookup[i] = new instruction("AND", this.AND, this.ZPX, 4 ); ++i;
        this.lookup[i] = new instruction("ROL", this.ROL, this.ZPX, 6 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 6 ); ++i;
        this.lookup[i] = new instruction("SEC", this.SEC, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("AND", this.AND, this.ABY, 4 ); ++i;
        this.lookup[i] = new instruction("???", this.NOP, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 7 ); ++i;
        this.lookup[i] = new instruction("???", this.NOP, this.IMP, 4 ); ++i;
        this.lookup[i] = new instruction("AND", this.AND, this.ABX, 4 ); ++i;
        this.lookup[i] = new instruction("ROL", this.ROL, this.ABX, 7 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 7 ); ++i;
        this.lookup[i] = new instruction("RTI", this.RTI, this.IMP, 6 ); ++i;
        this.lookup[i] = new instruction("EOR", this.EOR, this.IZX, 6 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 8 ); ++i;
        this.lookup[i] = new instruction("???", this.NOP, this.IMP, 3 ); ++i;
        this.lookup[i] = new instruction("EOR", this.EOR, this.ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("LSR", this.LSR, this.ZP0, 5 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 5 ); ++i;
        this.lookup[i] = new instruction("PHA", this.PHA, this.IMP, 3 ); ++i;
        this.lookup[i] = new instruction("EOR", this.EOR, this.IMM, 2 ); ++i;
        this.lookup[i] = new instruction("LSR", this.LSR, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("JMP", this.JMP, this.ABS, 3 ); ++i;
        this.lookup[i] = new instruction("EOR", this.EOR, this.ABS, 4 ); ++i;
        this.lookup[i] = new instruction("LSR", this.LSR, this.ABS, 6 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 6 ); ++i;
        this.lookup[i] = new instruction("BVC", this.BVC, this.REL, 2 ); ++i;
        this.lookup[i] = new instruction("EOR", this.EOR, this.IZY, 5 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 8 ); ++i;
        this.lookup[i] = new instruction("???", this.NOP, this.IMP, 4 ); ++i;
        this.lookup[i] = new instruction("EOR", this.EOR, this.ZPX, 4 ); ++i;
        this.lookup[i] = new instruction("LSR", this.LSR, this.ZPX, 6 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 6 ); ++i;
        this.lookup[i] = new instruction("CLI", this.CLI, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("EOR", this.EOR, this.ABY, 4 ); ++i;
        this.lookup[i] = new instruction("???", this.NOP, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 7 ); ++i;
        this.lookup[i] = new instruction("???", this.NOP, this.IMP, 4 ); ++i;
        this.lookup[i] = new instruction("EOR", this.EOR, this.ABX, 4 ); ++i;
        this.lookup[i] = new instruction("LSR", this.LSR, this.ABX, 7 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 7 ); ++i;
        this.lookup[i] = new instruction("RTS", this.RTS, this.IMP, 6 ); ++i;
        this.lookup[i] = new instruction("ADC", this.ADC, this.IZX, 6 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 8 ); ++i;
        this.lookup[i] = new instruction("???", this.NOP, this.IMP, 3 ); ++i;
        this.lookup[i] = new instruction("ADC", this.ADC, this.ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("ROR", this.ROR, this.ZP0, 5 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 5 ); ++i;
        this.lookup[i] = new instruction("PLA", this.PLA, this.IMP, 4 ); ++i;
        this.lookup[i] = new instruction("ADC", this.ADC, this.IMM, 2 ); ++i;
        this.lookup[i] = new instruction("ROR", this.ROR, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("JMP", this.JMP, this.IND, 5 ); ++i;
        this.lookup[i] = new instruction("ADC", this.ADC, this.ABS, 4 ); ++i;
        this.lookup[i] = new instruction("ROR", this.ROR, this.ABS, 6 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 6 ); ++i;
        this.lookup[i] = new instruction("BVS", this.BVS, this.REL, 2 ); ++i;
        this.lookup[i] = new instruction("ADC", this.ADC, this.IZY, 5 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 8 ); ++i;
        this.lookup[i] = new instruction("???", this.NOP, this.IMP, 4 ); ++i;
        this.lookup[i] = new instruction("ADC", this.ADC, this.ZPX, 4 ); ++i;
        this.lookup[i] = new instruction("ROR", this.ROR, this.ZPX, 6 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 6 ); ++i;
        this.lookup[i] = new instruction("SEI", this.SEI, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("ADC", this.ADC, this.ABY, 4 ); ++i;
        this.lookup[i] = new instruction("???", this.NOP, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 7 ); ++i;
        this.lookup[i] = new instruction("???", this.NOP, this.IMP, 4 ); ++i;
        this.lookup[i] = new instruction("ADC", this.ADC, this.ABX, 4 ); ++i;
        this.lookup[i] = new instruction("ROR", this.ROR, this.ABX, 7 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 7 ); ++i;
        this.lookup[i] = new instruction("???", this.NOP, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("STA", this.STA, this.IZX, 6 ); ++i;
        this.lookup[i] = new instruction("???", this.NOP, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 6 ); ++i;
        this.lookup[i] = new instruction("STY", this.STY, this.ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("STA", this.STA, this.ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("STX", this.STX, this.ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 3 ); ++i;
        this.lookup[i] = new instruction("DEY", this.DEY, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.NOP, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("TXA", this.TXA, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("STY", this.STY, this.ABS, 4 ); ++i;
        this.lookup[i] = new instruction("STA", this.STA, this.ABS, 4 ); ++i;
        this.lookup[i] = new instruction("STX", this.STX, this.ABS, 4 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 4 ); ++i;
        this.lookup[i] = new instruction("BCC", this.BCC, this.REL, 2 ); ++i;
        this.lookup[i] = new instruction("STA", this.STA, this.IZY, 6 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 6 ); ++i;
        this.lookup[i] = new instruction("STY", this.STY, this.ZPX, 4 ); ++i;
        this.lookup[i] = new instruction("STA", this.STA, this.ZPX, 4 ); ++i;
        this.lookup[i] = new instruction("STX", this.STX, this.ZPY, 4 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 4 ); ++i;
        this.lookup[i] = new instruction("TYA", this.TYA, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("STA", this.STA, this.ABY, 5 ); ++i;
        this.lookup[i] = new instruction("TXS", this.TXS, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 5 ); ++i;
        this.lookup[i] = new instruction("???", this.NOP, this.IMP, 5 ); ++i;
        this.lookup[i] = new instruction("STA", this.STA, this.ABX, 5 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 5 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 5 ); ++i;
        this.lookup[i] = new instruction("LDY", this.LDY, this.IMM, 2 ); ++i;
        this.lookup[i] = new instruction("LDA", this.LDA, this.IZX, 6 ); ++i;
        this.lookup[i] = new instruction("LDX", this.LDX, this.IMM, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 6 ); ++i;
        this.lookup[i] = new instruction("LDY", this.LDY, this.ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("LDA", this.LDA, this.ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("LDX", this.LDX, this.ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 3 ); ++i;
        this.lookup[i] = new instruction("TAY", this.TAY, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("LDA", this.LDA, this.IMM, 2 ); ++i;
        this.lookup[i] = new instruction("TAX", this.TAX, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("LDY", this.LDY, this.ABS, 4 ); ++i;
        this.lookup[i] = new instruction("LDA", this.LDA, this.ABS, 4 ); ++i;
        this.lookup[i] = new instruction("LDX", this.LDX, this.ABS, 4 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 4 ); ++i;
        this.lookup[i] = new instruction("BCS", this.BCS, this.REL, 2 ); ++i;
        this.lookup[i] = new instruction("LDA", this.LDA, this.IZY, 5 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 5 ); ++i;
        this.lookup[i] = new instruction("LDY", this.LDY, this.ZPX, 4 ); ++i;
        this.lookup[i] = new instruction("LDA", this.LDA, this.ZPX, 4 ); ++i;
        this.lookup[i] = new instruction("LDX", this.LDX, this.ZPY, 4 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 4 ); ++i;
        this.lookup[i] = new instruction("CLV", this.CLV, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("LDA", this.LDA, this.ABY, 4 ); ++i;
        this.lookup[i] = new instruction("TSX", this.TSX, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 4 ); ++i;
        this.lookup[i] = new instruction("LDY", this.LDY, this.ABX, 4 ); ++i;
        this.lookup[i] = new instruction("LDA", this.LDA, this.ABX, 4 ); ++i;
        this.lookup[i] = new instruction("LDX", this.LDX, this.ABY, 4 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 4 ); ++i;
        this.lookup[i] = new instruction("CPY", this.CPY, this.IMM, 2 ); ++i;
        this.lookup[i] = new instruction("CMP", this.CMP, this.IZX, 6 ); ++i;
        this.lookup[i] = new instruction("???", this.NOP, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 8 ); ++i;
        this.lookup[i] = new instruction("CPY", this.CPY, this.ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("CMP", this.CMP, this.ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("DEC", this.DEC, this.ZP0, 5 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 5 ); ++i;
        this.lookup[i] = new instruction("INY", this.INY, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("CMP", this.CMP, this.IMM, 2 ); ++i;
        this.lookup[i] = new instruction("DEX", this.DEX, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("CPY", this.CPY, this.ABS, 4 ); ++i;
        this.lookup[i] = new instruction("CMP", this.CMP, this.ABS, 4 ); ++i;
        this.lookup[i] = new instruction("DEC", this.DEC, this.ABS, 6 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 6 ); ++i;
        this.lookup[i] = new instruction("BNE", this.BNE, this.REL, 2 ); ++i;
        this.lookup[i] = new instruction("CMP", this.CMP, this.IZY, 5 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 8 ); ++i;
        this.lookup[i] = new instruction("???", this.NOP, this.IMP, 4 ); ++i;
        this.lookup[i] = new instruction("CMP", this.CMP, this.ZPX, 4 ); ++i;
        this.lookup[i] = new instruction("DEC", this.DEC, this.ZPX, 6 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 6 ); ++i;
        this.lookup[i] = new instruction("CLD", this.CLD, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("CMP", this.CMP, this.ABY, 4 ); ++i;
        this.lookup[i] = new instruction("NOP", this.NOP, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 7 ); ++i;
        this.lookup[i] = new instruction("???", this.NOP, this.IMP, 4 ); ++i;
        this.lookup[i] = new instruction("CMP", this.CMP, this.ABX, 4 ); ++i;
        this.lookup[i] = new instruction("DEC", this.DEC, this.ABX, 7 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 7 ); ++i;
        this.lookup[i] = new instruction("CPX", this.CPX, this.IMM, 2 ); ++i;
        this.lookup[i] = new instruction("SBC", this.SBC, this.IZX, 6 ); ++i;
        this.lookup[i] = new instruction("???", this.NOP, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 8 ); ++i;
        this.lookup[i] = new instruction("CPX", this.CPX, this.ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("SBC", this.SBC, this.ZP0, 3 ); ++i;
        this.lookup[i] = new instruction("INC", this.INC, this.ZP0, 5 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 5 ); ++i;
        this.lookup[i] = new instruction("INX", this.INX, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("SBC", this.SBC, this.IMM, 2 ); ++i;
        this.lookup[i] = new instruction("NOP", this.NOP, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.SBC, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("CPX", this.CPX, this.ABS, 4 ); ++i;
        this.lookup[i] = new instruction("SBC", this.SBC, this.ABS, 4 ); ++i;
        this.lookup[i] = new instruction("INC", this.INC, this.ABS, 6 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 6 ); ++i;
        this.lookup[i] = new instruction("BEQ", this.BEQ, this.REL, 2 ); ++i;
        this.lookup[i] = new instruction("SBC", this.SBC, this.IZY, 5 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 8 ); ++i;
        this.lookup[i] = new instruction("???", this.NOP, this.IMP, 4 ); ++i;
        this.lookup[i] = new instruction("SBC", this.SBC, this.ZPX, 4 ); ++i;
        this.lookup[i] = new instruction("INC", this.INC, this.ZPX, 6 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 6 ); ++i;
        this.lookup[i] = new instruction("SED", this.SED, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("SBC", this.SBC, this.ABY, 4 ); ++i;
        this.lookup[i] = new instruction("NOP", this.NOP, this.IMP, 2 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 7 ); ++i;
        this.lookup[i] = new instruction("???", this.NOP, this.IMP, 4 ); ++i;
        this.lookup[i] = new instruction("SBC", this.SBC, this.ABX, 4 ); ++i;
        this.lookup[i] = new instruction("INC", this.INC, this.ABX, 7 ); ++i;
        this.lookup[i] = new instruction("???", this.XXX, this.IMP, 7 );
    }
}