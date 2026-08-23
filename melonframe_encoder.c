#include "melonframe_encoder.h"
#include <string.h>
#include "melonframe_crc.h"
#include "melonframe.h"

ssize_t get_size_for_encoded(const size_t buffer_size) {
    const size_t size = buffer_size + MELONFRAME_HEADER + MELONFRAME_PAYLOAD_SIZE + MELONFRAME_CRC;
    if (size > MELONFRAME_MAX_SIZE)
        return -1;
    return (ssize_t)size;
}

ssize_t encode(
    const uint8_t *bytes,
    const size_t bytes_offset,
    const size_t bytes_size,
    uint8_t *encoded,
    const size_t encoded_offset,
    const size_t encoded_size) {

    if (!bytes || !encoded) {
        return -1;
    }

    const ssize_t pack_size = get_size_for_encoded(bytes_size);
    if (pack_size == -1 || pack_size > encoded_size)
        return -1;

    encoded[encoded_offset] = 0xAA;
    encoded[encoded_offset + 1] = 0x55;

    encoded[encoded_offset + 2] = ((uint16_t)bytes_size) >> 8;
    encoded[encoded_offset + 3] = ((uint16_t)bytes_size);

    memcpy(encoded + encoded_offset + MELONFRAME_HEADER + MELONFRAME_PAYLOAD_SIZE,
           bytes + bytes_offset,
           bytes_size);

    const size_t crc_payload_size = MELONFRAME_HEADER + MELONFRAME_PAYLOAD_SIZE + bytes_size;
    const uint16_t crc = crc16(encoded + encoded_offset,0, crc_payload_size);

    encoded[encoded_offset + (size_t)pack_size - MELONFRAME_CRC] = (uint8_t)(crc >> 8);
    encoded[encoded_offset + (size_t)pack_size - MELONFRAME_CRC + 1] = (uint8_t)(crc);

    return pack_size;
}