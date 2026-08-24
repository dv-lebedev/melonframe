/*
* Packet format (all multi-byte fields big-endian / network byte order):
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
 * Total packet size = PROTO_HEADER + PROTO_SIZE + bytes_size + PROTO_CRC,
 * and must not exceed PROTO_MAX_SIZE (65535) since SIZE is a 16-bit field.
 */

#ifndef MELONFRAME_MELONFRAME_H
#define MELONFRAME_MELONFRAME_H

enum {
    MELONFRAME_PROTO_HEADER_SIZE = 2,
    MELONFRAME_PROTO_PAYLOAD_SIZE = 2,
    MELONFRAME_PROTO_CRC_SIZE = 2,
    MELONFRAME_PROTO_FRAME_MAX_SIZE = 65535,
    MELONFRAME_PROTO_HEADER_VALUE = 0xAA55,
};

typedef enum {
    MELONFRAME_STATUS_BYTE_PROCESSED = 0,
    MELONFRAME_STATUS_NEW_PACKET = 1,
    MELONFRAME_STATUS_CRC_ERROR = -1,
    MELONFRAME_STATUS_OUT_OF_SYNC = -2,
} melonframe_status_t;

typedef enum {
    MELONFRAME_OK = 0,
    MELONFRAME_ERR_UNKNOWN = -1,
    MELONFRAME_ERR_ALLOC = -2,
    MELONFRAME_ERR_NULL_ARG = -3,
    MELONFRAME_ERR_BUFFER_OVERFLOW = -4,

} melonframe_error_t;


#endif //MELONFRAME_MELONFRAME_H
