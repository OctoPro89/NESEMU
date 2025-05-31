#pragma once
#include <emu/common.h>

typedef struct {
    u8 prg_banks;
    u8 chr_banks;
} mapper_000;

mapper_000 mapper_000_init(u8 prg_banks, u8 chr_banks);

u8 mapper_000_cpu_map_read(mapper_000* m, u16 addr, u32* mapped_addr, u8* data);
u8 mapper_000_cpu_map_write(mapper_000* m, u16 addr, u32* mapped_addr, u8 data);

u8 mapper_000_ppu_map_read(mapper_000* m, u16 addr, u32* mapped_addr);
u8 mapper_000_ppu_map_write(mapper_000* m, u16 addr, u32* mapped_addr);