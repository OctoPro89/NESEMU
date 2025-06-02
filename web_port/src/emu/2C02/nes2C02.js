
void nes2C02_connect_cartridge(nes2C02* nes, cartridge* cart) {
    nes->cart = cart;
    // memcpy(nes->tbl_pattern[0], cart->chr_memory, 4096);
    // memcpy(nes->tbl_pattern[1], cart->chr_memory + 4096, 4096);
}

void nes2C02_reset(nes2C02* nes) {
    nes->fine_x = 0x00;
    nes->address_latch = 0x00;
    nes->ppu_data_buffer = 0x00;
    nes->scanline = 0;
    nes->cycle = 0;
    nes->bg_next_tile_id = 0x00;
    nes->bg_next_tile_attrib = 0x00;
    nes->bg_next_tile_lsb = 0x00;
    nes->bg_next_tile_msb = 0x00;
    nes->bg_shifter_pattern_lo = 0x0000;
    nes->bg_shifter_pattern_hi = 0x0000;
    nes->bg_shifter_attrib_lo = 0x0000;
    nes->bg_shifter_attrib_hi = 0x0000;
    nes->status.reg = 0x00;
    nes->mask.reg = 0x00;
    nes->control.reg = 0x00;
    nes->vram_addr.reg = 0x0000;
    nes->tram_addr.reg = 0x0000;
    nes->scanline_trigger = 0;
    nes->odd_frame = 0;
}

void IncrementScrollX(nes2C02* nes) {
    if (nes->mask.render_background || nes->mask.render_sprites) {
        if (nes->vram_addr.coarse_x == 31) {
            nes->vram_addr.coarse_x = 0;
            nes->vram_addr.nametable_x = ~nes->vram_addr.nametable_x; // ~nes->vram_addr.nametable_x & 1;
        } else {
            ++nes->vram_addr.coarse_x;
        }
    }
}

void IncrementScrollY(nes2C02* nes) {
    if (nes->mask.render_background || nes->mask.render_sprites) {
        if (nes->vram_addr.fine_y < 7) {
            ++nes->vram_addr.fine_y;
        } else {
            nes->vram_addr.fine_y = 0;
            if (nes->vram_addr.coarse_y == 29) {
                nes->vram_addr.coarse_y = 0;
                nes->vram_addr.nametable_y = ~nes->vram_addr.nametable_y; // ~nes->vram_addr.nametable_y & 1;
            } else if (nes->vram_addr.coarse_y == 31) {
                nes->vram_addr.coarse_y = 0;
            } else {
                ++nes->vram_addr.coarse_y;
            }
        }
    }
}

void TransferAddressX(nes2C02* nes) {
    if (nes->mask.render_background || nes->mask.render_sprites) {
        nes->vram_addr.nametable_x = nes->tram_addr.nametable_x;
        nes->vram_addr.coarse_x = nes->tram_addr.coarse_x;
    }
}

void TransferAddressY(nes2C02* nes) {
    if (nes->mask.render_background || nes->mask.render_sprites) {
        nes->vram_addr.fine_y = nes->tram_addr.fine_y;
        nes->vram_addr.nametable_y = nes->tram_addr.nametable_y;
        nes->vram_addr.coarse_y = nes->tram_addr.coarse_y;
    }
}

void LoadBackgroundShifters(nes2C02* nes) {
    nes->bg_shifter_pattern_lo = (nes->bg_shifter_pattern_lo & 0xFF00) | nes->bg_next_tile_lsb;
    nes->bg_shifter_pattern_hi = (nes->bg_shifter_pattern_hi & 0xFF00) | nes->bg_next_tile_msb;

    nes->bg_shifter_attrib_lo = (nes->bg_shifter_attrib_lo & 0xFF00) | ((nes->bg_next_tile_attrib & 0b01) ? 0xFF : 0x00);
    nes->bg_shifter_attrib_hi = (nes->bg_shifter_attrib_hi & 0xFF00) | ((nes->bg_next_tile_attrib & 0b10) ? 0xFF : 0x00);
}

void UpdateShifters(nes2C02* nes) {
    if (nes->mask.render_background) {
        nes->bg_shifter_pattern_lo <<= 1;
        nes->bg_shifter_pattern_hi <<= 1;
        nes->bg_shifter_attrib_lo <<= 1;
        nes->bg_shifter_attrib_hi <<= 1;
    }

    if (nes->mask.render_sprites && nes->cycle >= 1 && nes->cycle < 258) {
        for (i32 i = 0; i < nes->sprite_count; ++i) {
            if (nes->sprite_scanline[i].x > 0) {
                --nes->sprite_scanline[i].x;
            } else {
                nes->sprite_shifter_pattern_lo[i] <<= 1;
                nes->sprite_shifter_pattern_hi[i] <<= 1;
            }
        }
    }
}

u8 flipbyte(u8 b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

void nes2C02_clock(nes2C02* nes) {
    if (nes->scanline >= -1 && nes->scanline < 240) {
        // -- BACKGROUND RENDERING --
        if (nes->scanline == 0 && nes->cycle == 0 && nes->odd_frame && (nes->mask.render_background || nes->mask.render_sprites)) {
            nes->cycle = 1;
        }

        if (nes->scanline == -1 && nes->cycle == 1) {
            // pretty much the start of a new frame
            nes->status.vertical_blank = 0;
            nes->status.sprite_overflow = 0;
            nes->status.sprite_zero_hit = 0;

            for (i32 i = 0; i < 8; ++i) {
                nes->sprite_shifter_pattern_lo[i] = 0;
                nes->sprite_shifter_pattern_hi[i] = 0;
            }
        }

        if ((nes->cycle >= 2 && nes->cycle < 258) || (nes->cycle >= 321 && nes->cycle < 338)) {
            UpdateShifters(nes);

            switch ((nes->cycle - 1) % 8) {
                case 0:
                    LoadBackgroundShifters(nes);
                    
                    nes->bg_next_tile_id = nes2C02_ppu_read(nes, 0x2000 | (nes->vram_addr.reg & 0x0FFF), false);
                    break;
                case 2:
                    nes->bg_next_tile_attrib = nes2C02_ppu_read(nes, 0x23C0 | (nes->vram_addr.nametable_y << 11)
                        | (nes->vram_addr.nametable_x << 10)
                        | ((nes->vram_addr.coarse_y >> 2) << 3)
                        | (nes->vram_addr.coarse_x >> 2), false);

                    if (nes->vram_addr.coarse_y & 0x02) nes->bg_next_tile_attrib >>= 4;
                    if (nes->vram_addr.coarse_x & 0x02) nes->bg_next_tile_attrib >>= 2;
                    nes->bg_next_tile_attrib &= 0x03;
                    break;

                    // Compared to the last two, the next two are the easy ones... :P

                case 4:
                    nes->bg_next_tile_lsb = nes2C02_ppu_read(nes, (nes->control.pattern_background << 12)
                        + ((u16)nes->bg_next_tile_id << 4)
                        + (nes->vram_addr.fine_y), false);

                    break;
                case 6:
                    nes->bg_next_tile_msb = nes2C02_ppu_read(nes, (nes->control.pattern_background << 12)
                        + ((u16)nes->bg_next_tile_id << 4)
                        + (nes->vram_addr.fine_y) + 8, false);
                    break;
                case 7:
                    IncrementScrollX(nes);
                    break;
            }
        }


        // End of a visible scanline, increment downwards
        if (nes->cycle == 256) {
            IncrementScrollY(nes);
        }

        // reset the x to go back to the start
        if (nes->cycle == 257) {
            LoadBackgroundShifters(nes);
            TransferAddressX(nes);
        }

        // Superfluous reads of tile ID at the end of scanline
        if (nes->cycle == 338 || nes->cycle == 340) {
            nes->bg_next_tile_id = nes2C02_ppu_read(nes, 0x2000 | (nes->vram_addr.reg & 0x0FFF), false);
        }

        if (nes->scanline == -1 && nes->cycle >= 280 && nes->cycle < 305) {
            // End of vertical blank period so reset the y address
            TransferAddressY(nes);
        }

        // -- FOREGROUND RENDERING --
        // Using a trick here that performs sprite evaluation all in one hit, the real NES doesn't do it like this though
        if (nes->cycle == 257 && nes->scanline >= 0) {
            memset(nes->sprite_scanline, 0xFF, 8 * sizeof(struct object_attribute_entry));
            nes->sprite_count = 0;

            for (u8 i = 0; i < 8; ++i) {
                nes->sprite_shifter_pattern_lo[i] = 0;
                nes->sprite_shifter_pattern_hi[i] = 0;
            }

            u8 oam_entry = 0;
            nes->sprite_zero_hit_possible = false;

            while (oam_entry < 64 && nes->sprite_count < 9) {
                i16 diff = ((i16)nes->scanline - (i16)nes->oam[oam_entry].y);

                if (diff >= 0 && diff < (nes->control.sprite_size ? 16 : 8) && nes->sprite_count < 8) {
                    if (nes->sprite_count < 8) {
                        if (oam_entry == 0) {
                            nes->sprite_zero_hit_possible = true;
                        }

                        memcpy(&nes->sprite_scanline[nes->sprite_count], &nes->oam[oam_entry], sizeof(struct object_attribute_entry));
                    }

                    ++nes->sprite_count;
                }

                ++oam_entry;
            }

            // nes->status.sprite_overflow = (nes->sprite_count >= 8);
            nes->status.sprite_overflow = (nes->sprite_count > 8);
        }

        if (nes->cycle == 340) {
            // End of scanline
            for (u8 i = 0; i < nes->sprite_count; ++i) {
                // Extract 8 bit row patterns of the sprite with vertical offset
                u8 sprite_pattern_bits_lo, sprite_pattern_bits_hi;
                u16 sprite_pattern_addr_lo, sprite_pattern_addr_hi;

                if (!nes->control.sprite_size) {
                    // 8x8 sprite mode
                    if (!(nes->sprite_scanline[i].attribute & 0x80)) {
                        // sprite is not flipped vertically
                        sprite_pattern_addr_lo = (nes->control.pattern_sprite << 12) | (nes->sprite_scanline[i].id << 4) | (nes->scanline - nes->sprite_scanline[i].y);
                    } else {
                        // sprite is flipped vertically
                        sprite_pattern_addr_lo = (nes->control.pattern_sprite << 12) | (nes->sprite_scanline[i].id << 4) | (7 - (nes->scanline - nes->sprite_scanline[i].y));
                    }
                } else {
                    // 8x16 sprite mode
                    if (!(nes->sprite_scanline[i].attribute & 0x80)) {
                        // sprite is not flipped vertically
                        if (nes->scanline - nes->sprite_scanline[i].y < 8) {
                            // read top half tile
                            sprite_pattern_addr_lo = ((nes->sprite_scanline[i].id & 0x01) << 12) | ((nes->sprite_scanline[i].id & 0xFE) << 4) | ((nes->scanline - nes->sprite_scanline[i].y) & 0x07);
                        } else {
                            // read bottom half tile
                            sprite_pattern_addr_lo = ((nes->sprite_scanline[i].id & 0x01) << 12) | (((nes->sprite_scanline[i].id & 0xFE) + 1) << 4) | ((nes->scanline - nes->sprite_scanline[i].y) & 0x07);
                        }
                    } else {
                        // sprite is flipped vertically
                        if (nes->scanline - nes->sprite_scanline[i].y < 8) {
                            // read top half tile
                            sprite_pattern_addr_lo = ((nes->sprite_scanline[i].id & 0x01) << 12) | (((nes->sprite_scanline[i].id & 0xFE) + 1) << 4) | (7 - ((nes->scanline - nes->sprite_scanline[i].y) & 0x07));
                        } else {
                            // read bottom half tile
                            sprite_pattern_addr_lo = ((nes->sprite_scanline[i].id & 0x01) << 12) | ((nes->sprite_scanline[i].id & 0xFE) << 4) | (7 - ((nes->scanline - nes->sprite_scanline[i].y) & 0x07));
                        }
                    }
                }

                sprite_pattern_addr_hi = sprite_pattern_addr_lo + 8;

                sprite_pattern_bits_lo = nes2C02_ppu_read(nes, sprite_pattern_addr_lo, false);
                sprite_pattern_bits_hi = nes2C02_ppu_read(nes, sprite_pattern_addr_hi, false);

                if (nes->sprite_scanline[i].attribute & 0x40) {
                    sprite_pattern_bits_lo = flipbyte(sprite_pattern_bits_lo);
                    sprite_pattern_bits_hi = flipbyte(sprite_pattern_bits_hi);
                }

                nes->sprite_shifter_pattern_lo[i] = sprite_pattern_bits_lo;
                nes->sprite_shifter_pattern_hi[i] = sprite_pattern_bits_hi;
            }
        }
    }

    // if (nes->scanline == 240) { Post Render Scanline / do nothing }

    if (nes->scanline >= 241 && nes->scanline < 261) {
        if (nes->scanline == 241 && nes->cycle == 1) {
            nes->status.vertical_blank = 1;

            if  (nes->control.enable_nmi) {
                nes->nmi = true;
            }
        }
    }

    u8 bg_pixel = 0x00;
    u8 bg_palette = 0x00;

    if (nes->mask.render_background) {
        if (nes->mask.render_background_left || (nes->cycle >= 9)) {
            u16 bit_mux = 0x8000 >> nes->fine_x;

            u8 p0_pixel = (nes->bg_shifter_pattern_lo & bit_mux) > 0;
            u8 p1_pixel = (nes->bg_shifter_pattern_hi & bit_mux) > 0;

            bg_pixel = (p1_pixel << 1) | p0_pixel;

            u8 bg_pal0 = (nes->bg_shifter_attrib_lo & bit_mux) > 0;
            u8 bg_pal1 = (nes->bg_shifter_attrib_hi & bit_mux) > 0;
            bg_palette = (bg_pal1 << 1) | bg_pal0;
        }
    }

    u8 fg_pixel = 0x00;
    u8 fg_palette = 0x00;
    u8 fg_priority = 0x00;

    if (nes->mask.render_sprites) {
        if (nes->mask.render_sprites_left || (nes->cycle >= 9)) {
            nes->sprite_zero_being_rendered = false;

            for (u8 i = 0; i < nes->sprite_count; ++i) {
                if (nes->sprite_scanline[i].x == 0) {
                    u8 fg_pixel_lo = (nes->sprite_shifter_pattern_lo[i] & 0x80) > 0;
                    u8 fg_pixel_hi = (nes->sprite_shifter_pattern_hi[i] & 0x80) > 0;
                    fg_pixel = (fg_pixel_hi << 1) | fg_pixel_lo;

                    fg_palette = (nes->sprite_scanline[i].attribute & 0x03) + 0x04;
                    fg_priority = (nes->sprite_scanline[i].attribute & 0x20) == 0;

                    if (fg_pixel != 0) {
                        if (i == 0) {
                            nes->sprite_zero_being_rendered = true;
                        }

                        break;
                    }
                }
            }
        }
    }

    u8 pixel = 0x00;
    u8 palette = 0x00;

    if (bg_pixel == 0 && fg_pixel == 0) {
        pixel = 0x00;
        palette = 0x00;
    } else if (bg_pixel == 0 && fg_pixel > 0) {
        pixel = fg_pixel;
        palette = fg_palette;
    } else if (bg_pixel > 0 && fg_pixel == 0) {
        pixel = bg_pixel;
        palette = bg_palette;
    } else if (bg_pixel > 0 && fg_pixel > 0) {
        if (fg_priority) {
            pixel = fg_pixel;
            palette = fg_palette;
        } else {
            pixel = bg_pixel;
            palette = bg_palette;
        }

        if (nes->sprite_zero_hit_possible && nes->sprite_zero_being_rendered) {
            if (nes->mask.render_background & nes->mask.render_sprites) {
                if (!(nes->mask.render_background_left || nes->mask.render_sprites_left)) {
                    if (nes->cycle >= 9 && nes->cycle < 258) {
                        nes->status.sprite_zero_hit = 1;
                    }
                } else {
                    if (nes->cycle >= 1 && nes->cycle < 258) {
                        nes->status.sprite_zero_hit = 1;
                    }
                }
            }
        }
    }

    i16 x = nes->cycle - 1;
    i16 y = nes->scanline;
    const i16 width = 256;
    const i16 height = 240;
    if (x >= 0 && x < width && y >= 0 && y < height) {
        nes->screen[y * width + x] = nes2C02_get_color_from_palette_ram(nes, palette, pixel);
    }

    ++nes->cycle;
    if (nes->mask.render_background || nes->mask.render_sprites) {
        if (nes->cycle == 260 && nes->scanline < 240) {
            mapper_scanline(cartridge_mapper(nes->cart));
        }
    }

    if (nes->cycle >= 341) {
        nes->cycle = 0;
        ++nes->scanline;
        if (nes->scanline >= 261) {
            nes->scanline = -1;
            nes->frame_complete = true;
            nes->odd_frame = !nes->odd_frame;
        }
    }
}

pixel nes2C02_get_color_from_palette_ram(nes2C02* nes, u8 palette, u8 pixel) {
    return nes->pal_screen[nes2C02_ppu_read(nes, 0x3F00 + (palette << 2) + pixel, false) & 0x3F];
}

class nes2C02 {
    cpu_read(addr, read_only) {
        u8 data = 0x00;

        if (read_only) {
            switch (addr) {
                case 0x0000: {
                    data = nes->control.reg;
                    break;
                }
                case 0x0001: {
                    data = nes->mask.reg;
                    break;
                }
                case 0x0002: {
                    data = nes->status.reg;
                    break;
                }
                case 0x0003: {
                    break;
                }
                case 0x0004: {
                    break;
                }
                case 0x0005: {
                    break;
                }
                case 0x0006: {
                    break;
                }
                case 0x0007: {
                    break;
                }
            }
        } else {
            switch (addr) {
                case 0x0000: { break; }
                case 0x0001: { break; }
                case 0x0002: {
                    data = (nes->status.reg & 0xE0) | (nes->ppu_data_buffer & 0x1F);
                    nes->status.vertical_blank = 0;
                    nes->address_latch = 0;
                    break;
                }
                case 0x0003: { break; }
                case 0x0004: {
                    u8* p_oam = (u8*)nes->oam;
                    data = p_oam[nes->oam_addr];
                    break;
                }
                case 0x0005: { break; }
                case 0x0006: { break; }
                case 0x0007: {
                    data = nes->ppu_data_buffer;
                    nes->ppu_data_buffer = nes2C02_ppu_read(nes, nes->vram_addr.reg, false);
                    if (nes->vram_addr.reg >= 0x3F00) {
                        data = nes->ppu_data_buffer;
                    }

                    nes->vram_addr.reg += (nes->control.increment_mode ? 32 : 1);
                    break;
                }
            }
        }

        return data;
    }

    cpu_write(addr, data) {
        switch (addr & 0x0007) {
            case 0x0000: {
                nes->control.reg = data;
                nes->tram_addr.nametable_x = nes->control.nametable_x;
                nes->tram_addr.nametable_y = nes->control.nametable_y;
                break;
            }
            case 0x0001: {
                nes->mask.reg = data;
                break;
            }
            case 0x0002: {
                break;
            }
            case 0x0003: {
                nes->oam_addr = data;
                break;
            }
            case 0x0004: {
                u8* p_oam = (u8*)nes->oam;
                p_oam[nes->oam_addr] = data;
                break;
            }
            case 0x0005: {
                if (nes->address_latch == 0) {
                    nes->fine_x = data & 0x07;
                    nes->tram_addr.coarse_x = data >> 3;
                    nes->address_latch = 1;
                } else {
                    nes->tram_addr.fine_y = data & 0x07;
                    nes->tram_addr.coarse_y = data >> 3;
                    nes->address_latch = 0;
                }

                break;
            }
            case 0x0006: {
                if (nes->address_latch == 0) {
                    nes->tram_addr.reg = (u16)((data & 0x3F) << 8) | (nes->tram_addr.reg & 0x00FF);
                    nes->address_latch = 1;
                } else {
                    nes->tram_addr.reg = (nes->tram_addr.reg & 0xFF00) | data;
                    nes->vram_addr = nes->tram_addr;
                    nes->address_latch = 0;
                }

                break;
            }
            case 0x0007: {
                nes2C02_ppu_write(nes, nes->vram_addr.reg, data);
                nes->vram_addr.reg += (nes->control.increment_mode ? 32 : 1);
                break;
            }
        }
    }

    ppu_read(addr, read_only) {
        u8 data = 0x00;
        addr &= 0x3FFF;

        if (cartridge_ppu_read(nes->cart, addr, &data)) {
            // Cartridge
        }
        else if (addr >= 0x0000 && addr <= 0x1FFF && !nes->cart->chr_is_rom) {
            data = nes->tbl_pattern[(addr & 0x1000) >> 12][addr & 0x0FFF];
        }
        else if (addr >= 0x2000 && addr <= 0x3EFF) {
            addr &= 0x0FFF;
            if (cartridge_mirror(nes->cart) == VERTICAL) {
                if (addr <= 0x03FF) {
                    data = nes->tbl_name[0][addr & 0x03FF];
                }
                else if (addr <= 0x07FF) {
                    data = nes->tbl_name[1][addr & 0x03FF];
                }
                else if (addr <= 0x0BFF) {
                    data = nes->tbl_name[0][addr & 0x03FF];
                }
                else {
                    data = nes->tbl_name[1][addr & 0x03FF];
                }
            }
            else if (cartridge_mirror(nes->cart) == HORIZONTAL) {
                if (addr <= 0x03FF) {
                    data = nes->tbl_name[0][addr & 0x03FF];
                }
                else if (addr <= 0x07FF) {
                    data = nes->tbl_name[0][addr & 0x03FF];
                }
                else if (addr <= 0x0BFF) {
                    data = nes->tbl_name[1][addr & 0x03FF];
                }
                else {
                    data = nes->tbl_name[1][addr & 0x03FF];
                }
            }
        }
        else if (addr >= 0x3F00 && addr <= 0x3FFF) {
            addr &= 0x001F;
            if (addr == 0x0010) addr = 0x0000;
            if (addr == 0x0014) addr = 0x0004;
            if (addr == 0x0018) addr = 0x0008;
            if (addr == 0x001C) addr = 0x000C;
            data = nes->tbl_palette[addr] & (nes->mask.grayscale ? 0x30 : 0x3F);
        }

        return data;
    }


    void nes2C02_ppu_write(nes2C02* nes, u16 addr, u8 data) {
        addr &= 0x3FFF;

        if (cartridge_ppu_write(nes->cart, addr, data)) {
            // Handled by cartridge
        }
        else if (addr >= 0x0000 && addr <= 0x1FFF && !nes->cart->chr_is_rom) {
            nes->tbl_pattern[(addr & 0x1000) >> 12][addr & 0x0FFF] = data;
        }
        else if (addr >= 0x2000 && addr <= 0x3EFF) {
            addr &= 0x0FFF;
            if (cartridge_mirror(nes->cart) == VERTICAL) {
                if (addr <= 0x03FF) {
                    nes->tbl_name[0][addr & 0x03FF] = data;
                }
                else if (addr <= 0x07FF) {
                    nes->tbl_name[1][addr & 0x03FF] = data;
                }
                else if (addr <= 0x0BFF) {
                    nes->tbl_name[0][addr & 0x03FF] = data;
                }
                else {
                    nes->tbl_name[1][addr & 0x03FF] = data;
                }
            }
            else if (cartridge_mirror(nes->cart) == HORIZONTAL) {
                if (addr <= 0x03FF) {
                    nes->tbl_name[0][addr & 0x03FF] = data;
                }
                else if (addr <= 0x07FF) {
                    nes->tbl_name[0][addr & 0x03FF] = data;
                }
                else if (addr <= 0x0BFF) {
                    nes->tbl_name[1][addr & 0x03FF] = data;
                }
                else {
                    nes->tbl_name[1][addr & 0x03FF] = data;
                }
            }
        }
        else if (addr >= 0x3F00 && addr <= 0x3FFF) {
            addr &= 0x001F;
            if (addr == 0x0010) addr = 0x0000;
            if (addr == 0x0014) addr = 0x0004;
            if (addr == 0x0018) addr = 0x0008;
            if (addr == 0x001C) addr = 0x000C;
            nes->tbl_palette[addr] = data;
        }
    }

    constructor() {
        this.pal_screen = new Array(0x40);

        this.pal_screen[0x00] = { r: 84, g: 84, b: 84};
        this.pal_screen[0x01] = { r: 0, g: 30, b: 116};
        this.pal_screen[0x02] = { r: 8, g: 16, b: 144};
        this.pal_screen[0x03] = { r: 48, g: 0, b: 136};
        this.pal_screen[0x04] = { r: 68, g: 0, b: 100};
        this.pal_screen[0x05] = { r: 92, g: 0, b: 48};
        this.pal_screen[0x06] = { r: 84, g: 4, b: 0};
        this.pal_screen[0x07] = { r: 60, g: 24, b: 0};
        this.pal_screen[0x08] = { r: 32, g: 42, b: 0};
        this.pal_screen[0x09] = { r: 8, g: 58, b: 0};
        this.pal_screen[0x0A] = { r: 0, g: 64, b: 0};
        this.pal_screen[0x0B] = { r: 0, g: 60, b: 0};
        this.pal_screen[0x0C] = { r: 0, g: 50, b: 60};
        this.pal_screen[0x0D] = { r: 0, g: 0, b: 0};
        this.pal_screen[0x0E] = { r: 0, g: 0, b: 0};
        this.pal_screen[0x0F] = { r: 0, g: 0, b: 0};
        this.pal_screen[0x10] = { r: 152, g: 150, b: 152};
        this.pal_screen[0x11] = { r: 8, g: 76, b: 196};
        this.pal_screen[0x12] = { r: 48, g: 50, b: 236};
        this.pal_screen[0x13] = { r: 92, g: 30, b: 228};
        this.pal_screen[0x14] = { r: 136, g: 20, b: 176};
        this.pal_screen[0x15] = { r: 160, g: 20, b: 100};
        this.pal_screen[0x16] = { r: 152, g: 34, b: 32};
        this.pal_screen[0x17] = { r: 120, g: 60, b: 0};
        this.pal_screen[0x18] = { r: 84, g: 90, b: 0};
        this.pal_screen[0x19] = { r: 40, g: 114, b: 0};
        this.pal_screen[0x1A] = { r: 8, g: 124, b: 0};
        this.pal_screen[0x1B] = { r: 0, g: 118, b: 40};
        this.pal_screen[0x1C] = { r: 0, g: 102, b: 120};
        this.pal_screen[0x1D] = { r: 0, g: 0, b: 0};
        this.pal_screen[0x1E] = { r: 0, g: 0, b: 0};
        this.pal_screen[0x1F] = { r: 0, g: 0, b: 0};
        this.pal_screen[0x20] = { r: 236, g: 238, b: 236};
        this.pal_screen[0x21] = { r: 76, g: 154, b: 236};
        this.pal_screen[0x22] = { r: 120, g: 124, b: 236};
        this.pal_screen[0x23] = { r: 176, g: 98, b: 236};
        this.pal_screen[0x24] = { r: 228, g: 84, b: 236};
        this.pal_screen[0x25] = { r: 236, g: 88, b: 180};
        this.pal_screen[0x26] = { r: 236, g: 106, b: 100};
        this.pal_screen[0x27] = { r: 212, g: 136, b: 32};
        this.pal_screen[0x28] = { r: 160, g: 170, b: 0};
        this.pal_screen[0x29] = { r: 116, g: 196, b: 0};
        this.pal_screen[0x2A] = { r: 76, g: 208, b: 32};
        this.pal_screen[0x2B] = { r: 56, g: 204, b: 108};
        this.pal_screen[0x2C] = { r: 56, g: 180, b: 204};
        this.pal_screen[0x2D] = { r: 60, g: 60, b: 60};
        this.pal_screen[0x2E] = { r: 0, g: 0, b: 0};
        this.pal_screen[0x2F] = { r: 0, g: 0, b: 0};
        this.pal_screen[0x30] = { r: 236, g: 238, b: 236};
        this.pal_screen[0x31] = { r: 168, g: 204, b: 236};
        this.pal_screen[0x32] = { r: 188, g: 188, b: 236};
        this.pal_screen[0x33] = { r: 212, g: 178, b: 236};
        this.pal_screen[0x34] = { r: 236, g: 174, b: 236};
        this.pal_screen[0x35] = { r: 236, g: 174, b: 212};
        this.pal_screen[0x36] = { r: 236, g: 180, b: 176};
        this.pal_screen[0x37] = { r: 228, g: 196, b: 144};
        this.pal_screen[0x38] = { r: 204, g: 210, b: 120};
        this.pal_screen[0x39] = { r: 180, g: 222, b: 120};
        this.pal_screen[0x3A] = { r: 168, g: 226, b: 144};
        this.pal_screen[0x3B] = { r: 152, g: 226, b: 180};
        this.pal_screen[0x3C] = { r: 160, g: 214, b: 228};
        this.pal_screen[0x3D] = { r: 160, g: 162, b: 160};
        this.pal_screen[0x3E] = { r: 0, g: 0, b: 0};
        this.pal_screen[0x3F] = { r: 0, g: 0, b: 0};

        this.screen = new Array(256 * 240);

        this.tbl_name = new Array(2);
        this.tbl_name[0] = new Uint8Array(1024);
        this.tbl_name[1] = new Uint8Array(1024);

        this.tbl_pattern = new Array(2);
        this.tbl_pattern[0] = new Uint8Array(4096);
        this.tbl_pattern[1] = new Uint8Array(4096);
    }
}