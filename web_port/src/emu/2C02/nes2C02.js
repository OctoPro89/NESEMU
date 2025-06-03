function flipbyte(b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

class nes2C02 {
    connect_cartridge(cart) {
        this.cart = cart;
    }

    get_color_from_palette_ram(palette, pixel) {
        return pal_screen[nes2C02_ppu_read(nes, 0x3F00 + (palette << 2) + pixel, false) & 0x3F];
    }

    reset() {
        this.fine_x = 0x00;
        address_latch = 0x00;
        ppu_data_buffer = 0x00;
        scanline = 0;
        cycle = 0;
        bg_next_tile_id = 0x00;
        bg_next_tile_attrib = 0x00;
        bg_next_tile_lsb = 0x00;
        bg_next_tile_msb = 0x00;
        bg_shifter_pattern_lo = 0x0000;
        bg_shifter_pattern_hi = 0x0000;
        bg_shifter_attrib_lo = 0x0000;
        bg_shifter_attrib_hi = 0x0000;
        this.status.reg = 0x00;
        mask.reg = 0x00;
        control.reg = 0x00;
        vram_addr.reg = 0x0000;
        tram_addr.reg = 0x0000;
        scanline_trigger = 0;
        odd_frame = 0;
    }

    cpu_read(addr, read_only) {
        let data = 0x00;

        if (read_only) {
            switch (addr) {
                case 0x0000: {
                    data = control.reg;
                    break;
                }
                case 0x0001: {
                    data = mask.reg;
                    break;
                }
                case 0x0002: {
                    data = status.reg;
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
                    data = (status.reg & 0xE0) | (ppu_data_buffer & 0x1F);
                    this.status.vertical_blank = 0;
                    address_latch = 0;
                    break;
                }
                case 0x0003: { break; }
                case 0x0004: {
                    let* p_oam = (let*)oam;
                    data = p_oam[oam_addr];
                    break;
                }
                case 0x0005: { break; }
                case 0x0006: { break; }
                case 0x0007: {
                    data = ppu_data_buffer;
                    ppu_data_buffer = nes2C02_ppu_read(nes, vram_addr.reg, false);
                    if (vram_addr.reg >= 0x3F00) {
                        data = ppu_data_buffer;
                    }

                    vram_addr.reg += (control.increment_mode ? 32 : 1);
                    break;
                }
            }
        }

        return data;
    }

    cpu_write(addr, data) {
        switch (addr & 0x0007) {
            case 0x0000: {
                control.reg = data;
                tram_addr.nametable_x = control.nametable_x;
                tram_addr.nametable_y = control.nametable_y;
                break;
            }
            case 0x0001: {
                mask.reg = data;
                break;
            }
            case 0x0002: {
                break;
            }
            case 0x0003: {
                oam_addr = data;
                break;
            }
            case 0x0004: {
                let* p_oam = (let*)oam;
                p_oam[oam_addr] = data;
                break;
            }
            case 0x0005: {
                if (address_latch == 0) {
                    this.fine_x = data & 0x07;
                    tram_addr.coarse_x = data >> 3;
                    address_latch = 1;
                } else {
                    tram_addr.fine_y = data & 0x07;
                    tram_addr.coarse_y = data >> 3;
                    address_latch = 0;
                }

                break;
            }
            case 0x0006: {
                if (address_latch == 0) {
                    tram_addr.reg = ((data & 0x3F) << 8) | (tram_addr.reg & 0x00FF);
                    address_latch = 1;
                } else {
                    tram_addr.reg = (tram_addr.reg & 0xFF00) | data;
                    vram_addr = tram_addr;
                    address_latch = 0;
                }

                break;
            }
            case 0x0007: {
                nes2C02_ppu_write(nes, vram_addr.reg, data);
                vram_addr.reg += (control.increment_mode ? 32 : 1);
                break;
            }
        }
    }

    ppu_read(addr, read_only) {
        let data = 0x00;
        addr &= 0x3FFF;

        if (this.cart.ppu_read(cart, addr)) {
            // Cartridge
        }
        else if (addr >= 0x0000 && addr <= 0x1FFF && !cart->chr_is_rom) {
            data = tbl_pattern[(addr & 0x1000) >> 12][addr & 0x0FFF];
        }
        else if (addr >= 0x2000 && addr <= 0x3EFF) {
            addr &= 0x0FFF;
            if (cartridge_mirror(cart) == VERTICAL) {
                if (addr <= 0x03FF) {
                    data = tbl_name[0][addr & 0x03FF];
                }
                else if (addr <= 0x07FF) {
                    data = tbl_name[1][addr & 0x03FF];
                }
                else if (addr <= 0x0BFF) {
                    data = tbl_name[0][addr & 0x03FF];
                }
                else {
                    data = tbl_name[1][addr & 0x03FF];
                }
            }
            else if (cartridge_mirror(cart) == HORIZONTAL) {
                if (addr <= 0x03FF) {
                    data = tbl_name[0][addr & 0x03FF];
                }
                else if (addr <= 0x07FF) {
                    data = tbl_name[0][addr & 0x03FF];
                }
                else if (addr <= 0x0BFF) {
                    data = tbl_name[1][addr & 0x03FF];
                }
                else {
                    data = tbl_name[1][addr & 0x03FF];
                }
            }
        }
        else if (addr >= 0x3F00 && addr <= 0x3FFF) {
            addr &= 0x001F;
            if (addr == 0x0010) addr = 0x0000;
            if (addr == 0x0014) addr = 0x0004;
            if (addr == 0x0018) addr = 0x0008;
            if (addr == 0x001C) addr = 0x000C;
            data = tbl_palette[addr] & (mask.grayscale ? 0x30 : 0x3F);
        }

        return data;
    }


    ppu_write(addr, data) {
        addr &= 0x3FFF;

        if (cartridge_ppu_write(cart, addr, data)) {
            // Handled by cartridge
        }
        else if (addr >= 0x0000 && addr <= 0x1FFF && !cart->chr_is_rom) {
            tbl_pattern[(addr & 0x1000) >> 12][addr & 0x0FFF] = data;
        }
        else if (addr >= 0x2000 && addr <= 0x3EFF) {
            addr &= 0x0FFF;
            if (cartridge_mirror(cart) == VERTICAL) {
                if (addr <= 0x03FF) {
                    tbl_name[0][addr & 0x03FF] = data;
                }
                else if (addr <= 0x07FF) {
                    tbl_name[1][addr & 0x03FF] = data;
                }
                else if (addr <= 0x0BFF) {
                    tbl_name[0][addr & 0x03FF] = data;
                }
                else {
                    tbl_name[1][addr & 0x03FF] = data;
                }
            }
            else if (cartridge_mirror(cart) == HORIZONTAL) {
                if (addr <= 0x03FF) {
                    tbl_name[0][addr & 0x03FF] = data;
                }
                else if (addr <= 0x07FF) {
                    tbl_name[0][addr & 0x03FF] = data;
                }
                else if (addr <= 0x0BFF) {
                    tbl_name[1][addr & 0x03FF] = data;
                }
                else {
                    tbl_name[1][addr & 0x03FF] = data;
                }
            }
        }
        else if (addr >= 0x3F00 && addr <= 0x3FFF) {
            addr &= 0x001F;
            if (addr == 0x0010) addr = 0x0000;
            if (addr == 0x0014) addr = 0x0004;
            if (addr == 0x0018) addr = 0x0008;
            if (addr == 0x001C) addr = 0x000C;
            tbl_palette[addr] = data;
        }
    }

    IncrementScrollX() {
        if (mask.render_background || mask.render_sprites) {
            if (vram_addr.coarse_x == 31) {
                vram_addr.coarse_x = 0;
                vram_addr.nametable_x = ~vram_addr.nametable_x; // ~vram_addr.nametable_x & 1;
            } else {
                ++vram_addr.coarse_x;
            }
        }
    }

    IncrementScrollY() {
        if (mask.render_background || mask.render_sprites) {
            if (vram_addr.fine_y < 7) {
                ++vram_addr.fine_y;
            } else {
                vram_addr.fine_y = 0;
                if (vram_addr.coarse_y == 29) {
                    vram_addr.coarse_y = 0;
                    vram_addr.nametable_y = ~vram_addr.nametable_y; // ~vram_addr.nametable_y & 1;
                } else if (vram_addr.coarse_y == 31) {
                    vram_addr.coarse_y = 0;
                } else {
                    ++vram_addr.coarse_y;
                }
            }
        }
    }

    TransferAddressX() {
        if (mask.render_background || mask.render_sprites) {
            vram_addr.nametable_x = tram_addr.nametable_x;
            vram_addr.coarse_x = tram_addr.coarse_x;
        }
    }

    TransferAddressY() {
        if (mask.render_background || mask.render_sprites) {
            vram_addr.fine_y = tram_addr.fine_y;
            vram_addr.nametable_y = tram_addr.nametable_y;
            vram_addr.coarse_y = tram_addr.coarse_y;
        }
    }

    LoadBackgroundShifters() {
        bg_shifter_pattern_lo = (bg_shifter_pattern_lo & 0xFF00) | bg_next_tile_lsb;
        bg_shifter_pattern_hi = (bg_shifter_pattern_hi & 0xFF00) | bg_next_tile_msb;

        bg_shifter_attrib_lo = (bg_shifter_attrib_lo & 0xFF00) | ((bg_next_tile_attrib & 0b01) ? 0xFF : 0x00);
        bg_shifter_attrib_hi = (bg_shifter_attrib_hi & 0xFF00) | ((bg_next_tile_attrib & 0b10) ? 0xFF : 0x00);
    }

    UpdateShifters() {
        if (mask.render_background) {
            bg_shifter_pattern_lo <<= 1;
            bg_shifter_pattern_hi <<= 1;
            bg_shifter_attrib_lo <<= 1;
            bg_shifter_attrib_hi <<= 1;
        }

        if (mask.render_sprites && cycle >= 1 && cycle < 258) {
            for (i32 i = 0; i < sprite_count; ++i) {
                if (sprite_scanline[i].x > 0) {
                    --sprite_scanline[i].x;
                } else {
                    sprite_shifter_pattern_lo[i] <<= 1;
                    sprite_shifter_pattern_hi[i] <<= 1;
                }
            }
        }
    }

    clock() {
        if (scanline >= -1 && scanline < 240) {
            // -- BACKGROUND RENDERING --
            if (scanline == 0 && cycle == 0 && odd_frame && (mask.render_background || mask.render_sprites)) {
                cycle = 1;
            }

            if (scanline == -1 && cycle == 1) {
                // pretty much the start of a new frame
                this.status.vertical_blank = 0;
                this.status.sprite_overflow = 0;
                this.status.sprite_zero_hit = 0;

                for (i32 i = 0; i < 8; ++i) {
                    sprite_shifter_pattern_lo[i] = 0;
                    sprite_shifter_pattern_hi[i] = 0;
                }
            }

            if ((cycle >= 2 && cycle < 258) || (cycle >= 321 && cycle < 338)) {
                UpdateShifters(nes);

                switch ((cycle - 1) % 8) {
                    case 0:
                        LoadBackgroundShifters(nes);
                        
                        bg_next_tile_id = nes2C02_ppu_read(nes, 0x2000 | (vram_addr.reg & 0x0FFF), false);
                        break;
                    case 2:
                        bg_next_tile_attrib = nes2C02_ppu_read(nes, 0x23C0 | (vram_addr.nametable_y << 11)
                            | (vram_addr.nametable_x << 10)
                            | ((vram_addr.coarse_y >> 2) << 3)
                            | (vram_addr.coarse_x >> 2), false);

                        if (vram_addr.coarse_y & 0x02) bg_next_tile_attrib >>= 4;
                        if (vram_addr.coarse_x & 0x02) bg_next_tile_attrib >>= 2;
                        bg_next_tile_attrib &= 0x03;
                        break;

                        // Compared to the last two, the next two are the easy ones... :P

                    case 4:
                        bg_next_tile_lsb = nes2C02_ppu_read(nes, (control.pattern_background << 12)
                            + (bg_next_tile_id << 4)
                            + (vram_addr.fine_y), false);

                        break;
                    case 6:
                        bg_next_tile_msb = nes2C02_ppu_read(nes, (control.pattern_background << 12)
                            + (bg_next_tile_id << 4)
                            + (vram_addr.fine_y) + 8, false);
                        break;
                    case 7:
                        IncrementScrollX(nes);
                        break;
                }
            }


            // End of a visible scanline, increment downwards
            if (cycle == 256) {
                IncrementScrollY(nes);
            }

            // reset the x to go back to the start
            if (cycle == 257) {
                LoadBackgroundShifters(nes);
                TransferAddressX(nes);
            }

            // Superfluous reads of tile ID at the end of scanline
            if (cycle == 338 || cycle == 340) {
                bg_next_tile_id = nes2C02_ppu_read(nes, 0x2000 | (vram_addr.reg & 0x0FFF), false);
            }

            if (scanline == -1 && cycle >= 280 && cycle < 305) {
                // End of vertical blank period so reset the y address
                TransferAddressY(nes);
            }

            // -- FOREGROUND RENDERING --
            // Using a trick here that performs sprite evaluation all in one hit, the real NES doesn't do it like this though
            if (cycle == 257 && scanline >= 0) {
                for (let i = 0; i < 8; ++i) {
                    this.sprite_scanline[i].y = 0;
                    this.sprite_scanline[i].id = 0;
                    this.sprite_scanline[i].attribute = 0;
                    this.sprite_scanline[i].x = 0;
                }

                this.sprite_count = 0;

                for (let i = 0; i < 8; ++i) {
                    sprite_shifter_pattern_lo[i] = 0;
                    sprite_shifter_pattern_hi[i] = 0;
                }

                let oam_entry = 0;
                sprite_zero_hit_possible = false;

                while (oam_entry < 64 && sprite_count < 9) {
                    let diff = (scanline - oam[oam_entry].y);

                    if (diff >= 0 && diff < (control.sprite_size ? 16 : 8) && sprite_count < 8) {
                        if (sprite_count < 8) {
                            if (oam_entry == 0) {
                                sprite_zero_hit_possible = true;
                            }

                            this.sprite_scanline[this.sprite_count] = this.oam[oam_entry];
                        }

                        ++this.;
                    }

                    ++oam_entry;
                }

                // status.sprite_overflow = (sprite_count >= 8);
                this.status.sprite_overflow = (sprite_count > 8);
            }

            if (cycle == 340) {
                // End of scanline
                for (let i = 0; i < sprite_count; ++i) {
                    // Extract 8 bit row patterns of the sprite with vertical offset
                    let sprite_pattern_bits_lo, sprite_pattern_bits_hi;
                    let sprite_pattern_addr_lo, sprite_pattern_addr_hi;

                    if (!control.sprite_size) {
                        // 8x8 sprite mode
                        if (!(sprite_scanline[i].attribute & 0x80)) {
                            // sprite is not flipped vertically
                            sprite_pattern_addr_lo = (control.pattern_sprite << 12) | (sprite_scanline[i].id << 4) | (scanline - sprite_scanline[i].y);
                        } else {
                            // sprite is flipped vertically
                            sprite_pattern_addr_lo = (control.pattern_sprite << 12) | (sprite_scanline[i].id << 4) | (7 - (scanline - sprite_scanline[i].y));
                        }
                    } else {
                        // 8x16 sprite mode
                        if (!(sprite_scanline[i].attribute & 0x80)) {
                            // sprite is not flipped vertically
                            if (scanline - sprite_scanline[i].y < 8) {
                                // read top half tile
                                sprite_pattern_addr_lo = ((sprite_scanline[i].id & 0x01) << 12) | ((sprite_scanline[i].id & 0xFE) << 4) | ((scanline - sprite_scanline[i].y) & 0x07);
                            } else {
                                // read bottom half tile
                                sprite_pattern_addr_lo = ((sprite_scanline[i].id & 0x01) << 12) | (((sprite_scanline[i].id & 0xFE) + 1) << 4) | ((scanline - sprite_scanline[i].y) & 0x07);
                            }
                        } else {
                            // sprite is flipped vertically
                            if (scanline - sprite_scanline[i].y < 8) {
                                // read top half tile
                                sprite_pattern_addr_lo = ((sprite_scanline[i].id & 0x01) << 12) | (((sprite_scanline[i].id & 0xFE) + 1) << 4) | (7 - ((scanline - sprite_scanline[i].y) & 0x07));
                            } else {
                                // read bottom half tile
                                sprite_pattern_addr_lo = ((sprite_scanline[i].id & 0x01) << 12) | ((sprite_scanline[i].id & 0xFE) << 4) | (7 - ((scanline - sprite_scanline[i].y) & 0x07));
                            }
                        }
                    }

                    sprite_pattern_addr_hi = sprite_pattern_addr_lo + 8;

                    sprite_pattern_bits_lo = nes2C02_ppu_read(nes, sprite_pattern_addr_lo, false);
                    sprite_pattern_bits_hi = nes2C02_ppu_read(nes, sprite_pattern_addr_hi, false);

                    if (sprite_scanline[i].attribute & 0x40) {
                        sprite_pattern_bits_lo = flipbyte(sprite_pattern_bits_lo);
                        sprite_pattern_bits_hi = flipbyte(sprite_pattern_bits_hi);
                    }

                    sprite_shifter_pattern_lo[i] = sprite_pattern_bits_lo;
                    sprite_shifter_pattern_hi[i] = sprite_pattern_bits_hi;
                }
            }
        }

        // if (scanline == 240) { Post Render Scanline / do nothing }

        if (scanline >= 241 && scanline < 261) {
            if (scanline == 241 && cycle == 1) {
                this.status.vertical_blank = 1;

                if  (control.enable_nmi) {
                    nmi = true;
                }
            }
        }

        let bg_pixel = 0x00;
        let bg_palette = 0x00;

        if (mask.render_background) {
            if (mask.render_background_left || (cycle >= 9)) {
                let bit_mux = 0x8000 >> fine_x;

                let p0_pixel = (bg_shifter_pattern_lo & bit_mux) > 0;
                let p1_pixel = (bg_shifter_pattern_hi & bit_mux) > 0;

                bg_pixel = (p1_pixel << 1) | p0_pixel;

                let bg_pal0 = (bg_shifter_attrib_lo & bit_mux) > 0;
                let bg_pal1 = (bg_shifter_attrib_hi & bit_mux) > 0;
                bg_palette = (bg_pal1 << 1) | bg_pal0;
            }
        }

        let fg_pixel = 0x00;
        let fg_palette = 0x00;
        let fg_priority = 0x00;

        if (mask.render_sprites) {
            if (mask.render_sprites_left || (cycle >= 9)) {
                sprite_zero_being_rendered = false;

                for (let i = 0; i < sprite_count; ++i) {
                    if (sprite_scanline[i].x == 0) {
                        let fg_pixel_lo = (sprite_shifter_pattern_lo[i] & 0x80) > 0;
                        let fg_pixel_hi = (sprite_shifter_pattern_hi[i] & 0x80) > 0;
                        fg_pixel = (fg_pixel_hi << 1) | fg_pixel_lo;

                        fg_palette = (sprite_scanline[i].attribute & 0x03) + 0x04;
                        fg_priority = (sprite_scanline[i].attribute & 0x20) == 0;

                        if (fg_pixel != 0) {
                            if (i == 0) {
                                sprite_zero_being_rendered = true;
                            }

                            break;
                        }
                    }
                }
            }
        }

        let pixel = 0x00;
        let palette = 0x00;

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

            if (sprite_zero_hit_possible && sprite_zero_being_rendered) {
                if (mask.render_background & mask.render_sprites) {
                    if (!(mask.render_background_left || mask.render_sprites_left)) {
                        if (cycle >= 9 && cycle < 258) {
                            this.status.sprite_zero_hit = 1;
                        }
                    } else {
                        if (cycle >= 1 && cycle < 258) {
                            this.status.sprite_zero_hit = 1;
                        }
                    }
                }
            }
        }

        const x = cycle - 1;
        const y = scanline;
        const width = 256;
        const height = 240;
        if (x >= 0 && x < width && y >= 0 && y < height) {
            this.screen[y * width + x] = this.get_color_from_palette_ram(nes, palette, pixel);
        }

        ++cycle;
        if (mask.render_background || mask.render_sprites) {
            if (cycle == 260 && scanline < 240) {
                this.cart.mapper().scanline();
            }
        }

        if (cycle >= 341) {
            cycle = 0;
            ++scanline;
            if (scanline >= 261) {
                scanline = -1;
                frame_complete = true;
                odd_frame = !odd_frame;
            }
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

        this.cart = null;
    }
}