#include "mapper_base.h"

#include "mapper_000.h"
#include "mapper_001.h"
#include "mapper_002.h"
#include "mapper_003.h"
#include "mapper_004.h"
#include "mapper_066.h"

mapper mapper_create(u8 prg_banks, u8 chr_banks, MAPPER_TYPE type) {
    mapper m = { 0 };
    m.type = type;
    
    switch (m.type) {
        case MAPPER_TYPE_000: {
            m.mp.m_000 = mapper_000_init(prg_banks, chr_banks);
            break;
        }
        case MAPPER_TYPE_001: {
            m.mp.m_001 = mapper_001_init(prg_banks, chr_banks);
            break;
        }
        case MAPPER_TYPE_002: {
            m.mp.m_002 = mapper_002_init(prg_banks, chr_banks);
            break;
        }
        case MAPPER_TYPE_003: {
            m.mp.m_003 = mapper_003_init(prg_banks, chr_banks);
            break;
        }
        case MAPPER_TYPE_004: {
            m.mp.m_004 = mapper_004_init(prg_banks, chr_banks);
            break;
        }
        case MAPPER_TYPE_066: {
            m.mp.m_066 = mapper_066_init(prg_banks, chr_banks);
            break;
        }
    }

    return m;
}

void mapper_destroy(mapper* m) {
   switch (m->type) {
        case MAPPER_TYPE_000: {
            break;
        }
        case MAPPER_TYPE_001: {
            mapper_001_destroy(&m->mp.m_001);
            break;
        }
        case MAPPER_TYPE_002: {
            break;
        }
        case MAPPER_TYPE_003: {
            break;
        }
        case MAPPER_TYPE_004: {
            mapper_004_destroy(&m->mp.m_004);
            break;
        }
        case MAPPER_TYPE_066: {
            break;
        }
    }
}

u8 mapper_cpu_map_read(mapper* m, u16 addr, u32* mapped_addr, u8* data) {
    switch (m->type) {
        case MAPPER_TYPE_000: {
            return mapper_000_cpu_map_read(&m->mp.m_000, addr, mapped_addr, data);
        }
        case MAPPER_TYPE_001: {
            return mapper_001_cpu_map_read(&m->mp.m_001, addr, mapped_addr, data);
        }
        case MAPPER_TYPE_002: {
            return mapper_002_cpu_map_read(&m->mp.m_002, addr, mapped_addr, data);
        }
        case MAPPER_TYPE_003: {
            return mapper_003_cpu_map_read(&m->mp.m_003, addr, mapped_addr, data);
        }
        case MAPPER_TYPE_004: {
            return mapper_004_cpu_map_read(&m->mp.m_004, addr, mapped_addr, data);
        }
        case MAPPER_TYPE_066: {
            return mapper_066_cpu_map_read(&m->mp.m_066, addr, mapped_addr, data);
        }
    }

    return false;
}

u8 mapper_cpu_map_write(mapper* m, u16 addr, u32* mapped_addr, u8 data) {
    switch (m->type) {
        case MAPPER_TYPE_000: {
            return mapper_000_cpu_map_write(&m->mp.m_000, addr, mapped_addr, data);
        }
        case MAPPER_TYPE_001: {
            return mapper_001_cpu_map_write(&m->mp.m_001, addr, mapped_addr, data);
        }
        case MAPPER_TYPE_002: {
            return mapper_002_cpu_map_write(&m->mp.m_002, addr, mapped_addr, data);
        }
        case MAPPER_TYPE_003: {
            return mapper_003_cpu_map_write(&m->mp.m_003, addr, mapped_addr, data);
        }
        case MAPPER_TYPE_004: {
            return mapper_004_cpu_map_write(&m->mp.m_004, addr, mapped_addr, data);
        }
        case MAPPER_TYPE_066: {
            return mapper_066_cpu_map_write(&m->mp.m_066, addr, mapped_addr, data);
        }
    }

    return false;
}

u8 mapper_ppu_map_read(mapper* m, u16 addr, u32* mapped_addr) {
    switch (m->type) {
        case MAPPER_TYPE_000: {
            return mapper_000_ppu_map_read(&m->mp.m_000, addr, mapped_addr);
        }
        case MAPPER_TYPE_001: {
            return mapper_001_ppu_map_read(&m->mp.m_001, addr, mapped_addr);
        }
        case MAPPER_TYPE_002: {
            return mapper_002_ppu_map_read(&m->mp.m_002, addr, mapped_addr);
        }
        case MAPPER_TYPE_003: {
            return mapper_003_ppu_map_read(&m->mp.m_003, addr, mapped_addr);
        }
        case MAPPER_TYPE_004: {
            return mapper_004_ppu_map_read(&m->mp.m_004, addr, mapped_addr);
        }
        case MAPPER_TYPE_066: {
            return mapper_066_ppu_map_read(&m->mp.m_066, addr, mapped_addr);
        }
    }

    return false;
}

u8 mapper_ppu_map_write(mapper* m, u16 addr, u32* mapped_addr) {
    switch (m->type) {
        case MAPPER_TYPE_000: {
            return mapper_000_ppu_map_write(&m->mp.m_000, addr, mapped_addr);
        }
        case MAPPER_TYPE_001: {
            return mapper_001_ppu_map_write(&m->mp.m_001, addr, mapped_addr);
        }
        case MAPPER_TYPE_002: {
            return mapper_002_ppu_map_write(&m->mp.m_002, addr, mapped_addr);
        }
        case MAPPER_TYPE_003: {
            return false;
        }
        case MAPPER_TYPE_004: {
            return false;
        }
        case MAPPER_TYPE_066: {
            return false;
        }
    }

    return false;
}

void mapper_reset(mapper* m) {
    switch (m->type) {
        case MAPPER_TYPE_000: {
            break;
        }
        case MAPPER_TYPE_001: {
            mapper_001_reset(&m->mp.m_001);
            break;
        }
        case MAPPER_TYPE_002: {
            mapper_002_reset(&m->mp.m_002);
            break;
        }
        case MAPPER_TYPE_003: {
            mapper_003_reset(&m->mp.m_003);
            break;
        }
        case MAPPER_TYPE_004: {
            mapper_004_reset(&m->mp.m_004);
            break;
        }
        case MAPPER_TYPE_066: {
            mapper_066_reset(&m->mp.m_066);
            break;
        }
    }
}

MIRROR mapper_mirror(mapper* m) {
    switch (m->type) {
        case MAPPER_TYPE_000: {
            return HARDWARE;
        }
        case MAPPER_TYPE_001: {
            return mapper_001_mirror(&m->mp.m_001);
        }
        case MAPPER_TYPE_002: {
            return HARDWARE;
        }
        case MAPPER_TYPE_003: {
            return HARDWARE;
        }
        case MAPPER_TYPE_004: {
            return mapper_004_mirror(&m->mp.m_004);            
        }
        case MAPPER_TYPE_066: {
            return HARDWARE;
        }
    }

    return HARDWARE;
}

u8 mapper_irq_state(mapper* m) {
    switch (m->type) {
        case MAPPER_TYPE_000: {
            return false;
        }
        case MAPPER_TYPE_001: {
            return false;
        }
        case MAPPER_TYPE_002: {
            return false;
        }
        case MAPPER_TYPE_003: {
            return false;
        }
        case MAPPER_TYPE_004: {
            return mapper_004_irq_state(&m->mp.m_004);
        }
        case MAPPER_TYPE_066: {
            return false;
        }
    }

    return false;
}

void mapper_irq_clear(mapper* m) {
    switch (m->type) {
        case MAPPER_TYPE_000: {
            break;
        }
        case MAPPER_TYPE_001: {
            break;
        }
        case MAPPER_TYPE_002: {
            break;
        }
        case MAPPER_TYPE_003: {
            break;
        }
        case MAPPER_TYPE_004: {
            mapper_004_irq_clear(&m->mp.m_004);
            break;
        }
        case MAPPER_TYPE_066: {
            break;
        }
    }
}

void mapper_scanline(mapper* m) {
    switch (m->type) {
        case MAPPER_TYPE_000: {
            break;
        }
        case MAPPER_TYPE_001: {
            break;
        }
        case MAPPER_TYPE_002: {
            
            break;
        }
        case MAPPER_TYPE_003: {
            
            break;
        }
        case MAPPER_TYPE_004: {
            mapper_004_scanline(&m->mp.m_004);
            break;
        }
        case MAPPER_TYPE_066: {
            break;
        }
    }
}