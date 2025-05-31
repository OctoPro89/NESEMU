#pragma once
#include <emu/cartridge.h>

union loopy_register {
    struct {
        u16 coarse_x : 5;
        u16 coarse_y : 5;
        u16 nametable_x : 1;
        u16 nametable_y : 1;
        u16 fine_y : 3;
        u16 unused : 1;
    };

    u16 reg;
};

typedef struct {
    u8* tbl_name[2];
    u8* tbl_pattern[2];
    u8 tbl_palette[32];
    pixel* pal_screen;

    pixel* screen;
    pixel* pattern_table;

    union PPUSTATUS {
        struct {
            u8 unused : 5;
            u8 sprite_overflow : 1;
            u8 sprite_zero_hit : 1;
            u8 vertical_blank : 1;
        };

        u8 reg;
    } status;

    union PPUMASK {
        struct {
            u8 grayscale : 1;
            u8 render_background_left : 1;
            u8 render_sprites_left : 1;
            u8 render_background : 1;
            u8 render_sprites : 1;
            u8 enhance_red : 1;
            u8 enhance_green : 1;
            u8 enhance_blue : 1;
        };

        u8 reg;
    } mask;

    union PPUCTRL {
        struct {
            u8 nametable_x : 1;
            u8 nametable_y : 1;
            u8 increment_mode : 1;
            u8 pattern_sprite : 1;
            u8 pattern_background : 1;
            u8 sprite_size : 1;
            u8 slave_mode : 1; // Unused
            u8 enable_nmi : 1;
        };

        u8 reg;
    } control;

    union loopy_register vram_addr;
    union loopy_register tram_addr;

    u8 fine_x;

    u8 address_latch;
    u8 ppu_data_buffer;

    i16 scanline;
    i16 cycle;
    u8 odd_frame;

    u8 bg_next_tile_id;
    u8 bg_next_tile_attrib;
    u8 bg_next_tile_lsb;
    u8 bg_next_tile_msb;
    u16 bg_shifter_pattern_lo;
    u16 bg_shifter_pattern_hi;
    u16 bg_shifter_attrib_lo;
    u16 bg_shifter_attrib_hi;

    struct object_attribute_entry {
        u8 y;
        u8 id;
        u8 attribute;
        u8 x;
    } oam[64];

    u8 oam_addr;

    struct object_attribute_entry sprite_scanline[8];
    u8 sprite_count;
    u8 sprite_shifter_pattern_lo[8];
    u8 sprite_shifter_pattern_hi[8];

    u8 sprite_zero_hit_possible;
    u8 sprite_zero_being_rendered;

    cartridge* cart;

    u8 nmi;
    u8 scanline_trigger;

    u8 frame_complete;
} nes2C02;

nes2C02 nes2C02_init();
void nes2C02_destroy(nes2C02* nes);

pixel* nes2C02_get_pattern_table(nes2C02* nes, u8 i, u8 palette);

u8 nes2C02_cpu_read(nes2C02* nes, u16 addr, u8 read_only);
void nes2C02_cpu_write(nes2C02* nes, u16 addr, u8 data);

u8 nes2C02_ppu_read(nes2C02* nes, u16 addr, u8 read_only);
void nes2C02_ppu_write(nes2C02* nes, u16 addr, u8 data);

void nes2C02_connect_cartridge(nes2C02* nes, cartridge* cart);
void nes2C02_clock(nes2C02* nes);
void nes2C02_reset(nes2C02* nes);

pixel nes2C02_get_color_from_palette_ram(nes2C02* nes, u8 palette, u8 pixel);