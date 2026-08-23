#include "melonframe_crc.h"
#include <stdint.h>

uint16_t crc16(const uint8_t *data, size_t offset, size_t size) {
    uint16_t crc = 0xFFFF;
    for (size_t i = offset; i < size; i++) {
        crc ^= (uint16_t)data[i];
        for (int b = 0; b < 8; b++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc = crc >> 1;
            }
        }
    }
    return crc;
}