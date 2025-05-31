#include <emu/emu.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    if (!emu_run()) {
        printf("Failed to start emulator!\n");
        return 1;
    }

    return 0;
}