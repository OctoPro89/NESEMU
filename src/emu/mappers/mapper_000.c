#include "mapper_000.h"

mapper_000 mapper_000_init(u8 prg_banks, u8 chr_banks) {
    mapper_000 m = { 0 };
    m.prg_banks = prg_banks;
    m.chr_banks = chr_banks;
    return m;
}

u8 mapper_000_cpu_map_read(mapper_000* m, u16 addr, u32* mapped_addr, u8* data) {
    if (addr >= 0x8000 && addr <= 0xFFFF) {
        // 16KB PRG ROM mirrored if only 1 bank
        *mapped_addr = addr & (m->prg_banks > 1 ? 0x7FFF : 0x3FFF);
        return true;
    }
    return false;
}

u8 mapper_000_cpu_map_write(mapper_000* m, u16 addr, u32* mapped_addr, u8 data) {
    if (addr >= 0x8000 && addr <= 0xFFFF) {
        *mapped_addr = addr & (m->prg_banks > 1 ? 0x7FFF : 0x3FFF);
        return true;
    }
    return false;
}

u8 mapper_000_ppu_map_read(mapper_000* m, u16 addr, u32* mapped_addr) {
    if (addr < 0x2000) {
        *mapped_addr = addr;
        return true;
    }
    return false;
}

u8 mapper_000_ppu_map_write(mapper_000* m, u16 addr, u32* mapped_addr) {
    // Only allow writes to CHR if using CHR RAM
    if (addr < 0x2000 && m->chr_banks == 0) {
        *mapped_addr = addr;
        return true;
    }
    return false;
}