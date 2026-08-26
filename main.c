#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "stdint.h"
#include <unistd.h>

#include "tests/encoding.h"
#include "melonframe.h"

void encode_test() {
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

    for (size_t i = 0; i < encoded_size; i++) {
        printf("arr[%zu]: %X\n", i, encoded[i]);
    }
}

void encode_counter(uint32_t start, uint32_t end) {

}

static void handler(void *ctx, melonframe_decoder_event_t status, uint8_t *data, size_t data_len) {
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
    melonframe_buffer_t buffer = {
        .data = malloc(1024),
        .size = 1024,
    };
    melonframe_decoder_init(&parser, &buffer, handler, NULL);

    melonframe_decoder_event_t status;
    for (size_t i = 0; i < sizeof(encoded); i++) {
        melonframe_decoder_process_byte(&parser, encoded[i], &status);
        if (status == MELONFRAME_STATUS_NEW_PACKET) {
            handler(NULL, status, buffer.data, parser.pos);
        }
    }

    melonframe_decoder_free(&parser);
    printf("\n--- END ---\n");
}

int main(void) {

    __encoding_test(3, 65537);

    return 0;

    encode_test();
    decode_test();

    return 0;
}


typedef enum {
    MELONFRAME_MODE_UNKNOWN = 0,
    MELONFRAME_MODE_ENCODE = 1,
    MELONFRAME_MODE_DECODE = 2,
} melonframe_mode_t;


int test(int argc, char **argv)
{
    melonframe_mode_t mode = 0;
    char *filePath;
    uint8_t counter_enabled = 0;
    const uint8_t template[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x10};

    int option;
    while ((option = getopt(argc, argv, "m:f:c:t:")) != -1) {
        switch (option) {
            case 'm':
                if (strcmp(optarg, "encode") == 0)
                    mode = MELONFRAME_MODE_ENCODE;
                else if (strcmp(optarg, "decode") == 0)
                    mode = MELONFRAME_MODE_DECODE;
                break;
            case 'f':
                if (optarg != NULL) {
                    filePath = malloc(strlen(optarg) + 1);
                    if (filePath != NULL) {
                        strcpy(filePath, optarg);
                        printf("filePath: %s\n", filePath);
                    }
                }
                break;
            case 'c':
                counter_enabled = 1;
                printf("counter: %s\n", optarg);
                break;
            case 't':
                printf("template: %s\n", optarg);
            case '?':
                exit(EXIT_FAILURE);
                break;
            default:
                exit(EXIT_FAILURE);
        }
    }

    if (mode == MELONFRAME_MODE_UNKNOWN) {
        printf("err: mode is not selected\n");
        return 0;
    }

    if (mode == MELONFRAME_MODE_ENCODE) {
        FILE *f = fopen(filePath, "wb");
        fwrite(template, sizeof(template), 1, f);
        fclose(f);
    }

    /* Print remaining arguments. */
    for (; optind < argc; optind++)
        printf("%s\n", argv[optind]);
    return 0;
}




