#include "mapper_001.h"
#include <stdlib.h>

mapper_001 mapper_001_init(u8 prg_banks, u8 chr_banks) {
    mapper_001 m = { 0 };

    m.prg_banks = prg_banks;
    m.chr_banks = chr_banks;

    m.ram_static = (u8*)malloc(32 * 1024);
    mapper_001_reset(&m);

    return m;
}

void mapper_001_destroy(mapper_001* m) {
    free(m->ram_static);
}

u8 mapper_001_cpu_map_read(mapper_001* m, u16 addr, u32* mapped_addr, u8* data) {
    if (addr >= 0x6000 && addr <= 0x7FFF) {
        *mapped_addr = 0xFFFFFFFF;
        *data = m->ram_static[addr & 0x1FFF];
        return true;
    }

    if (addr >= 0x8000) {
        if (m->control_register & 0b10000) {
            if (addr <= 0xBFFF) {
                *mapped_addr = m->prg_bank_select_16lo * 0x4000 + (addr & 0x3FFF);
                return true;
            } else {
                *mapped_addr = m->prg_bank_select_16hi * 0x4000 + (addr & 0x3FFF);
                return true;
            }
        } else {
            *mapped_addr = m->prg_bank_select_32 * 0x8000 + (addr & 0x7FFF);
            return true;
        }
    }

    return false;
}

u8 mapper_001_cpu_map_write(mapper_001* m, u16 addr, u32* mapped_addr, u8 data) {
    if (addr >= 0x6000 && addr <= 0x7FFF) {
        *mapped_addr = 0xFFFFFFFF;
        m->ram_static[addr & 0x1FFF] = data;
        return true;
    }

    if (addr >= 0x8000) {
        if (data & 0x80) {
            m->load_register = 0;
            m->load_register_count = 0;
            m->control_register |= 0x0C;
        } else {
            m->load_register >>= 1;
            m->load_register |= (data & 0x01) << 4;
            m->load_register_count++;

            if (m->load_register_count == 5) {
                u8 target_register = (addr >> 13) & 0x03;

                if (target_register == 0) {
                    m->control_register = m->load_register & 0x1F;

                    switch (m->control_register & 0x03) {
                        case 0: m->mirror_mode = ONESCREEN_LO; break;
                        case 1: m->mirror_mode = ONESCREEN_HI; break;
                        case 2: m->mirror_mode = VERTICAL;     break;
                        case 3: m->mirror_mode = HORIZONTAL;   break;
                    }

                } else if (target_register == 1) {
                    if (m->control_register & 0b10000)
                        m->chr_bank_select_4lo = m->load_register & 0x1F;
                    else
                        m->chr_bank_select_8 = m->load_register & 0x1E;

                } else if (target_register == 2) {
                    if (m->control_register & 0b10000)
                        m->chr_bank_select_4hi = m->load_register & 0x1F;

                } else if (target_register == 3) {
                    u8 mode = (m->control_register >> 2) & 0x03;
                    if (mode == 0 || mode == 1) {
                        m->prg_bank_select_32 = (m->load_register & 0x0E) >> 1;
                    } else if (mode == 2) {
                        m->prg_bank_select_16lo = 0;
                        m->prg_bank_select_16hi = m->load_register & 0x0F;
                    } else if (mode == 3) {
                        m->prg_bank_select_16lo = m->load_register & 0x0F;
                        m->prg_bank_select_16hi = m->prg_banks - 1;
                    }
                }

                m->load_register = 0;
                m->load_register_count = 0;
            }
        }

        return true;
    }

    return false;
}

u8 mapper_001_ppu_map_read(mapper_001* m, u16 addr, u32* mapped_addr) {
    if (addr < 0x2000) {
        if (m->chr_banks == 0) {
            *mapped_addr = addr;
            return true;
        }

        if (m->control_register & 0b10000) {
            if (addr < 0x1000)
                *mapped_addr = m->chr_bank_select_4lo * 0x1000 + (addr & 0x0FFF);
            else
                *mapped_addr = m->chr_bank_select_4hi * 0x1000 + (addr & 0x0FFF);
        } else {
            *mapped_addr = m->chr_bank_select_8 * 0x2000 + (addr & 0x1FFF);
        }

        return true;
    }

    return false;
}

u8 mapper_001_ppu_map_write(mapper_001* m, u16 addr, u32* mapped_addr) {
    if (addr < 0x2000) {
        if (m->chr_banks == 0) {
            *mapped_addr = addr;
            return true;
        }
        return false;
    }

    return false;
}

void mapper_001_reset(mapper_001* m) {
    m->control_register = 0x1C;
    m->load_register = 0;
    m->load_register_count = 0;

    m->chr_bank_select_4lo = 0;
    m->chr_bank_select_4hi = 0;
    m->chr_bank_select_8 = 0;

    m->prg_bank_select_32 = 0;
    m->prg_bank_select_16lo = 0;
    m->prg_bank_select_16hi = m->prg_banks - 1;

    m->mirror_mode = VERTICAL;
}

MIRROR mapper_001_mirror(mapper_001* m) {
    return m->mirror_mode;
}