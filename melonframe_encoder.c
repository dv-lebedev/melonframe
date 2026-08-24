#include "melonframe_encoder.h"
#include <string.h>
#include "melonframe_crc.h"
#include "melonframe.h"

melonframe_error_t melonframe_get_size_for_encoded(const size_t buffer_size, size_t *encoded_size) {
    const size_t size = buffer_size + MELONFRAME_PROTO_HEADER_SIZE + MELONFRAME_PROTO_PAYLOAD_SIZE + MELONFRAME_PROTO_CRC_SIZE;

    if (size > MELONFRAME_PROTO_FRAME_MAX_SIZE)
        return MELONFRAME_ERR_BUFFER_OVERFLOW;

    *encoded_size = size;

    return MELONFRAME_OK;
}

melonframe_error_t melonframe_encode(
    const uint8_t *bytes,
    const size_t bytes_size,
    uint8_t *encoded,
    const size_t encoded_size) {

    if (!bytes || !encoded) {
        return MELONFRAME_ERR_NULL_ARG;
    }

    size_t pack_size;
    const melonframe_error_t err = melonframe_get_size_for_encoded(bytes_size, &pack_size);
    if (err != MELONFRAME_OK) {
        return err;
    }

    if (encoded_size < pack_size) {
        return MELONFRAME_ERR_BUFFER_OVERFLOW;
    }

    encoded[0] = 0xAA;
    encoded[1] = 0x55;

    encoded[2] = ((uint16_t)bytes_size) >> 8;
    encoded[3] = ((uint16_t)bytes_size);

    memcpy(encoded + MELONFRAME_PROTO_HEADER_SIZE + MELONFRAME_PROTO_PAYLOAD_SIZE, bytes, bytes_size);

    const size_t crc_payload_size = MELONFRAME_PROTO_HEADER_SIZE + MELONFRAME_PROTO_PAYLOAD_SIZE + bytes_size;
    const uint16_t crc = melonframe_crc16(encoded,0, crc_payload_size);

    encoded[pack_size - MELONFRAME_PROTO_CRC_SIZE] = (uint8_t)(crc >> 8);
    encoded[pack_size - MELONFRAME_PROTO_CRC_SIZE + 1] = (uint8_t)(crc);

    return MELONFRAME_OK;
}