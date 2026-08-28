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

#include <io.h>
#include <stdint.h>
#include <stdio.h>

#include "../melonframe.h"

static int32_t write_repeated_encoded_packages(const uint16_t package_size, const uint64_t packages_count, const char *filepath) {
    uint8_t test[package_size];
    uint8_t counter = 0;
    for (uint16_t i = 0; i < package_size; i++) {
        test[i] = counter++;
    }

    FILE *f = fopen(filepath, "wb");

    size_t encoded_size = 0;
    const melonframe_result_t res = melonframe_get_size_for_encoded(sizeof(test), &encoded_size);
    if (res != MELONFRAME_OK) {
        printf("melonframe_decoder_process_byte returned %d\n, encoded_size: %llu\n", res, encoded_size);
        fclose(f);
        return -1;
    }

    uint8_t encoded[encoded_size];
    melonframe_encode(test, package_size, encoded, encoded_size);

    for (int i = 0; i < packages_count; i++) {
        fwrite(encoded, encoded_size, 1, f);
    }

    fclose(f);
    return 0;
}

static void handler(void *ctx, melonframe_decoder_event_t status, uint8_t *data, const size_t data_len) {
    if (status == MELONFRAME_STATUS_NEW_PACKET) {
        FILE *f = (FILE*)ctx;
        fwrite(data, sizeof(uint8_t),data_len, f);

        for (int i = 0; i < data_len; i++) {
            //printf("data[%i]: %i\n", i, data[i]);
        }

    } else {
        printf("handler melonframe_decoder_event_t returned %d\n", status);
    }
}

static int32_t decode_and_write_encoded_packages(const char *encoded_filepath, const char *decoded_filepath) {
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
                printf("melonframe_decoder_process_byte returned %d\n", res);
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

static int32_t compare_files(const char *encoded_filepath, const char *decoded_filepath) {
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

    uint8_t enc_buffer[1024];
    uint8_t dec_buffer[1024];

    while (1) {
        const size_t enc = fread(enc_buffer, sizeof(uint8_t), 1024, encoded_file);
        const size_t dec = fread(dec_buffer, sizeof(uint8_t), 1024, decoded_file);
        if (enc != dec) {
            printf("compare_files: %llu != %llu\n", enc, dec);
            fclose(encoded_file);
            fclose(decoded_file);
            return -1;
        }

        if (enc == 0 && dec == 0) {
            printf("compare_files: The files contain the same data.\n");
            fclose(encoded_file);
            fclose(decoded_file);
            return 0;
        }

        const size_t len = enc < dec ? enc : dec;
        for (size_t i = 0; i < len; i++) {
            if (enc_buffer[i] != dec_buffer[i]) {
                printf("compare_files: pos: %llu vals: %i != %i\n", i, enc_buffer[i], dec_buffer[i]);
                fclose(encoded_file);
                fclose(decoded_file);
                return -1;
            }
        }
    }

    return 0;
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

    res = decode_and_write_encoded_packages(encoded_filepath, decoded_filepath);
    if (res != 0) {
        printf("decode_and_write_encoded_packages returned %d\n", res);
        return -1;
    }

    return compare_files(encoded_filepath, decoded_filepath);
}