#pragma once
#include <emu/common.h>
#include "mapper_common.h"

typedef struct {
    u8 prg_banks;
    u8 chr_banks;

    u8 target_register;
    u8 prg_bank_mode;
    u8 chr_inversion;
    MIRROR mirror_mode;

    u32 _register[8];
    u32 chr_bank[8];
    u32 prg_bank[4];

    u8 irq_active;
    u8 irq_enable;
    u8 irq_update;

    u16 irq_counter;
    u16 irq_reload;

    u8* ram_static;
} mapper_004;

mapper_004 mapper_004_init(u8 prg_banks, u8 chr_banks);
void mapper_004_destroy(mapper_004* m);

u8 mapper_004_cpu_map_read(mapper_004* m, u16 addr, u32* mapped_addr, u8* data);
u8 mapper_004_cpu_map_write(mapper_004* m, u16 addr, u32* mapped_addr, u8 data);

u8 mapper_004_ppu_map_read(mapper_004* m, u16 addr, u32* mapped_addr);

void mapper_004_reset(mapper_004* m);

u8 mapper_004_irq_state(mapper_004* m);
void mapper_004_irq_clear(mapper_004* m);

void mapper_004_scanline(mapper_004* m);
MIRROR mapper_004_mirror(mapper_004* m);