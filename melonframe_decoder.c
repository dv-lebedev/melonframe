#include "melonframe_decoder.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "melonframe.h"
#include "melonframe_crc.h"


/* positions */
static const uint32_t BEGINNING_OF_HEADER = 1;
static const uint32_t END_OF_HEADER = 2;
static const uint32_t END_OF_LEN_FIELD = 4;

#define END_OF_PAYLOAD(p) (PROTO_HEADER + PROTO_PAYLOAD_SIZE + p->payload_len)


void parser_init(stream_parser_t *p, const size_t buffer_size, const PacketHandler h, void *ctx) {
    memset(p, 0, sizeof(*p));
    p->buffer = malloc(buffer_size);
    p->state = STATE_SEARCH_HEADER;
    p->handler = h;
    p->ctx = ctx;
}

void parser_free(stream_parser_t *p) {
    free(p->buffer);
    p->buffer = NULL;

    free(p);
    p = NULL;
}

 void reset(stream_parser_t *p) {
    p->state = STATE_SEARCH_HEADER;
    p->pos = 0;
}

static int process_header(stream_parser_t *p, const uint8_t b) {
    p->buffer[p->pos++] = b;

    if (p->pos == END_OF_HEADER) {
        const uint16_t header = (uint16_t)((p->buffer[0] << 8) | p->buffer[1]);

        if (header == PROTO_HEADER_VALUE) {
            p->state = STATE_READ_LENGTH;
        } else {
            /* resync: slide the window by one byte */
            p->buffer[0] = p->buffer[1];
            p->pos = BEGINNING_OF_HEADER;
        }
    }

    return 0;
}

static int process_length(stream_parser_t *p, const uint8_t b) {
    p->buffer[p->pos++] = b;

    if (p->pos == END_OF_LEN_FIELD) {
        const uint16_t payload_len = (uint16_t)((p->buffer[2] << 8) | p->buffer[3]);
        p->payload_len = payload_len;

        if (payload_len == 0) {
            p->state = STATE_READ_CRC;
        } else {
            p->state = STATE_READ_PAYLOAD;
        }
    }

    return 0;
}

static int process_payload(stream_parser_t *p, const uint8_t b) {
    p->buffer[p->pos++] = b;

    if ((p->pos - PROTO_HEADER - PROTO_PAYLOAD_SIZE) == p->payload_len) {
        p->state = STATE_READ_CRC;
    }

    return 0;
}

static int process_crc(stream_parser_t *p, const uint8_t b) {
    p->buffer[p->pos++] = b;

    const int payload_size = END_OF_PAYLOAD(p);
    if (p->pos < payload_size + 2) {
        return 0;
    }

    const uint16_t crc =
        (uint16_t)((p->buffer[payload_size] << 8)  // CRC_H
            | p->buffer[payload_size + 1]);        // CRC_L

    const uint16_t calc = crc16(p->buffer, 0, payload_size);

    return crc == calc ? 1 : -1;
}

int parser_process_byte(stream_parser_t *p, const uint8_t b) {
    switch (p->state) {
        case STATE_SEARCH_HEADER:
            return process_header(p, b);
        case STATE_READ_LENGTH:
            return process_length(p, b);
        case STATE_READ_PAYLOAD:
            return process_payload(p, b);
        case STATE_READ_CRC:
            const int is_valid = process_crc(p, b);
            if (is_valid) {
                p->handler(p->ctx, p->buffer, p->pos);
                reset(p);
            };
            return is_valid;
        default:
            return -1;
    }
}