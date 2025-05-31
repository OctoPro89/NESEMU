#include "mapper_003.h"

mapper_003 mapper_003_init(u8 prg_banks, u8 chr_banks) {
    mapper_003 m = { 0 };

    m.prg_banks = prg_banks;
    m.chr_banks = chr_banks;

    return m;
}

u8 mapper_003_cpu_map_read(mapper_003* m, u16 addr, u32* mapped_addr, u8* data) {
    if (addr >= 0x8000 && addr <= 0xFFFF) {
		if (m->prg_banks == 1) { *mapped_addr = addr & 0x3FFF; }
		if (m->prg_banks == 2) { *mapped_addr = addr & 0x7FFF; }

		return true;
	}

    return false;
}

u8 mapper_003_cpu_map_write(mapper_003* m, u16 addr, u32* mapped_addr, u8 data) {
	if (addr >= 0x8000 && addr <= 0xFFFF) {
		m->chr_bank_select = data & 0x03;
		*mapped_addr = addr;		
	}

    return false;
}

u8 mapper_003_ppu_map_read(mapper_003* m, u16 addr, u32* mapped_addr) {
    if (addr < 0x2000) {
        *mapped_addr = m->chr_bank_select * 0x2000 + addr;
        return true;
    }

    return false;
}

void mapper_003_reset(mapper_003* m) {
    m->chr_bank_select = 0;
}