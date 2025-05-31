#pragma once
#include <emu/common.h>

typedef struct {
    u8 prg_banks;
    u8 chr_banks;
    u8 prg_bank_select_lo;
    u8 prg_bank_select_hi;
} mapper_002;

mapper_002 mapper_002_init(u8 prg_banks, u8 chr_banks);

u8 mapper_002_cpu_map_read(mapper_002* m, u16 addr, u32* mapped_addr, u8* data);
u8 mapper_002_cpu_map_write(mapper_002* m, u16 addr, u32* mapped_addr, u8 data);

u8 mapper_002_ppu_map_read(mapper_002* m, u16 addr, u32* mapped_addr);
u8 mapper_002_ppu_map_write(mapper_002* m, u16 addr, u32* mapped_addr);

void mapper_002_reset(mapper_002* m);