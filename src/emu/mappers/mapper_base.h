#pragma once
#include <emu/common.h>
#include "mapper_common.h"

#include "mapper_000.h"
#include "mapper_001.h"
#include "mapper_002.h"
#include "mapper_003.h"
#include "mapper_004.h"
#include "mapper_066.h"

typedef struct {
    MAPPER_TYPE type;
    union {
        mapper_000 m_000;
        mapper_001 m_001;
        mapper_002 m_002;
        mapper_003 m_003;
        mapper_004 m_004;
        mapper_066 m_066;
    } mp;
} mapper;

mapper mapper_create(u8 prg_banks, u8 chr_banks, MAPPER_TYPE type);
void mapper_destroy(mapper* m);

u8 mapper_cpu_map_read(mapper* m, u16 addr, u32* mapped_addr, u8* data);
u8 mapper_cpu_map_write(mapper* m, u16 addr, u32* mapped_addr, u8 data);

u8 mapper_ppu_map_read(mapper* m, u16 addr, u32* mapped_addr);
u8 mapper_ppu_map_write(mapper* m, u16 addr, u32* mapped_addr);

void mapper_reset(mapper* m);

MIRROR mapper_mirror(mapper* m);

u8 mapper_irq_state(mapper* m);
void mapper_irq_clear(mapper* m);

void mapper_scanline(mapper* m);