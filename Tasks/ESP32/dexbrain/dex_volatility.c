
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
float my_sqrtf(float x) {
    if (x <= 0.0f) return 0.0f;

    float guess = x / 2.0f;
    for (int i = 0; i < 10; ++i) {
        guess = 0.5f * (guess + x / guess);
    }
    return guess;
}

int local_main()
{
    printf("DEXBrain Volatility Analyzer task\n");

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

    if (i != REQUIRED_INPUTS) {
        printf("Invalid input size: %d (expected %d)\n", i, REQUIRED_INPUTS);
        return 1;
    }

    // Oblicz średnią
    float sum = 0.0f;
    for (int j = 0; j < REQUIRED_INPUTS; j++) {
        sum += prices[j];
    }
    float mean = sum / REQUIRED_INPUTS;

    // Oblicz wariancję i stddev
    float var = 0.0f;
    for (int j = 0; j < REQUIRED_INPUTS; j++) {
        float diff = prices[j] - mean;
        var += diff * diff;
    }
    var /= REQUIRED_INPUTS;
    float stddev = my_sqrtf(var);

    // Określ poziom zmienności
    const char* level = "HIGH";
    if (stddev <= 0.01f) level = "LOW";
    else if (stddev <= 0.05f) level = "MEDIUM";

    printf("VOLATILITY: %.6f\n", stddev);
    printf("LEVEL: %s\n", level);

    // Zapisz wynik
    FILE* output_file = fopen("/spiffs/payload", "w");
    if (output_file) {
        fprintf(output_file,"{ \"task\": \"esp32dex_volatility_analyzer\", \"type\": \"volatility\", \"volatility\": %.6f, \"level\": \"%s\" }",stddev, level);
        fclose(output_file);
    }

    return 0;
}
