#include <emu/emu.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    const char* test_fp = "smb_duck_hunt.nes";

    const char* cart_fp = argc == 2 ? argv[1] : test_fp;
    const char* save_fp = argc == 3 ? argv[2] : NULL;

    if (!emu_run(cart_fp, save_fp)) {
        printf("Failed to start emulator!\n");
        return 1;
    }

    return 0;
}
