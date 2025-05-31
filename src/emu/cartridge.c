#include "cartridge.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

typedef struct {
    char name[4];
    u8 prg_rom_chunks;
    u8 chr_rom_chunks;
    u8 mapper1;
    u8 mapper2;
    u8 prg_ram_size;
    u8 tv_system1;
    u8 tv_system2;
    char unused[5];
} iNESHEADER;

cartridge cartridge_init(const char* filepath) {
    cartridge cart = { 0 };

    FILE* f = fopen(filepath, "rb");
    if (!f) {
        printf("Failed to open file: %s\n", filepath);
        exit(-1);
    }

    iNESHEADER header = { 0 };
    fread(&header, sizeof(header), 1, f);

    if (header.mapper1 & 0x04) {
        fseek(f, 512, SEEK_CUR);
    }

    cart.mapper_id = ((header.mapper2 >> 4) << 4) | (header.mapper1 >> 4);
    cart.hw_mirror = (header.mapper1 & 0x01) ? VERTICAL : HORIZONTAL;
    
    u8 file_type = 1;
    if ((header.mapper2 & 0x0C) == 0x08) {
        file_type = 2;
    }

    if (file_type == 1) {
        cart.prg_banks = header.prg_rom_chunks;
        u32 prg_mem_size = cart.prg_banks * 16384;
        cart.prg_memory = (u8*)malloc(prg_mem_size);
        fread(cart.prg_memory, prg_mem_size, 1, f);

        cart.chr_banks = header.chr_rom_chunks;

        u32 chr_mem_size = 0;
        if (cart.chr_banks == 0) {
            chr_mem_size = 8192;
            cart.chr_memory = (u8*)malloc(chr_mem_size);
            memset(cart.chr_memory, 0, chr_mem_size);
        } else {
            chr_mem_size = cart.chr_banks * 8192;
            cart.chr_memory = (u8*)malloc(chr_mem_size);
            fread(cart.chr_memory, chr_mem_size, 1, f);
        }

    } else if (file_type == 2) {
        cart.prg_banks = ((header.prg_ram_size & 0x07) << 8) | header.prg_rom_chunks;

        u32 prg_mem_size = cart.prg_banks * 16384;
        cart.prg_memory = (u8*)malloc(prg_mem_size);
        fread(cart.prg_memory, prg_mem_size, 1, f);

        u32 chr_mem_size = cart.chr_banks * 8192;
        cart.chr_banks = ((header.prg_ram_size & 0x38) << 8) | header.chr_rom_chunks;
        cart.chr_memory = (u8*)malloc(chr_mem_size);
        fread(cart.chr_memory, chr_mem_size, 1, f);
    }

    switch (cart.mapper_id) {
        case 0: {
            cart.m = mapper_create(cart.prg_banks, cart.chr_banks, MAPPER_TYPE_000);
            break;
        }
        case 1: {
            cart.m = mapper_create(cart.prg_banks, cart.chr_banks, MAPPER_TYPE_001);
            break;
        }
        case 2: {
            cart.m = mapper_create(cart.prg_banks, cart.chr_banks, MAPPER_TYPE_002);
            break;
        }
        case 3: {
            cart.m = mapper_create(cart.prg_banks, cart.chr_banks, MAPPER_TYPE_003);
            break;
        }
        case 4: {
            cart.m = mapper_create(cart.prg_banks, cart.chr_banks, MAPPER_TYPE_004);
            break;
        }
        case 66: {
            cart.m = mapper_create(cart.prg_banks, cart.chr_banks, MAPPER_TYPE_066);
            break;
        }
        default: {
            printf("Found unknown mapper! Continuing play but certain features may not work right.\n");
            break;
        }
    }

    cart.image_valid = true;
    fclose(f);

    printf("[MAPPER] Mapper ID: %d\n", cart.mapper_id);
    printf("[MAPPER] PRG ROM Banks: %d\n", cart.prg_banks);
    printf("[MAPPER] CHR ROM Banks: %d\n", cart.chr_banks);
    printf("[MAPPER] CHR is %s\n", cart.chr_banks == 0 ? "RAM" : "ROM");

    if (cart.chr_banks == 0) {
        cart.chr_is_rom = false;
        printf("[INFO] CHR RAM allocated (8 KB)\n");
    } else {
        cart.chr_is_rom = true;
        printf("[INFO] CHR ROM size: %d KB\n", cart.chr_banks * 8);
    }

    if (cart.mapper_id == 0) {
        printf("[M0] Mapping PRG ROM...\n");
        printf(" - PRG $8000-$BFFF = bank 0\n");
        printf(" - PRG $C000-$FFFF = bank %d (last bank)\n", cart.prg_banks - 1);
    }

    return cart;
}

void cartridge_destroy(cartridge* cart) {
    free(cart->chr_memory);
    free(cart->prg_memory);
}

u8 cartridge_image_valid(cartridge* cart) {
    return cart->image_valid;
}

u8 cartridge_cpu_read(cartridge* cart, u16 addr, u8* data) {
    u32 mapped_addr = 0;
    if (mapper_cpu_map_read(&cart->m, addr, &mapped_addr, data)) {
        if (mapped_addr == 0xFFFFFFFF) {
            return true;
        } else {
            *data = cart->prg_memory[mapped_addr];
        }

        return true;
    } else {
        return false;
    }
}

u8 cartridge_cpu_write(cartridge* cart, u16 addr, u8 data) {
    u32 mapped_addr = 0;
    if (mapper_cpu_map_write(&cart->m, addr, &mapped_addr, data)) {
        if (mapped_addr == 0xFFFFFFFF) {
            return true;
        } else {
            cart->prg_memory[mapped_addr] = data;
        }

        return true;
    } else {
        return false;
    }
}

u8 cartridge_ppu_read(cartridge* cart, u16 addr, u8* data) {
    u32 mapped_addr = 0;
    if (mapper_ppu_map_read(&cart->m, addr, &mapped_addr)) {
        if (mapped_addr < cart->chr_banks * 8192 || !cart->chr_is_rom) {
            *data = cart->chr_memory[mapped_addr];
            return true;
        }
        return true;
    } else {
        return false;
    }
}

u8 cartridge_ppu_write(cartridge* cart, u16 addr, u8 data) {
    u32 mapped_addr = 0;
    if (mapper_ppu_map_write(&cart->m, addr, &mapped_addr)) {
        cart->chr_memory[mapped_addr] = data;
        return true;
    } else {
        return false;
    }
}

void cartridge_reset(cartridge* cart) {
    mapper_reset(&cart->m);
}

MIRROR cartridge_mirror(cartridge* cart) {
    MIRROR m = mapper_mirror(&cart->m);
    if (m == HARDWARE) {
        return cart->hw_mirror;
    } else {
        return m;
    }
}

mapper* cartridge_mapper(cartridge* cart) {
    return &cart->m;
}