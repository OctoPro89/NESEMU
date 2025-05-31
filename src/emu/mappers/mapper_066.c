#include "mapper_066.h"

mapper_066 mapper_066_init(u8 prg_banks, u8 chr_banks) {
    mapper_066 m = { 0 };

    m.prg_banks = prg_banks;
    m.chr_banks = chr_banks;
    m.prg_bank_select = 0;
    m.chr_bank_select = 0;

    return m;
}

u8 mapper_066_cpu_map_read(mapper_066* m, u16 addr, u32* mapped_addr, u8* data) {
    if (addr >= 0x8000 && addr <= 0xFFFF) {
        if (m->prg_bank_select < (m->prg_banks / 2)) {
            *mapped_addr = m->prg_bank_select * 0x8000 + (addr & 0x7FFF);
            return true;
        }
    }

    return false;
}

u8 mapper_066_cpu_map_write(mapper_066* m, u16 addr, u32* mapped_addr, u8 data) {
    if (addr >= 0x8000 && addr <= 0xFFFF) {
        m->prg_bank_select = (data >> 4) & 0x03;
        m->chr_bank_select = data & 0x03;

        // Bound-check
        m->prg_bank_select %= (m->prg_banks / 2);
        m->chr_bank_select %= m->chr_banks;

        return true;
    }

    return false;
}

u8 mapper_066_ppu_map_read(mapper_066* m, u16 addr, u32* mapped_addr) {
    if (addr < 0x2000) {
        *mapped_addr = m->chr_bank_select * 0x2000 + (addr & 0x1FFF);
        return true;
    }

    return false;
}

u8 mapper_066_ppu_map_write(mapper_066* m, u16 addr, u32* mapped_addr) {
    if (addr < 0x2000 && m->chr_banks == 0) {
        *mapped_addr = addr;
        return true;
    }

    return false;
}

void mapper_066_reset(mapper_066* m) {
    m->prg_bank_select = 0;
    m->chr_bank_select = 0;
}