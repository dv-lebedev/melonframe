#ifndef MELONFRAME_MELONFRAME_ENCODER_H
#define MELONFRAME_MELONFRAME_ENCODER_H

#include <stdlib.h>
#include <stdint.h>

#include "melonframe.h"

melonframe_error_t melonframe_get_size_for_encoded(size_t buffer_size, size_t *encoded_size);

melonframe_error_t melonframe_encode(
    const uint8_t *bytes,
    size_t bytes_size,
    uint8_t *encoded,
    size_t encoded_size);

#endif //MELONFRAME_MELONFRAME_ENCODER_H
