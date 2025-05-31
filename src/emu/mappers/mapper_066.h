#pragma once
#include <emu/common.h>

typedef struct {
    u8 prg_banks;
    u8 chr_banks;
    u8 prg_bank_select;
    u8 chr_bank_select;
} mapper_066;

mapper_066 mapper_066_init(u8 prg_banks, u8 chr_banks);

u8 mapper_066_cpu_map_read(mapper_066* m, u16 addr, u32* mapped_addr, u8* data);
u8 mapper_066_cpu_map_write(mapper_066* m, u16 addr, u32* mapped_addr, u8 data);

u8 mapper_066_ppu_map_read(mapper_066* m, u16 addr, u32* mapped_addr);

void mapper_066_reset(mapper_066* m);