#include "mapper_004.h"
#include <stdlib.h>

mapper_004 mapper_004_init(u8 prg_banks, u8 chr_banks) {
    mapper_004 m = { 0 };
    
    m.prg_banks = prg_banks;
    m.chr_banks = chr_banks;

    m.ram_static = (u8*)malloc(32 * 1024);

    return m;
}

void mapper_004_destroy(mapper_004* m) {
    free(m->ram_static);
}

u8 mapper_004_cpu_map_read(mapper_004* m, u16 addr, u32* mapped_addr, u8* data) {
    if (addr >= 0x6000 && addr <= 0x7FFF) {
		*mapped_addr = 0xFFFFFFFF;
		*data = m->ram_static[addr & 0x1FFF];

		return true;
	}

	if (addr >= 0x8000 && addr <= 0x9FFF) {
		*mapped_addr = m->prg_bank[0] + (addr & 0x1FFF);
		return true;
	}

	if (addr >= 0xA000 && addr <= 0xBFFF) {
		*mapped_addr = m->prg_bank[1] + (addr & 0x1FFF);
		return true;
	}

	if (addr >= 0xC000 && addr <= 0xDFFF) {
		*mapped_addr = m->prg_bank[2] + (addr & 0x1FFF);
		return true;
	}

	if (addr >= 0xE000 && addr <= 0xFFFF) {
		*mapped_addr = m->prg_bank[3] + (addr & 0x1FFF);
		return true;
	}

	return false;
}

u8 mapper_004_cpu_map_write(mapper_004* m, u16 addr, u32* mapped_addr, u8 data) {
    if (addr >= 0x6000 && addr <= 0x7FFF) {
		*mapped_addr = 0xFFFFFFFF;
		m->ram_static[addr & 0x1FFF] = data;

		return true;
	}

	if (addr >= 0x8000 && addr <= 0x9FFF) {
		if (!(addr & 0x0001)) {
			m->target_register = data & 0x07;
			m->prg_bank_mode = (data & 0x40);
			m->chr_inversion = (data & 0x80);
		}
		else
		{
			m->_register[m->target_register] = data;

			if (m->chr_inversion) {
				m->chr_bank[0] = m->_register[2] * 0x0400;
				m->chr_bank[1] = m->_register[3] * 0x0400;
				m->chr_bank[2] = m->_register[4] * 0x0400;
				m->chr_bank[3] = m->_register[5] * 0x0400;
				m->chr_bank[4] = (m->_register[0] & 0xFE) * 0x0400;
				m->chr_bank[5] = m->_register[0] * 0x0400 + 0x0400;
				m->chr_bank[6] = (m->_register[1] & 0xFE) * 0x0400;
				m->chr_bank[7] = m->_register[1] * 0x0400 + 0x0400;
			}
			else {
				m->chr_bank[0] = (m->_register[0] & 0xFE) * 0x0400;
				m->chr_bank[1] = m->_register[0] * 0x0400 + 0x0400;
				m->chr_bank[2] = (m->_register[1] & 0xFE) * 0x0400;
				m->chr_bank[3] = m->_register[1] * 0x0400 + 0x0400;
				m->chr_bank[4] = m->_register[2] * 0x0400;
				m->chr_bank[5] = m->_register[3] * 0x0400;
				m->chr_bank[6] = m->_register[4] * 0x0400;
				m->chr_bank[7] = m->_register[5] * 0x0400;
			}

			if (m->prg_bank_mode) {
				m->prg_bank[2] = (m->_register[6] & 0x3F) * 0x2000;
				m->prg_bank[0] = (m->prg_banks * 2 - 2) * 0x2000;
			}
			else {
				m->prg_bank[0] = (m->_register[6] & 0x3F) * 0x2000;
				m->prg_bank[2] = (m->prg_banks * 2 - 2) * 0x2000;
			}

			m->prg_bank[1] = (m->_register[7] & 0x3F) * 0x2000;
			m->prg_bank[3] = (m->prg_banks * 2 - 1) * 0x2000;

		}

		return false;
	}

	if (addr >= 0xA000 && addr <= 0xBFFF) {
		if (!(addr & 0x0001)) {
			if (data & 0x01)
				m->mirror_mode = HORIZONTAL;
			else
				m->mirror_mode = VERTICAL;
		}
		else {
			// PRG Ram Protect
			// TODO:
		}

		return false;
	}

	if (addr >= 0xC000 && addr <= 0xDFFF) {
		if (!(addr & 0x0001)) {
			m->irq_reload = data;
		}
		else {
			m->irq_counter = 0x0000;
		}
		return false;
	}

	if (addr >= 0xE000 && addr <= 0xFFFF) {
		if (!(addr & 0x0001)) {
			m->irq_enable = false;
			m->irq_active = false;
		}
		else {
			m->irq_enable = true;
		}
		return false;
	}

	return false;
}

u8 mapper_004_ppu_map_read(mapper_004* m, u16 addr, u32* mapped_addr) {
	if (addr >= 0x0000 && addr <= 0x03FF) {
		*mapped_addr = m->chr_bank[0] + (addr & 0x03FF);
		return true;
	}

	if (addr >= 0x0400 && addr <= 0x07FF) {
		*mapped_addr = m->chr_bank[1] + (addr & 0x03FF);
		return true;
	}

	if (addr >= 0x0800 && addr <= 0x0BFF) {
		*mapped_addr = m->chr_bank[2] + (addr & 0x03FF);
		return true;
	}

	if (addr >= 0x0C00 && addr <= 0x0FFF) {
		*mapped_addr = m->chr_bank[3] + (addr & 0x03FF);
		return true;
	}

	if (addr >= 0x1000 && addr <= 0x13FF) {
		*mapped_addr = m->chr_bank[4] + (addr & 0x03FF);
		return true;
	}

	if (addr >= 0x1400 && addr <= 0x17FF) {
		*mapped_addr = m->chr_bank[5] + (addr & 0x03FF);
		return true;
	}

	if (addr >= 0x1800 && addr <= 0x1BFF) {
		*mapped_addr = m->chr_bank[6] + (addr & 0x03FF);
		return true;
	}

	if (addr >= 0x1C00 && addr <= 0x1FFF) {
		*mapped_addr = m->chr_bank[7] + (addr & 0x03FF);
		return true;
	}

	return false;
}

void mapper_004_reset(mapper_004* m) {
	m->target_register = 0x00;
	m->prg_bank_mode = false;
	m->chr_inversion = false;
	m->mirror_mode = HORIZONTAL;

	m->irq_active = false;
	m->irq_enable = false;
	m->irq_update = false;
	m->irq_counter = 0x0000;
	m->irq_reload = 0x0000;

	for (i32 i = 0; i < 4; i++)	m->prg_bank[i] = 0;
	for (i32 i = 0; i < 8; i++) { m->chr_bank[i] = 0; m->_register[i] = 0; }

	m->prg_bank[0] = 0 * 0x2000;
	m->prg_bank[1] = 1 * 0x2000;
	m->prg_bank[2] = (m->prg_banks * 2 - 2) * 0x2000;
	m->prg_bank[3] = (m->prg_banks * 2 - 1) * 0x2000;
}

u8 mapper_004_irq_state(mapper_004* m) {
    return m->irq_active;
}

void mapper_004_irq_clear(mapper_004* m) {
    m->irq_active = false;
}

void mapper_004_scanline(mapper_004* m) {
    if (m->irq_counter == 0) {
        m->irq_counter = m->irq_reload;
    } else {
        --m->irq_counter;
    }

    if (m->irq_counter == 00 && m->irq_enable) {
        m->irq_active = true;
    }
}

MIRROR mapper_004_mirror(mapper_004* m) {
    return m->mirror_mode;
}