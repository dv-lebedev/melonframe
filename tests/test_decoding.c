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

#include <stdint.h>
#include <stdio.h>

#include "../melonframe.h"

static int32_t write_repeated_encoded_packages(
    const uint16_t package_size,
    const uint64_t packages_count,
    const char *filepath) {

    uint8_t test[package_size];
    uint8_t counter = 0;
    for (uint16_t i = 0; i < package_size; i++) {
        test[i] = counter++;
    }

    FILE *f = fopen(filepath, "wb");

    size_t encoded_size = 0;
    const MelonframeResult res = melonframe_get_size_for_encoded(sizeof(test), &encoded_size);
    if (res != MELONFRAME_OK) {
        printf("melonframe_decoder_process_byte returned %s\n, encoded_size: %llu\n",
            melonframe_result_to_string(res), encoded_size);
        fclose(f);
        return -1;
    }

    uint8_t encoded[encoded_size];
    melonframe_encode(test, package_size, encoded, encoded_size);

    for (uint64_t i = 0; i < packages_count; i++) {
        fwrite(encoded, encoded_size, 1, f);
    }

    fclose(f);
    return 0;
}

static void handler(void *ctx, enum MelonframeStatus status, uint8_t *data, const size_t data_len) {
    if (status == MELONFRAME_STATUS_NEW_PACKET) {
        FILE *f = (FILE*)ctx;
        fwrite(data, sizeof(uint8_t),data_len, f);
    } else if (status < 0) {
        if (status == MELONFRAME_STATUS_OUT_OF_SYNC) {
            printf("decoder handler MELONFRAME_STATUS_OUT_OF_SYNC: %d\n", data[0]);
        } else {
            printf("decoder handler returned: %s\n", melonframe_status_to_string(status));
        }
    }
}

static int32_t decode_and_write_encoded_packages(
    const char *encoded_filepath,
    const char *decoded_filepath,
    const size_t decoder_buffer_size,
    const size_t read_buffer_size) {

    FILE *encoded_file = fopen(encoded_filepath, "rb");
    if (encoded_file == NULL) {
        printf("encoded_file == NULL");
        return -1;
    }

    FILE *decoded_file = fopen(decoded_filepath, "wb");
    if (decoded_file == NULL) {
        fclose(encoded_file);
        printf("decoded_file == NULL");
        return -1;
    }

    struct MelonframeDecoder decoder;
    struct MelonframeBuffer decoder_buffer = {
        .data = malloc(sizeof(uint8_t) * decoder_buffer_size),
        .size = decoder_buffer_size,
    };
    enum MelonframeStatus decoder_event = MELONFRAME_STATUS_NONE;
    melonframe_decoder_init(&decoder, &decoder_buffer, handler, (void*)decoded_file);

    uint8_t buffer[read_buffer_size];
    while (1) {
        const size_t read = fread(buffer, sizeof(uint8_t), read_buffer_size, encoded_file);
        if (read == 0) {
            break;
        }

        for (int i = 0; i < read; i++) {
            const MelonframeResult res = melonframe_decoder_process_byte(&decoder, buffer[i], &decoder_event);
            if (res != MELONFRAME_OK) {
                printf("melonframe_decoder_process_byte returned %s\n", melonframe_result_to_string(res));
                fclose(decoded_file);
                fclose(encoded_file);
                free(decoder_buffer.data);
                return -1;
            }
        }
    }

    fclose(decoded_file);
    fclose(encoded_file);
    free(decoder_buffer.data);
    return 0;
}

static int32_t compare_files(
    const char *encoded_filepath,
    const char *decoded_filepath,
    const size_t encoded_buffer_size,
    const size_t decoded_buffer_size) {

    FILE *encoded_file = fopen(encoded_filepath, "rb");
    if (encoded_file == NULL) {
        printf("encoded_file == NULL");
        return -1;
    }
    FILE *decoded_file = fopen(decoded_filepath, "rb");
    if (decoded_file == NULL) {
        fclose(encoded_file);
        printf("decoded_file == NULL");
        return -1;
    }

    uint8_t enc_buffer[encoded_buffer_size];
    uint8_t dec_buffer[decoded_buffer_size];

    int32_t result = 0;
    while (1) {
        const size_t enc = fread(enc_buffer, sizeof(uint8_t), encoded_buffer_size, encoded_file);
        const size_t dec = fread(dec_buffer, sizeof(uint8_t), decoded_buffer_size, decoded_file);
        if (enc != dec) {
            printf("compare_files: %llu != %llu\n", enc, dec);
            result = -1;
            break;
        }

        if (enc == 0 && dec == 0) {
            printf("compare_files: The files contain the same data.\n");
            break;
        }

        const size_t len = enc < dec ? enc : dec;
        size_t i;
        for (i = 0; i < len; i++) {
            if (enc_buffer[i] != dec_buffer[i]) {
                printf("compare_files: pos: %llu vals: %i != %i\n", i, enc_buffer[i], dec_buffer[i]);
                break;
            }
        }
        if (i < len) {
            result = -1;
            break;
        }
    }

    fclose(encoded_file);
    fclose(decoded_file);
    return result;
}

int main() {

    const char *encoded_filepath = "encoded.dat";
    const char *decoded_filepath = "decoded.dat";

    const uint32_t min_pkg_size = 6;
    int32_t res = write_repeated_encoded_packages(16 - min_pkg_size, 0xFFFF, encoded_filepath);
    if (res != 0) {
        printf("write_repeated_encoded_packages returned %d\n", res);
        return -1;
    }

    res = decode_and_write_encoded_packages(encoded_filepath, decoded_filepath, 1024, 1024);
    if (res != 0) {
        printf("decode_and_write_encoded_packages returned %d\n", res);
        return -1;
    }

    return compare_files(encoded_filepath, decoded_filepath, 1024, 1024);
}