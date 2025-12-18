
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// FIX FOR ERRNO -------------------------------------------------------------------
#undef __errno
int * __errno_location(void);
__attribute__((used)) int * __errno = 0;
__attribute__((constructor)) static void init_errno(void) { __errno = __errno_location(); }
int * __errno_location(void) { static int local_errno = 0; return &local_errno; }
// FIX FOR ERRNO -------------------------------------------------------------------

#define REQUIRED_INPUTS 10

int local_main()
{
    printf("DEXBrain Impulse Detector task\n");

    FILE* input_file = fopen("/spiffs/payload", "r");
    if (!input_file) {
        printf("Can't open input payload\n");
        return 1;
    }

    float prices[REQUIRED_INPUTS];
    int i = 0;
    char line[64];

    while (fgets(line, sizeof(line), input_file) && i < REQUIRED_INPUTS) {
        long epoch;
        float price;
        if (sscanf(line, "%ld,%f", &epoch, &price) == 2) {
            prices[i++] = price;
        } else {
            printf("Invalid line skipped: %s", line);
        }
    }
    fclose(input_file);

    if (i < 2) {
        printf("Not enough data to calculate impulse\n");
        return 1;
    }

    float prev = prices[REQUIRED_INPUTS - 2];
    float last = prices[REQUIRED_INPUTS - 1];
    float delta = last - prev;
    float impulse = (delta >= 0) ? delta : -delta;

    const char* direction = "FLAT";
    if (delta > 0.0001f) direction = "UP";
    else if (delta < -0.0001f) direction = "DOWN";

    printf("Impulse: %.6f\n", impulse);
    printf("Direction: %s\n", direction);
 
    FILE* output_file = fopen("/spiffs/payload", "w");
    if (output_file) {
        fprintf(output_file,
            "{ \"task\": \"esp32dex_impulse_detector\", \"type\": \"impulse\", \"impulse\": %.6f, \"direction\": \"%s\" }",
            impulse, direction);
        fclose(output_file);
    }

    return 0;
}
