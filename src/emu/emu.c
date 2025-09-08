#include "emu.h"
#include "bus.h"
#include <platform.h>

#include <stdio.h>
#include <stdlib.h>

char h[9];

char* hex(uint32_t n, uint8_t d) {
    static const char hex_digits[] = "0123456789ABCDEF";

    for (int i = d - 1; i >= 0; --i, n >>= 4)
        h[i] = hex_digits[n & 0xF];

    h[d] = '\0';  // Null-terminate the string
    return h;
}

u8 emu_run(const char* cart_fp, const char* save_fp) {
    platform_open_window(NES_WIDTH * 4, NES_HEIGHT * 4);

    bus_global = bus_init();

    cartridge cart = cartridge_init(cart_fp);

    if (save_fp != NULL) {
        if (!cartridge_load_save(&cart, save_fp)) {
            printf("Failed to open save file or game doesn't support loading saves.\n");
        }
    }

    if (!cartridge_image_valid(&cart)) {
        printf("Invalid cartridge image! (Failed to load cartridge)\n");
        return false;
    }

    printf("Loaded cart: %s\n", cart_fp);

    bus_insert_cartridge(&bus_global, &cart);

    bus_reset(&bus_global);
    bus_set_sample_frequency(&bus_global, 48000);

    while (platform_should_run()) {
        platform_pump_messages();

        keys keyboard = platform_get_keys();

        bus_global.controller[0] =
            (keyboard.right << 0) |
            (keyboard.left << 1) |
            (keyboard.down << 2) |
            (keyboard.up << 3) |
            (keyboard.enter << 4) |
            (keyboard.shift << 5) |
            (keyboard.z << 6) |
            (keyboard.x << 7);

        // Draw rendered output
        for (u32 j = 0; j < NES_HEIGHT; ++j) {
            for (u32 i = 0; i < NES_WIDTH; ++i) {
                u32 index = j * NES_WIDTH + i;
                pixel* p = &bus_global.ppu.screen[index];

                // Flip the Y-coordinate
                u32 flipped_j = NES_HEIGHT - 1 - j;

                platform_put_pixel(i, flipped_j, p->r, p->g, p->b);
            }
        }

        // TODO: Platform message boxes
        if (keyboard.s) {
            if (!cartridge_save_to_file(&cart, "save_file")) {
                printf("File failed to open or game doesn't support saving.\n");
            }
        }

        platform_render();
    }

    platform_shutdown();

    bus_destroy(&bus_global);

    return true;
}
