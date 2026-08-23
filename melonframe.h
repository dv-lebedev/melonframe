#ifndef MELONFRAME_MELONFRAME_H
#define MELONFRAME_MELONFRAME_H

#include <stdint.h>

static const size_t PROTO_HEADER = 2;
static const size_t PROTO_PAYLOAD_SIZE = 2;
static const size_t PROTO_CRC = 2;
static const size_t PROTO_MAX_SIZE = 65535;
static const uint16_t PROTO_HEADER_VALUE = 0xAA55;

#endif //MELONFRAME_MELONFRAME_H
