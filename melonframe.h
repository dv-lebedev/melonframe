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

#include <stdint.h>

static const size_t PROTO_HEADER = 2;
static const size_t PROTO_PAYLOAD_SIZE = 2;
static const size_t PROTO_CRC = 2;
static const size_t PROTO_MAX_SIZE = 65535;
static const uint16_t PROTO_HEADER_VALUE = 0xAA55;

#endif //MELONFRAME_MELONFRAME_H
