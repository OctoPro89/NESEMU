#pragma once
#include <emu/6502/nes6502.h>
#include <emu/2C02/nes2C02.h>
#include <emu/2A03/nes2A03.h>
#include <emu/cartridge.h>

typedef struct bus {
    nes6502 cpu;
    nes2C02 ppu;
    nes2A03 apu;
    cartridge* cart;

    u8* ram;
    u8 controller[2];
    u8 controller_state[2];
    u8 controller_strobe;

    f64 audio_sample;
    f64 audio_time;
    f64 audio_global_time;
    f64 audio_time_per_nes_clock;
    f64 audio_time_per_system_sample;

    u32 system_clock_counter;
    // u32 dma_stall_cycles;

    u8 dma_page;
    u8 dma_addr;
    u8 dma_data;

    u8 dma_dummy;
    u8 dma_transfer;
} bus;

bus bus_init();
void bus_destroy(bus* b);

void bus_set_sample_frequency(bus* b, u32 sample_rate);
void bus_cpu_write(bus* b, u16 addr, u8 data);
u8 bus_cpu_read(bus* b, u16 addr, u8 read_only);

void bus_insert_cartridge(bus* b, cartridge* cart);
void bus_reset(bus* b);
u8 bus_clock(bus* b);