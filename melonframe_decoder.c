#include "melonframe_decoder.h"
#include <string.h>
#include "melonframe_crc.h"
#include "melonframe_decoder.h"

#include <stdbool.h>

#include "melonframe.h"
#include <stdio.h>
#include <stdlib.h>

void parser_init(stream_parser_t *p, const size_t buffer_size, PacketHandler h, void *ctx) {
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

    if (p->pos == 2) {
        const uint16_t header = (uint16_t)((p->buffer[0] << 8) | p->buffer[1]);

        if (header == PROTO_HEADER_VALUE) {
            p->state = STATE_READ_LENGTH;
        } else {

            printf("desync header: %x\n", header);

            /* resync: slide the window by one byte */
            p->buffer[0] = p->buffer[1];
            p->pos = 1;
        }
    }

    return 0;
}

static int process_length(stream_parser_t *p, const uint8_t b) {
    p->buffer[p->pos++] = b;

    if (p->pos == 4) {
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

    const int payload_size = PROTO_HEADER + PROTO_PAYLOAD_SIZE + p->payload_len;
    if (p->pos < payload_size + 2) {
        return 0;
    }

    const int crc_h = payload_size;
    const int crc_l = payload_size + 1;
    const uint16_t crc = (uint16_t)((p->buffer[crc_h] << 8) | p->buffer[crc_l]);
    const uint16_t calc = crc16(p->buffer, 0, payload_size);

    const int valid = crc == calc ? 1 : -1;
    if (valid) {
        p->handler(p->ctx, p->buffer, p->pos);
    }
    reset(p);
    return valid;
}

int parser_process_byte(stream_parser_t *p, const uint8_t b) {

    printf("%x - %x\n", b, p->pos);

    switch (p->state) {
        case STATE_SEARCH_HEADER:
            return process_header(p, b);
        case STATE_READ_LENGTH:
            return process_length(p, b);
        case STATE_READ_PAYLOAD:
            return process_payload(p, b);
        case STATE_READ_CRC:
            return process_crc(p, b);
        default:
            return -1;
    }
}