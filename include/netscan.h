#ifndef NETSCAN_H
#define NETSCAN_H

#include <stdint.h>
#include <stdbool.h>
#include <cstddef>

#define TARGET_PORT 1411
#define SCAN_TIMEOUT_SECONDS 0.2

#define SUBNET_BASE "192.168.0."
#define IP_RANGE_START 1
#define IP_RANGE_END 254
#define MAX_THREADS 10

bool scanIPForPort(const char* ipAddr);

bool startSubnetScan(char* foundIPBuffer, size_t bufSize);

#endif // NETSCAN_H