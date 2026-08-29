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

static int test_encoding() {
    const uint8_t arr[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x10};

    size_t encoded_size;
    MelonframeResult err = melonframe_get_size_for_encoded(sizeof(arr), &encoded_size);
    if (err != MELONFRAME_OK) {
        printf("melonframe_get_size_for_encoded: %d", err);
        return -1;
    }

    uint8_t encoded[encoded_size];
    err = melonframe_encode(
        arr,
        sizeof(arr),
        encoded,
        encoded_size);

    if (err != MELONFRAME_OK) {
        printf("melonframe_encode: %d", err);
        return -1;
    }

    printf("simple_encode_test result:\n");
    for (size_t i = 0; i < encoded_size; i++) {
        printf("arr[%zu]:%02X\n", i, encoded[i]);
    }

    const uint8_t expected[] = {
        0xAA, 0x55, 0x00, 0x07, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x10, 0xF6, 0x31
    };

    for (size_t i = 0; i < sizeof(expected); i++) {
        if (encoded[i] != expected[i]) {
            printf("encoded[i] != expected[i]:%02X != %02X\n", encoded[i], expected[i]);
            return -1;
        }
    }

    return 0;
}

int main() {
    return test_encoding();
}