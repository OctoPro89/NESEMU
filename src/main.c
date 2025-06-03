#include <emu/emu.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    const char* cart_fp = NULL;
    const char* test_fp = "nestest.nes";
    
    if (argc < 2) {
        cart_fp = test_fp;
    } else {
        cart_fp = argv[1];
    }

    if (!emu_run(cart_fp)) {
        printf("Failed to start emulator!\n");
        return 1;
    }

    return 0;
}
