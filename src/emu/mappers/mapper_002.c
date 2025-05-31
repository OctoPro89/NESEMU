#include "mapper_002.h"

mapper_002 mapper_002_init(u8 prg_banks, u8 chr_banks) {
    mapper_002 m = { 0 };

    m.prg_banks = prg_banks;
    m.chr_banks = chr_banks;

    return m;
}

u8 mapper_002_cpu_map_read(mapper_002* m, u16 addr, u32* mapped_addr, u8* data) {
	if (addr >= 0x8000 && addr <= 0xBFFF) {
		*mapped_addr = m->prg_bank_select_lo * 0x4000 + (addr & 0x3FFF);
		return true;
	}

	if (addr >= 0xC000 && addr <= 0xFFFF) {
		*mapped_addr = m->prg_bank_select_hi * 0x4000 + (addr & 0x3FFF);
		return true;
	}
	
	return false;
}

u8 mapper_002_cpu_map_write(mapper_002* m, u16 addr, u32* mapped_addr, u8 data) {
	if (addr >= 0x8000 && addr <= 0xFFFF) {		
		m->prg_bank_select_lo = data & 0x0F;
	}

	return false;
}

u8 mapper_002_ppu_map_read(mapper_002* m, u16 addr, u32* mapped_addr) {
    if (addr < 0x2000) {
        *mapped_addr = addr;
        return true;
    }

    return false;
}

u8 mapper_002_ppu_map_write(mapper_002* m, u16 addr, u32* mapped_addr) {
    if (addr < 0x2000) {
        if (m->chr_banks == 0) {
            *mapped_addr = addr;
            return true;
        }
    }

    return false;
}

void mapper_002_reset(mapper_002* m) {
    m->prg_bank_select_lo = 0;
    m->prg_bank_select_hi = m->prg_banks - 1;
}