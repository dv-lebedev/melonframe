#ifndef MELONFRAME_MELONFRAME_DECODER_H
#define MELONFRAME_MELONFRAME_DECODER_H

#include <stdint.h>

typedef enum {
    STATE_SEARCH_HEADER,
    STATE_READ_LENGTH,
    STATE_READ_PAYLOAD,
    STATE_READ_CRC
} parser_state_t;

typedef void (*PacketHandler)(void *ctx, uint8_t *data, size_t data_len);

typedef void (*ErrorHandler)(void *ctx, int32_t error_code);

typedef struct {
    parser_state_t state;
    uint8_t *buffer;
    size_t buffer_size;
    size_t payload_len;
    uint32_t pos;
    PacketHandler packet_handler;
    ErrorHandler error_handler;
    void *ctx;
} stream_parser_t;

/* Initialize / reset parser state. */
int32_t parser_init(
    stream_parser_t *p,
    size_t buffer_size,
    PacketHandler packet_handler,
    ErrorHandler error_handler,
    void *ctx);

void parser_free(stream_parser_t *p);

/*
 * Feed one byte into the parser.
 * Returns:
 *   1  -> a complete, CRC-valid packet is ready in *out (out->payload is
 *         heap-allocated; caller must free it).
 *   0  -> byte consumed, no complete packet yet.
 *  -1  -> a complete packet was read but CRC check failed (parser resets;
 *         no allocation left dangling).
 */
int32_t parser_process_byte(stream_parser_t *p, uint8_t b);

void reset(stream_parser_t *p);


#endif //MELONFRAME_MELONFRAME_DECODER_H
