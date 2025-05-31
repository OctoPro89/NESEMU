#pragma once
#include <emu/common.h>
#include "mapper_common.h"

typedef struct {
    u8 prg_banks;
    u8 chr_banks;

    u8 chr_bank_select_4lo;
    u8 chr_bank_select_4hi;
    u8 chr_bank_select_8;

    u8 prg_bank_select_16lo;
    u8 prg_bank_select_16hi;
    u8 prg_bank_select_32;

    u8 load_register;
    u8 load_register_count;
    u8 control_register;

    MIRROR mirror_mode;

    u8* ram_static;
} mapper_001;

mapper_001 mapper_001_init(u8 prg_banks, u8 chr_banks);
void mapper_001_destroy(mapper_001* m);

u8 mapper_001_cpu_map_read(mapper_001* m, u16 addr, u32* mapped_addr, u8* data);
u8 mapper_001_cpu_map_write(mapper_001* m, u16 addr, u32* mapped_addr, u8 data);

u8 mapper_001_ppu_map_read(mapper_001* m, u16 addr, u32* mapped_addr);
u8 mapper_001_ppu_map_write(mapper_001* m, u16 addr, u32* mapped_addr);

void mapper_001_reset(mapper_001* m);

MIRROR mapper_001_mirror(mapper_001* m);