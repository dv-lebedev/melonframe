#ifndef MELONFRAME_MELONFRAME_DECODER_H
#define MELONFRAME_MELONFRAME_DECODER_H

#include <stdint.h>

#include "melonframe.h"

typedef enum {
    STATE_SEARCH_HEADER,
    STATE_READ_LENGTH,
    STATE_READ_PAYLOAD,
    STATE_READ_CRC
} decoder_state_t;

typedef void (*data_handler_cb_t)(void *ctx, melonframe_status_t status, uint8_t *data, size_t data_len);

typedef struct {
    decoder_state_t state;
    uint8_t *buffer;
    size_t buffer_size;
    size_t payload_len;
    uint32_t pos;
    data_handler_cb_t data_handler;
    void *ctx;
} melonframe_decoder_t;

melonframe_error_t melonframe_decoder_init(
    melonframe_decoder_t *p,
    size_t buffer_size,
    data_handler_cb_t data_handler,
    void *ctx);

melonframe_error_t melonframe_decoder_free(melonframe_decoder_t *p);

melonframe_error_t melonframe_decoder_reset(melonframe_decoder_t *p);

melonframe_error_t melonframe_decoder_process_byte(melonframe_decoder_t *p, uint8_t b);


#endif //MELONFRAME_MELONFRAME_DECODER_H
