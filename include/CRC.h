#ifndef CRC_H
#define CRC_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CRC32_POLYNOMAL 0xEDB88320

uint32_t calcCRC32(const uint8_t *data, size_t length);

#ifdef __cplusplus
}
#endif

#endif // CRC_H