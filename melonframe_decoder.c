#include "melonframe_decoder.h"

#include <stdlib.h>
#include <string.h>

#include "melonframe.h"
#include "melonframe_crc.h"


/* positions */
enum {
    MELONFRAME_POS_BEGINNING_OF_HEADER = 1,
    MELONFRAME_POS_END_OF_HEADER = 2,
    MELONFRAME_POS_END_OF_LEN_FIELD = 4,
};


melonframe_error_t melonframe_decoder_init(
    melonframe_decoder_t *p,
    const size_t buffer_size,
    const data_handler_cb_t data_handler,
    void *ctx) {

    memset(p, 0, sizeof(*p));

    p->buffer = malloc(buffer_size);
    if (!p->buffer) {
        return MELONFRAME_ERR_ALLOC;
    }
    p->buffer_size = buffer_size;
    p->data_handler = data_handler;
    p->ctx = ctx;
    p->state = STATE_SEARCH_HEADER;

    return MELONFRAME_OK;
}

melonframe_error_t melonframe_decoder_free(melonframe_decoder_t *p) {
    free(p->buffer);
    p->buffer = NULL;
    return MELONFRAME_OK;
}

 melonframe_error_t melonframe_decoder_reset(melonframe_decoder_t *p) {
    p->state = STATE_SEARCH_HEADER;
    p->pos = 0;
    return MELONFRAME_OK;
}

static void notify_new_packet(const melonframe_decoder_t *p) {
    if (p->data_handler) {
        p->data_handler(p->ctx, MELONFRAME_STATUS_NEW_PACKET, p->buffer, p->pos);
    }
}

static void notify_error(const melonframe_decoder_t *p, const melonframe_status_t status) {
    if (p->data_handler) {
        p->data_handler(p->ctx, status, NULL, 0);
    }
}

static melonframe_error_t process_header(melonframe_decoder_t *p, const uint8_t b) {
    p->buffer[p->pos++] = b;

    if (p->pos == MELONFRAME_POS_END_OF_HEADER) {
        const uint16_t header = (uint16_t)((p->buffer[0] << 8) | p->buffer[1]);

        if (header == MELONFRAME_PROTO_HEADER_VALUE) {
            p->state = STATE_READ_LENGTH;
        } else {
            /* resync: slide the window by one byte */
            p->buffer[0] = p->buffer[1];
            p->pos = MELONFRAME_POS_BEGINNING_OF_HEADER;
            notify_error(p, MELONFRAME_STATUS_OUT_OF_SYNC);
        }
    }

    return MELONFRAME_OK;
}

static melonframe_error_t process_length(melonframe_decoder_t *p, const uint8_t b) {
    p->buffer[p->pos++] = b;

    if (p->pos == MELONFRAME_POS_END_OF_LEN_FIELD) {
        const uint16_t payload_len = (uint16_t)((p->buffer[2] << 8) | p->buffer[3]);
        p->payload_len = payload_len;

        if (payload_len == 0) {
            p->state = STATE_READ_CRC;
        } else if (payload_len > p->buffer_size) {
            // TODO: handle this more gracefully
            p->state = STATE_SEARCH_HEADER;
        } else {
            p->state = STATE_READ_PAYLOAD;
        }
    }

    return MELONFRAME_OK;
}

static melonframe_error_t process_payload(melonframe_decoder_t *p, const uint8_t b) {
    p->buffer[p->pos++] = b;

    if ((p->pos - MELONFRAME_PROTO_HEADER_SIZE - MELONFRAME_PROTO_PAYLOAD_SIZE) == p->payload_len) {
        p->state = STATE_READ_CRC;
    }

    return MELONFRAME_OK;
}

static melonframe_error_t process_crc(melonframe_decoder_t *p, const uint8_t b, melonframe_status_t *status) {
    p->buffer[p->pos++] = b;

    const uint32_t payload_size = MELONFRAME_PROTO_HEADER_SIZE + MELONFRAME_PROTO_PAYLOAD_SIZE + p->payload_len;
    if (p->pos < payload_size + 2) {
        *status = MELONFRAME_STATUS_BYTE_PROCESSED;
        return MELONFRAME_OK;
    }

    const uint16_t crc =
        (uint16_t)((p->buffer[payload_size] << 8)  // CRC_H
            | p->buffer[payload_size + 1]);        // CRC_L

    const uint16_t calc = melonframe_crc16(p->buffer, 0, payload_size);
    * status = (calc == crc ? MELONFRAME_STATUS_NEW_PACKET : MELONFRAME_STATUS_CRC_ERROR);

    return MELONFRAME_OK;
}

melonframe_error_t melonframe_decoder_process_byte(melonframe_decoder_t *p, const uint8_t b) {
    switch (p->state) {
        case STATE_SEARCH_HEADER:
            return process_header(p, b);
        case STATE_READ_LENGTH:
            return process_length(p, b);
        case STATE_READ_PAYLOAD:
            return process_payload(p, b);
        case STATE_READ_CRC:
            melonframe_status_t status;
            const melonframe_error_t err = process_crc(p, b, &status);

            if (status == MELONFRAME_STATUS_NEW_PACKET) {
                notify_new_packet(p);
                melonframe_decoder_reset(p);
            } else if (status == MELONFRAME_STATUS_CRC_ERROR) {
                notify_error(p, MELONFRAME_STATUS_CRC_ERROR);
                melonframe_decoder_reset(p);
            }

            return err;
        default:
            return MELONFRAME_ERR_UNKNOWN;
    }
}