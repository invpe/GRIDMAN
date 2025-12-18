 

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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
    printf("DEXBrain RSI Predictor task\n");

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

    if (i < REQUIRED_INPUTS) {
        printf("Invalid input size: %d (expected %d)\n", i, REQUIRED_INPUTS);
        return 1;
    }

    // RSI calculation (simplified)
    float gains = 0.0f;
    float losses = 0.0f;
    for (int j = 1; j < REQUIRED_INPUTS; j++) {
        float diff = prices[j] - prices[j - 1];
        if (diff > 0) gains += diff;
        else losses -= diff;
    }

    float avg_gain = gains / (REQUIRED_INPUTS - 1);
    float avg_loss = losses / (REQUIRED_INPUTS - 1);

    float rsi = 50.0f;
    if (avg_loss > 0.0f) {
        float rs = avg_gain / avg_loss;
        rsi = 100.0f - (100.0f / (1.0f + rs));
    } else if (avg_gain > 0.0f) {
        rsi = 100.0f;
    }

    const char* zone = "NEUTRAL";
    if (rsi < 30.0f) zone = "OVERSOLD";
    else if (rsi > 70.0f) zone = "OVERBOUGHT";

    printf("RSI: %.2f\n", rsi);
    printf("ZONE: %s\n", zone);

    // Zapisz wynik jako JSON
    FILE* output_file = fopen("/spiffs/payload", "w");
    if (output_file) {
        fprintf(output_file,
            "{ \"task\": \"esp32dex_rsi_predictor\", \"type\": \"rsi\", \"rsi\": %.2f, \"zone\": \"%s\" }",
            rsi, zone);
        fclose(output_file);
    }

    return 0;
}
