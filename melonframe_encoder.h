#ifndef MELONFRAME_MELONFRAME_ENCODER_H
#define MELONFRAME_MELONFRAME_ENCODER_H

#include <stdlib.h>
#include <stdint.h>

ssize_t get_size_for_encoded(size_t buffer_size);

ssize_t encode(
    const uint8_t *bytes,
    size_t bytes_offset,
    size_t bytes_size,
    uint8_t *encoded,
    size_t encoded_offset,
    size_t encoded_size);

#endif //MELONFRAME_MELONFRAME_ENCODER_H
