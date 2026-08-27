/*
*    MIT License
 *
 *    Copyright (c) 2026 Denis Lebedev
 *
 *    Permission is hereby granted, free of charge, to any person obtaining a copy
 *    of this software and associated documentation files (the "Software"), to deal
 *    in the Software without restriction, including without limitation the rights
 *    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *    copies of the Software, and to permit persons to whom the Software is
 *    furnished to do so, subject to the following conditions:
 *
 *    The above copyright notice and this permission notice shall be included in all
 *    copies or substantial portions of the Software.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *    SOFTWARE.
 */


#include "melonframe.h"
#include <stdint.h>
#include <string.h>


uint16_t melonframe_crc16(const uint8_t *data, const size_t offset, const size_t data_len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = offset; i < data_len; i++) {
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

melonframe_result_t melonframe_get_size_for_encoded(const size_t buffer_size, size_t *encoded_size) {
    const size_t size = buffer_size + MELONFRAME_PROTO_HEADER_SIZE +
            MELONFRAME_PROTO_PAYLOAD_SIZE + MELONFRAME_PROTO_CRC_SIZE;

    if (size > MELONFRAME_PROTO_FRAME_MAX_SIZE)
        return MELONFRAME_ERR_BUFFER_OVERFLOW;

    *encoded_size = size;

    return MELONFRAME_OK;
}

melonframe_result_t melonframe_encode(
    const uint8_t *bytes,
    const size_t bytes_size,
    uint8_t *encoded,
    const size_t encoded_size) {

    if (!bytes || !encoded) {
        return MELONFRAME_ERR_NULL_ARG;
    }

    size_t pack_size;
    const melonframe_result_t err = melonframe_get_size_for_encoded(bytes_size, &pack_size);
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

/* positions */
enum {
    MELONFRAME_POS_BEGINNING_OF_HEADER = 1,
    MELONFRAME_POS_END_OF_HEADER = 2,
    MELONFRAME_POS_END_OF_LEN_FIELD = 4,
};


melonframe_result_t melonframe_decoder_init(
    melonframe_decoder_t *p,
    melonframe_buffer_t *buffer,
    const melonframe_decoder_event_handler_t handler,
    void *context) {

    memset(p, 0, sizeof(*p));

    p->buffer = buffer;
    p->state = STATE_SEARCH_HEADER;
    p->handler = handler;
    p->context = context;

    return MELONFRAME_OK;
}

melonframe_result_t melonframe_decoder_free(melonframe_decoder_t *p) {
    return MELONFRAME_OK;
}

 melonframe_result_t melonframe_decoder_reset(melonframe_decoder_t *p) {
    p->state = STATE_SEARCH_HEADER;
    p->pos = 0;
    return MELONFRAME_OK;
}

uint8_t *get_buffer_data(melonframe_decoder_t *p) {
    if (!p || !p->buffer || !p->buffer->data) {
        return NULL;
    }
    return p->buffer->data;
}

static melonframe_result_t push_to_buffer(melonframe_decoder_t *p, const uint8_t b) {
    if (!p) {
        return MELONFRAME_ERR_NULL_POINTER;
    }

    if (!p->buffer || !p->buffer->data) {
        return MELONFRAME_ERR_BUFFER_IS_NO_INITIALIZED;
    }

    if (p->pos >= p->buffer->size) {
        return MELONFRAME_ERR_BUFFER_OVERFLOW;
    }

    p->buffer->data[p->pos++] = b;
    return MELONFRAME_OK;
}

static melonframe_result_t process_header(melonframe_decoder_t *p, const uint8_t b, melonframe_decoder_event_t *status) {
    const melonframe_result_t err = push_to_buffer(p, b);
    if (err != MELONFRAME_OK) {
        return err;
    }

    if (p->pos == MELONFRAME_POS_END_OF_HEADER) {
        uint8_t *buf = get_buffer_data(p);
        const uint16_t header = (uint16_t)((buf[0] << 8) | buf[1]);

        if (header == MELONFRAME_PROTO_HEADER_VALUE) {
            p->state = STATE_READ_LENGTH;
        } else {
            /* resync: slide the window by one byte */
            buf[0] = buf[1];
            p->pos = MELONFRAME_POS_BEGINNING_OF_HEADER;
            *status = MELONFRAME_STATUS_OUT_OF_SYNC;
        }
    }

    return MELONFRAME_OK;
}

static melonframe_result_t process_length(melonframe_decoder_t *p, const uint8_t b, melonframe_decoder_event_t *status) {
    *status = MELONFRAME_STATUS_PROCESSING_LENGTH;
    const melonframe_result_t err = push_to_buffer(p, b);
    if (err != MELONFRAME_OK) {
        return err;
    }

    if (p->pos == MELONFRAME_POS_END_OF_LEN_FIELD) {
        const uint8_t *buf = get_buffer_data(p);
        const uint16_t payload_len = (uint16_t)((buf[2] << 8) | buf[3]);
        p->payload_len = payload_len;

        if (payload_len == 0) {
            p->state = STATE_READ_CRC;
        } else if (payload_len > p->buffer->size) {
            // TODO: handle this more gracefully
            p->state = STATE_SEARCH_HEADER;
        } else {
            p->state = STATE_READ_PAYLOAD;
        }
    }

    return MELONFRAME_OK;
}

static melonframe_result_t process_payload(melonframe_decoder_t *p, const uint8_t b, melonframe_decoder_event_t *status) {
    *status = MELONFRAME_STATUS_PROCESSING_PAYLOAD;
    const melonframe_result_t err = push_to_buffer(p, b);
    if (err != MELONFRAME_OK) {
        return err;
    }

    if ((p->pos - MELONFRAME_PROTO_HEADER_SIZE - MELONFRAME_PROTO_PAYLOAD_SIZE) == p->payload_len) {
        p->state = STATE_READ_CRC;
    }

    return MELONFRAME_OK;
}

static melonframe_result_t process_crc(melonframe_decoder_t *p, const uint8_t b, melonframe_decoder_event_t *status) {
    *status = MELONFRAME_STATUS_PROCESSING_CRC;
    const melonframe_result_t err = push_to_buffer(p, b);
    if (err != MELONFRAME_OK) {
        return err;
    }

    const uint32_t data_size = MELONFRAME_PROTO_HEADER_SIZE + MELONFRAME_PROTO_PAYLOAD_SIZE + p->payload_len;
    if (p->pos < data_size + 2) {
        return MELONFRAME_OK;
    }

    const uint8_t *buf = get_buffer_data(p);

    const uint16_t crc =
        (uint16_t)((buf[data_size] << 8)  // CRC_H
            | buf[data_size + 1]);        // CRC_L

    const uint16_t calc = melonframe_crc16(buf, 0, data_size);
    * status = (calc == crc ? MELONFRAME_STATUS_NEW_PACKET : MELONFRAME_STATUS_CRC_ERROR);

    return MELONFRAME_OK;
}

melonframe_result_t melonframe_decoder_process_byte(melonframe_decoder_t *p, const uint8_t b, melonframe_decoder_event_t *status) {
    switch (p->state) {
        case STATE_SEARCH_HEADER:
            return process_header(p, b, status);
        case STATE_READ_LENGTH:
            return process_length(p, b, status);
        case STATE_READ_PAYLOAD:
            return process_payload(p, b, status);
        case STATE_READ_CRC: {
            const melonframe_result_t err = process_crc(p, b, status);
            if (*status == MELONFRAME_STATUS_NEW_PACKET || *status < 0) {
                p->handler(p->context, *status, p->buffer->data, p->pos);
                melonframe_decoder_reset(p);
            }
            return err;
        }
        default:
            return MELONFRAME_ERR_UNKNOWN;
    }
}
