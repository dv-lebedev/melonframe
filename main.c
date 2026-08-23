#include <stdio.h>
#include <stdlib.h>
#include "melonframe_decoder.h"
#include "stdint.h"
#include "melonframe_encoder.h"

void encode_test() {
    const uint8_t arr[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x10};
    uint8_t encoded[get_size_for_encoded(sizeof(arr))];
    const size_t encoded_size = encode(
        arr,
        0,
        sizeof(arr),
        encoded,
        0,
        sizeof(encoded));


    //printf("encoded size: %llu\n", encoded_size);

    for (size_t i = 0; i < encoded_size; i++) {
        printf("arr[%zu]: %X\n", i, encoded[i]);
    }
}

void handler(void *ctx, uint8_t *data, size_t data_len) {
    printf("Handler started:\n");
    for (size_t j = 0; j < data_len; j++) {
        printf("%x\n", data[j]);
    }
    printf("Handler finished.\n");
}

void decode_test() {

    const uint8_t encoded[] = {
        0xAA,
        0x55,
        0x00,
        0x07,
        0x01,
        0x02,
        0x03,
        0x04,
        0x05,
        0x06,
        0x10,
        0xF6,
        0x31,

                0xFF,
                0xDD,
                0xCC,


        0xAA,
        0x55,
        0x00,
        0x07,
        0x01,
        0x02,
        0x03,
        0x04,
        0x05,
        0x06,
        0x10,
        0xF6,
        0x31
            };

    stream_parser_t parser;
    parser_init(&parser, 1024, handler, NULL, NULL);

    for (size_t i = 0; i < sizeof(encoded); i++) {
        parser_process_byte(&parser, encoded[i]);
    }

    printf("\n--- END ---\n");
}

int main(void) {

    encode_test();
    decode_test();

    return 0;
}




