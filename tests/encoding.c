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


#include "encoding.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../melonframe.h"

void simple_encode_test() {
    const uint8_t arr[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x10};

    size_t encoded_size;
    melonframe_result_t err = melonframe_get_size_for_encoded(sizeof(arr), &encoded_size);
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

    printf("simple_encode_test result:\n");
    for (size_t i = 0; i < encoded_size; i++) {
        printf("arr[%zu]:%02X\n", i, encoded[i]);
    }

    const uint8_t expected[] = {
        0xAA, 0x55, 0x00, 0x07, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x10, 0xF6, 0x31
    };

    for (size_t i = 0; i < sizeof(expected); i++) {
        assert(encoded[i] == expected[i]);
    }
}

void write_repeated_encoded_packages(const uint16_t package_size, const uint64_t packages_count, const char *filepath) {
    uint8_t test[package_size];
    uint8_t counter = 0;
    for (uint16_t i = 0; i < package_size; i++) {
        test[i] = counter++;
    }

    FILE *f = fopen(filepath, "wb");

    size_t encoded_size = 0;
    const melonframe_result_t res = melonframe_get_size_for_encoded(sizeof(test), &encoded_size);
    if (res != MELONFRAME_OK) {
        assert(res == MELONFRAME_OK);
    }

    uint8_t encoded[encoded_size];
    melonframe_encode(test, package_size, encoded, encoded_size);

    for (int i = 0; i < packages_count; i++) {
        fwrite(encoded, encoded_size, 1, f);
    }

    fclose(f);
}

static void handler(void *ctx, melonframe_decoder_event_t status, uint8_t *data, const size_t data_len) {
    if (status == MELONFRAME_STATUS_NEW_PACKET) {
        FILE *f = (FILE*)ctx;
        fwrite(data, data_len, 1, f);
    } else {
        assert(status == MELONFRAME_STATUS_NEW_PACKET);
    }
}

void decode_and_write_encoded_packages(const char *encoded_filepath, const char *decoded_filepath) {
    FILE *encoded_file = fopen(encoded_filepath, "rb");
    assert(encoded_file != NULL);

    FILE *decoded_file = fopen(decoded_filepath, "wb");
    assert(decoded_file != NULL);

    melonframe_decoder_t decoder;
    melonframe_buffer_t decoder_buffer = {
        .data = malloc(sizeof(uint8_t) * 1024),
        .size = 1024,
    };
    melonframe_decoder_event_t decoder_event;
    melonframe_decoder_init(&decoder, &decoder_buffer, handler, (void*)decoded_file);

    uint8_t buffer[1024];
    while (1) {
        const size_t read = fread(buffer, sizeof(uint8_t), 6, encoded_file);
        if (read == 0) {
            break;
        }

        for (int i = 0; i < read; i++) {
            const melonframe_result_t res = melonframe_decoder_process_byte(&decoder, buffer[i], &decoder_event);
            if (res != MELONFRAME_OK) {
                assert(res == MELONFRAME_OK);
            }
        }
    }

    fclose(decoded_file);
    fclose(encoded_file);
}