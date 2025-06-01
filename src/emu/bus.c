#include "bus.h"
#include <emu/mappers/mapper_base.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

#define NES_PPU_CLOCK 5369318.0

bus bus_init() {
    bus b = { 0 };
    b.dma_dummy = true;
    b.ram = (u8*)malloc(2048);
    memset(b.ram, 0, 2048);

    b.cpu = nes6502_init();
    nes6502_connect_bus(&b.cpu, &b);

    b.ppu = nes2C02_init();
    // b.apu = nes2A03_init();

    return b;
}

void bus_destroy(bus* b) {
    nes6502_destroy(&b->cpu);
    nes2C02_destroy(&b->ppu);
    // nes2A03_destroy(&b->cpu);
    cartridge_destroy(b->cart);
}

void bus_set_sample_frequency(bus* b, u32 sample_rate) {
    b->audio_time_per_system_sample = 1.0 / (f64)sample_rate;
    b->audio_time_per_nes_clock = 1.0 / NES_PPU_CLOCK; // PPU Clock Frequency
}

void bus_cpu_write(bus* b, u16 addr, u8 data) {
    if (cartridge_cpu_write(b->cart, addr, data)) {
        // Custom hardware, nes doesn't use it
    } else if (addr >= 0x0000 && addr <= 0x1FFF) {
        b->ram[addr & 0x07FF] = data;
    } else if (addr >= 0x2000 && addr <= 0x3FFF) {
        nes2C02_cpu_write(&b->ppu, addr & 0x007, data);
    } else if ((addr >= 0x4000 && addr <= 0x4013) || addr == 0x4015 || addr == 0x4017) {
        // nes2A03_cpu_write(&b->apu, addr, data);
    } else if (addr == 0x4014) {
        b->dma_page = data;
        b->dma_addr = 0x00;
        b->dma_transfer = true;

        // Stall CPU
        // b->dma_stall_cycles = (b->system_clock_counter % 2 == 1) ? 514 : 513;
    } else if (addr == 0x4016) {
        b->controller_strobe = data & 0x01;

        if (b->controller_strobe) {
            // Latch the controller state when writing 1
            b->controller_state[0] = b->controller[0];
            b->controller_state[1] = b->controller[1];
        }
    }
}

u8 bus_cpu_read(bus* b, u16 addr, u8 read_only) {
    u8 data = 0x00;
    if (cartridge_cpu_read(b->cart, addr, &data)) {
        // Cartridge address range
    } else if (addr >= 0x0000 && addr <= 0x1FFF) {
        data = b->ram[addr & 0x07FF];
    } else if (addr >= 0x2000 && addr <= 0x3FFF) {
        data = nes2C02_cpu_read(&b->ppu, addr & 0x0007, read_only);
    } else if (addr == 0x4015) {
        // data = nes2A03_cpu_read(&b->apu, addr & 0x0007, read_only);
    } else if (addr == 0x4016) {
        u8 bit = (b->controller_state[0] & 0x80) ? 1 : 0;

        if (!b->controller_strobe) {
            b->controller_state[0] <<= 1;
        }

        return bit;
    }
    else if (addr == 0x4017) {
        u8 bit = (b->controller_state[1] & 0x80) ? 1 : 0;

        if (!b->controller_strobe) {
            b->controller_state[1] <<= 1;
        }

        return bit;
    }

    return data;
}

void bus_insert_cartridge(bus* b, cartridge* cart) {
    b->cart = cart;
    b->cpu.b = b; // Not sure why but msvc needs this
    nes2C02_connect_cartridge(&b->ppu, cart);
}

void bus_reset(bus* b) {
    cartridge_reset(b->cart);
    nes6502_reset(&b->cpu);
    nes2C02_reset(&b->ppu);
    b->system_clock_counter = 0;
    b->dma_page = 0x00;
    b->dma_addr = 0x00;
    b->dma_data = 0x00;
    b->dma_dummy = true;
    b->dma_transfer = false;
}

u8 bus_clock(bus* b) {
	nes2C02_clock(&b->ppu);
	// nes2A03_clock(&b->apu);

	// CPU runs 3 times slower than the apu and ppu
	if (b->system_clock_counter % 3 == 0) {
		if (b->dma_transfer) {
			// Wait for next clock
			if (b->dma_dummy) {
				// Wait a couple cycles
				if (b->system_clock_counter % 2 == 1) {
                    // Start DMA
					b->dma_dummy = false;
				}
			}
			else {
				// DMA can take place!
				if (b->system_clock_counter % 2 == 0) {
					// On even cycles read from cpu
					b->dma_data = bus_cpu_read(b, b->dma_page << 8 | b->dma_addr, false);
				}
				else {
					// On odd cycles write to PPU OAM
                    u8* p_oam = (u8*)&b->ppu.oam[0];
                    p_oam[b->dma_addr] = b->dma_data;
					// Increment addr lo byte 
					b->dma_addr++;
                    // If this wraps around then all 256 bytes have been written
					if (b->dma_addr == 0x00) {
						b->dma_transfer = false;
						b->dma_dummy = true;
					}
				}
			}
		}
		else {
            // No DMA, keep clockin dem cycles
			nes6502_clock(&b->cpu);
		}
	}

	// Synchronize with audio
    u8 sample_ready = false;
	b->audio_time += b->audio_time_per_nes_clock;
	if (b->audio_time >= b->audio_time_per_system_sample) {
		b->audio_time -= b->audio_time_per_system_sample;
		// b->audio_sample = nes2A03_get_output_sample(&b->apu);
        sample_ready = true;
	}

    // Vertical blank ended
	if (b->ppu.nmi) {
        b->ppu.nmi = false;
		nes6502_nmi(&b->cpu);
	}


	// Check if cart is requesting IRQ
	if (mapper_irq_state(cartridge_mapper(b->cart))) {
        mapper_irq_clear(cartridge_mapper(b->cart));
		nes6502_irq(&b->cpu);
	}

	b->system_clock_counter++;

	return sample_ready;
}