const MIRROR = {
    HORIZONTAL: 0,
    VERTICAL: 1,
    HARDWARE: 2,
};

class cartridge {
    constructor(file) {
        this.mapper_id = 0;
        this.prg_banks = 0;
        this.chr_banks = 0;
        this.prg_memory = null;
        this.chr_memory = null;
        this.hw_mirror = MIRROR.HORIZONTAL;
        this.chr_is_rom = false;
        this.image_valid = false;
        this.m = null;

        const reader = new FileReader();

        reader.onload = function (e) {
            const buffer = e.target.result;
            const view = new DataView(buffer);

            const name =
                String.fromCharCode(view.getUint8(0)) +
                String.fromCharCode(view.getUint8(1)) +
                String.fromCharCode(view.getUint8(2)) +
                String.fromCharCode(view.getUint8(3));

            if (name !== "NES\u001A") {
                log("Invalid NES file.");
                return;
            }

            const prg_rom_chunks = view.getUint8(4);
            const chr_rom_chunks = view.getUint8(5);
            const mapper1 = view.getUint8(6);
            const mapper2 = view.getUint8(7);
            const prg_ram_size = view.getUint8(8);

            let offset = 16;

            if (mapper1 & 0x04) {
                offset += 512;
            }

            this.mapper_id = ((mapper2 >> 4) << 4) | (mapper1 >> 4);
            this.hw_mirror = (mapper1 & 0x01) ? MIRROR.VERTICAL : MIRROR.HORIZONTAL;

            const file_type = ((mapper2 & 0x0C) === 0x08) ? 2 : 1;

            if (file_type === 1) {
                this.prg_banks = prg_rom_chunks;
                const prg_size = this.prg_banks * 16384;
                this.prg_memory = new Uint8Array(buffer.slice(offset, offset + prg_size));
                offset += prg_size;

                this.chr_banks = chr_rom_chunks;
                if (this.chr_banks === 0) {
                    this.chr_memory = new Uint8Array(8192);
                } else {
                    const chr_size = this.chr_banks * 8192;
                    this.chr_memory = new Uint8Array(buffer.slice(offset, offset + chr_size));
                    offset += chr_size;
                }
            } else {
                log("Unsupported NES 2.0 format (file type 2)");
                return;
            }

            this.m = cartridge.create_mapper(this.mapper_id, this.prg_banks, this.chr_banks);

            this.image_valid = true;

            if (this.chr_banks == 0) {
                this.chr_is_rom = false;
            } else {
                this.chr_is_rom = true;
            }

            log(`[MAPPER] Mapper ID: ${this.mapper_id}`);
            log(`[MAPPER] PRG ROM Banks: ${this.prg_banks}`);
            log(`[MAPPER] CHR ROM Banks: ${this.chr_banks}`);
            log(`[MAPPER] CHR is ${this.chr_is_rom ? "ROM" : "RAM"}`);

            if (!this.chr_is_rom) {
                log("[INFO] CHR RAM allocated (8 KB)");
            } else {
                log(`[INFO] CHR ROM size: ${this.chr_banks * 8} KB`);
            }

            if (this.mapper_id === 0) {
                log("[M0] Mapping PRG ROM...");
                log(" - PRG $8000-$BFFF = bank 0");
                log(` - PRG $C000-$FFFF = bank ${this.prg_banks - 1}`);
            }
        };

        reader.readAsArrayBuffer(file);
    }

    static create_mapper(id, prg, chr) {
        return { id, prg, chr, type: `mapper_${id}` };
    }

    is_valid() {
        return this.image_valid;
    }

    destroy() {
        this.prg_memory = null;
        this.chr_memory = null;
        this.image_valid = false;
    }

    cpu_read(addr) {
        log("cpu_read() not implemented");
        return 0;
    }

    cpu_write(addr, value) {
        log("cpu_write() not implemented");
    }

    ppu_read(addr) {
        log("ppu_read() not implemented");
        return 0;
    }

    ppu_write(addr, value) {
        log("ppu_write() not implemented");
    }

    mirror() {
        return MIRROR.HARDWARE ? this.hw_mirror : MIRROR.HORIZONTAL;
    }
}

document.getElementById("rom_input").addEventListener("change", async function (e) {
    const file = e.target.files[0];
    if (file) {
        let cart = new cartridge(file);
        await main(cart);
    }
});