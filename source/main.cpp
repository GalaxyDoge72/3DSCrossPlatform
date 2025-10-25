// C++ Standard Libraries //
#include <stdio.h>
#include <cstring>
#include <stdint.h>

// 3DS DevKitPro Libraries //
#include <3ds.h>

// Custom Libraries //
#include "CRC.h"
#include "netscan.h"

int main() {
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

    printf("Netscanning Demo\n");
    printf("Make sure Wi-Fi is enabled and a connection is active.\n");
    printf("Press A to start scan, START to exit.\n\n");

    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();

        if (kDown & KEY_START){
            break;
        }

        if (kDown & KEY_A) {
            char foundIP[64] = {0};

            printf("Starting subnet scan...\n");
            bool found = startSubnetScan(foundIP, sizeof(foundIP));

            if (found) {
                printf("FOUND: %s:%d\n", foundIP, TARGET_PORT);
            }
            else {
                printf("No hosts found in range %s%d..%d (port %d)\n",
                    SUBNET_BASE, IP_RANGE_START, IP_RANGE_END, TARGET_PORT);
            }

            printf("\nScan finished. Press A to scan again or START to exit.\n");
        }

        // basic frame housekeeping
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    gfxExit();
    return 0;
}