#include "emu.h"
#include "bus.h"
#include <platform.h>

#include <stdio.h>
#include <stdlib.h>

u32 frame_count = 0;
f64 fps_timer = 0.0;
f64 fps = 0.0;

const f64 target_frame_time_ms = 1000.0 / 60.0988;  // ~16.639 ms

char h[9];

char* hex(uint32_t n, uint8_t d) {
    static const char hex_digits[] = "0123456789ABCDEF";

    for (int i = d - 1; i >= 0; --i, n >>= 4)
        h[i] = hex_digits[n & 0xF];

    h[d] = '\0';  // Null-terminate the string
    return h;
}

u8 emu_run(const char* cart_fp) {
    platform_open_window(256 * 4, 240 * 4);
    fps_timer = platform_get_elapsed_time_ms();

    bus nes = bus_init();

    cartridge cart = cartridge_init(cart_fp);

    if (!cartridge_image_valid(&cart)) {
        printf("Invalid cartridge image! (Failed to load cartridge)\n");
        return false;
    }

    printf("Loaded cart: %s\n", cart_fp);

    bus_insert_cartridge(&nes, &cart);

    bus_reset(&nes);
    bus_set_sample_frequency(&nes, 44100);

    f64 prev_time = 0.0;
    f64 now_time = 0.0;
    f64 residual_time = 0.0;
    f64 elapsed_time = 0.0;

    while (platform_should_run()) {
        f64 frame_start_time = platform_get_elapsed_time_ms();

        platform_pump_messages();

        keys keyboard = platform_get_keys();

        nes.controller[0] =
            (keyboard.right << 0) |
            (keyboard.left << 1) |
            (keyboard.down << 2) |
            (keyboard.up << 3) |
            (keyboard.enter << 4) |
            (keyboard.shift << 5) |
            (keyboard.x << 6) |
            (keyboard.z << 7);

        now_time = platform_get_elapsed_time_ms();
        elapsed_time = ((now_time - prev_time) / 1000.0);
        prev_time = now_time;

        if (residual_time > 0.0) {
            residual_time -= elapsed_time;
        }
        else {
            residual_time += (1.0 / 60.0) - elapsed_time;
            do {
                bus_clock(&nes);
            } while (!nes.ppu.frame_complete);
            nes.ppu.frame_complete = false;
        }

        // Draw rendered output
        for (u32 j = 0; j < NES_HEIGHT; ++j) {
            for (u32 i = 0; i < NES_WIDTH; ++i) {
                u32 index = j * NES_WIDTH + i;
                pixel* p = &nes.ppu.screen[index];

                // Flip the Y-coordinate
                u32 flipped_j = NES_HEIGHT - 1 - j;

                platform_put_pixel(i, flipped_j, p->r, p->g, p->b);
            }
        }

        if (keyboard.u) {
            system("cls");

            printf("Status: %s %s %s %s %s %s %s %s\n",
                nes.cpu.status & FLAGS_6502_N ? "N" : " ",
                nes.cpu.status & FLAGS_6502_V ? "V" : " ",
                nes.cpu.status & FLAGS_6502_U ? "-" : " ",
                nes.cpu.status & FLAGS_6502_B ? "B" : " ",
                nes.cpu.status & FLAGS_6502_D ? "D" : " ",
                nes.cpu.status & FLAGS_6502_I ? "I" : " ",
                nes.cpu.status & FLAGS_6502_Z ? "Z" : " ",
                nes.cpu.status & FLAGS_6502_C ? "C" : " "
            );
            printf("PC: $%s\n", hex(nes.cpu.pc, 4));
            printf("A: $%s\n", hex(nes.cpu.a, 2));
            printf("X: $%s\n", hex(nes.cpu.x, 2));
            printf("Y: $%s\n", hex(nes.cpu.y, 2));
            printf("Stack Pointer: $%s\n", hex(nes.cpu.stkp, 4));

            for (int i = 0; i < 26; i++)
            {
                char s[100] = "";
                snprintf(s, 100, "%s : (%u, %u) ID: %s AT: %s\n",
                    hex(i, 2),
                    ((u8*)nes.ppu.oam)[i * 4 + 3],
                    ((u8*)nes.ppu.oam)[i * 4],
                    hex(((u8*)nes.ppu.oam)[i * 4 + 1], 2),
                    hex(((u8*)nes.ppu.oam)[i * 4 + 2], 2)
                );

                printf("%s\n", s);
            }
        }

        platform_render();

        printf("FPS: %0.1f\n", fps);

        frame_count++;

        f64 current_time = platform_get_elapsed_time_ms();
        if (current_time - fps_timer >= 1000.0) {
            fps = (f64)frame_count * 1000.0 / (current_time - fps_timer);
            fps_timer = current_time;
            frame_count = 0;
        }

        // Throttle to ~60fps
        f64 frame_end_time = platform_get_elapsed_time_ms();
        f64 elapsed = frame_end_time - frame_start_time;
        if (elapsed < target_frame_time_ms) {
            // This sleep needs to be precise, could also just while loop for insanely precise, albeight using 100% cpu
            platform_sleep_ms((u32)(target_frame_time_ms - elapsed));
        }
    }

    platform_shutdown();

    bus_destroy(&nes);

    return true;
}