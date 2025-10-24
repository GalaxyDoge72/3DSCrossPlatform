#include "CRC.h"
// No need for externs here, C++ compiler is the one building the function. //

uint32_t calcCRC32(const uint8_t *data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < length; i++) {
        // XOR the current byte with the LSB of the CRC //
        crc ^= data[i];

        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ CRC32_POLYNOMAL;
            }
            else {
                crc >>= 1;
            }
        }
    }
    return ~crc;
}