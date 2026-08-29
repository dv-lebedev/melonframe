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


/*
 *   Packet format (all multi-byte fields big-endian / network byte order):
 *
 *   +--------+--------+--------+--------+------------------+--------+--------+
 *   |  0xAA  |  0x55  | SIZE_H | SIZE_L | DATA (N bytes)   | CRC_H  | CRC_L  |
 *   +--------+--------+--------+--------+------------------+--------+--------+
 *
 *   - Header (2 bytes): fixed magic 0xAA 0x55.
 *   - Size   (2 bytes): length of DATA.
 *   - Data   (N bytes): the caller-supplied payload, N = bytes_size.
 *   - CRC    (2 bytes): CRC-16 computed over [Header | Size | Data].
 *
 *   Total packet size = PROTO_HEADER + PROTO_SIZE + bytes_size + PROTO_CRC,
 *   and must not exceed PROTO_MAX_SIZE (65535) since SIZE is a 16-bit field.
 */

#ifndef INCLUDE_MELONFRAME_H
#define INCLUDE_MELONFRAME_H

#include <stdlib.h>
#include <stdint.h>

#if defined(_WIN32) || defined(__CYGWIN__)
    #define MELONFRAME_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
    #define MELONFRAME_API __attribute__((visibility("default")))
#else
    #define MELONFRAME_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum MelonframeProto {
    MELONFRAME_PROTO_HEADER_SIZE    = 2,
    MELONFRAME_PROTO_PAYLOAD_SIZE   = 2,
    MELONFRAME_PROTO_CRC_SIZE       = 2,
    MELONFRAME_PROTO_FRAME_MAX_SIZE = 65535,
    MELONFRAME_PROTO_HEADER_VALUE   = 0xAA55,
};

enum MelonframeStatus {
    MELONFRAME_STATUS_PROCESSING_CRC     = 5,
    MELONFRAME_STATUS_PROCESSING_PAYLOAD = 4,
    MELONFRAME_STATUS_PROCESSING_LENGTH  = 3,
    MELONFRAME_STATUS_PROCESSING_HEADER  = 2,
    MELONFRAME_STATUS_NEW_PACKET         = 1,
    MELONFRAME_STATUS_NONE               = 0,
    MELONFRAME_STATUS_CRC_ERROR          = -1,
    MELONFRAME_STATUS_OUT_OF_SYNC        = -2,
    MELONFRAME_STATUS_PAYLOAD_TOO_LARGE  = -3,
};

MELONFRAME_API const char *melonframe_status_to_string(enum MelonframeStatus status);

typedef enum MelonframeResult {
    MELONFRAME_OK                            = 0,
    MELONFRAME_ERR_UNKNOWN                   = -1,
    MELONFRAME_ERR_ALLOC                     = -2,
    MELONFRAME_ERR_NULL_ARG                  = -3,
    MELONFRAME_ERR_BUFFER_OVERFLOW           = -4,
    MELONFRAME_ERR_NULL_POINTER              = -5,
    MELONFRAME_ERR_BUFFER_IS_NOT_INITIALIZED = -6,
} MelonframeResult;

MELONFRAME_API const char *melonframe_result_to_string(MelonframeResult result);

typedef void (*melonframe_decoder_event_handler_t)(
    void *ctx,
    enum MelonframeStatus status,
    uint8_t *data,
    size_t data_len);

MELONFRAME_API MelonframeResult melonframe_get_size_for_encoded(size_t buffer_size, size_t *encoded_size);

MELONFRAME_API MelonframeResult melonframe_encode(
    const uint8_t *bytes,
    size_t bytes_size,
    uint8_t *encoded,
    size_t encoded_size);

typedef enum MelonframeDecoderState {
    MELONFRAME_DECODER_STATE_SEARCH_HEADER,
    MELONFRAME_DECODER_STATE_READ_LENGTH,
    MELONFRAME_DECODER_STATE_READ_PAYLOAD,
    MELONFRAME_DECODER_STATE_READ_CRC
} MelonframeDecoderState;

struct MelonframeBuffer {
    uint8_t *data;
    size_t size;
};

struct MelonframeDecoder {
    MelonframeDecoderState state;
    struct MelonframeBuffer *buffer;
    size_t payload_len;
    uint32_t pos;
    melonframe_decoder_event_handler_t handler;
    void *context;
};

MELONFRAME_API MelonframeResult melonframe_decoder_init(
    struct MelonframeDecoder *p,
    struct MelonframeBuffer *buffer,
    melonframe_decoder_event_handler_t handler,
    void *context);

MELONFRAME_API MelonframeResult melonframe_decoder_free(struct MelonframeDecoder *p);

MELONFRAME_API MelonframeResult melonframe_decoder_reset(struct MelonframeDecoder *p);

MELONFRAME_API MelonframeResult melonframe_decoder_process_byte(
    struct MelonframeDecoder *p,
    uint8_t b,
    enum MelonframeStatus *status);

#ifdef __cplusplus
}
#endif

#endif // INCLUDE_MELONFRAME_H