#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "esp32/rom/sha.h"   // SHA_CTX, ets_sha_enable/init/update/finish/disable, SHA1

#define PAYLOAD_PATH "/spiffs/payload"
#define SHA1_LEN     20
#define SHA1_HEX_LEN (SHA1_LEN * 2)

static void sha1_to_hex(const uint8_t *digest, char *hex_out /* >= 41 */) {
    for (int i = 0; i < SHA1_LEN; ++i) {
        sprintf(hex_out + (i * 2), "%02x", digest[i]);
    }
    hex_out[SHA1_HEX_LEN] = '\0';
}

int local_main(void)
{
    printf("ESP32 SHA1 Task\n");

    FILE *input_file = fopen(PAYLOAD_PATH, "r");
    if (!input_file) {
        printf("Can't open input payload\n");
        return 1;
    }

    char inputBuf[512];
    if (!fgets(inputBuf, sizeof(inputBuf), input_file)) {
        printf("Failed to read input\n");
        fclose(input_file);
        return 1;
    }
    fclose(input_file);

    //  
    size_t len = strlen(inputBuf);
    while (len > 0 && (inputBuf[len - 1] == '\n' || inputBuf[len - 1] == '\r')) {
        inputBuf[--len] = '\0';
    }

    //  
    uint8_t digest[SHA1_LEN];
    SHA_CTX ctx;

    ets_sha_enable();
    ets_sha_init(&ctx);
    ets_sha_update(&ctx, SHA1, (const uint8_t*)inputBuf, (uint32_t)(len * 8));
    ets_sha_finish(&ctx, SHA1, digest);
    ets_sha_disable();

    char hex[SHA1_HEX_LEN + 1];
    sha1_to_hex(digest, hex);

    //  
    FILE *output_file = fopen(PAYLOAD_PATH, "w");
    if (!output_file) {
        printf("Can't open output payload for writing\n");
        return 1;
    }

    fprintf(output_file,"%s",hex);
    fclose(output_file);

    return 0;
}
