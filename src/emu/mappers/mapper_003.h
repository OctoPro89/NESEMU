#pragma once
#include <emu/common.h>

typedef struct {
    u8 prg_banks;
    u8 chr_banks;
    u8 chr_bank_select;
} mapper_003;

mapper_003 mapper_003_init(u8 prg_banks, u8 chr_banks);

u8 mapper_003_cpu_map_read(mapper_003* m, u16 addr, u32* mapped_addr, u8* data);
u8 mapper_003_cpu_map_write(mapper_003* m, u16 addr, u32* mapped_addr, u8 data);

u8 mapper_003_ppu_map_read(mapper_003* m, u16 addr, u32* mapped_addr);

void mapper_003_reset(mapper_003* m);