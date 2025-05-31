#pragma once

#include "common.h"

#include "mappers/mapper_base.h"

typedef struct {
    u8 image_valid;
    MIRROR hw_mirror;

    u8 mapper_id;
    u8 prg_banks;
    u8 chr_banks;

    u8* prg_memory;
    u8* chr_memory;

    u8 chr_is_rom;

    mapper m;
} cartridge;

cartridge cartridge_init(const char* filepath);
void cartridge_destroy(cartridge* cart);

u8 cartridge_image_valid(cartridge* cart);

u8 cartridge_cpu_read(cartridge* cart, u16 addr, u8* data);
u8 cartridge_cpu_write(cartridge* cart, u16 addr, u8 data);

u8 cartridge_ppu_read(cartridge* cart, u16 addr, u8* data);
u8 cartridge_ppu_write(cartridge* cart, u16 addr, u8 data);

void cartridge_reset(cartridge* cart);

MIRROR cartridge_mirror(cartridge* cart);

mapper* cartridge_mapper(cartridge* cart);