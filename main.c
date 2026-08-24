#include <stdio.h>
#include <stdlib.h>
#include "melonframe_decoder.h"
#include "stdint.h"
#include "melonframe_encoder.h"

void encode_test() {
    const uint8_t arr[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x10};

    size_t encoded_size;
    melonframe_error_t err = melonframe_get_size_for_encoded(sizeof(arr), &encoded_size);
    if (err != MELONFRAME_OK) {
        return;
    }

    uint8_t encoded[encoded_size];
    err = melonframe_encode(
        arr,
        sizeof(arr),
        encoded,
        encoded_size);

    if (err != MELONFRAME_OK) {
        return;
    }

    for (size_t i = 0; i < encoded_size; i++) {
        printf("arr[%zu]: %X\n", i, encoded[i]);
    }
}

void handler(void *ctx, int32_t status, uint8_t *data, size_t data_len) {
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

    melonframe_decoder_t parser;
    melonframe_decoder_init(&parser, 1024, handler, NULL);

    for (size_t i = 0; i < sizeof(encoded); i++) {
        melonframe_decoder_process_byte(&parser, encoded[i]);
    }

    melonframe_decoder_free(&parser);
    printf("\n--- END ---\n");
}

int main(void) {

    encode_test();
    decode_test();

    return 0;
}




