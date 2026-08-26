//
// Created by dvleb on 8/26/2026.
//

#include "encoding.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void print_hex_arr(const uint8_t *arr, const size_t arr_size) {
    for (size_t i = 0; i < arr_size; i++) {
        printf("%02X ", arr[i]);
    }
    printf("\n");
}

static void increment_counter(uint8_t *counter, const size_t counter_size) {
    for (int64_t i = (int64_t)counter_size - 1; i >= 0; i--) {
        if (counter[i] != 0xFF) {
            counter[i]++;
            return;
        }
        counter[i] = 0x00;
    }
}


void __encoding_test(const size_t counter_size, const uint32_t iterations) {

    uint8_t test[counter_size];
    memset(test, 0, counter_size);

    for (uint32_t i = 0; i < iterations; i++) {
        increment_counter(test, counter_size);
        print_hex_arr(test, counter_size);
    }
}

