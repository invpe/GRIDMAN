#include <stdio.h>
#include <stdlib.h>

// FIX FOR ERRNO
#undef __errno
int * __errno_location(void);
__attribute__((used)) int * __errno = 0;
__attribute__((constructor)) static void init_errno(void) { __errno = __errno_location(); }
int * __errno_location(void) { static int local_errno = 0; return &local_errno; }

int local_main()
{
    printf("DEXBrain Price Impact Task\n");

    FILE* input_file = fopen("/spiffs/payload", "r");
    if (!input_file) {
        printf("Can't open input payload\n");
        return 1;
    }

    char impactStr[64];
    if (!fgets(impactStr, sizeof(impactStr), input_file)) {
        printf("Failed to read impact value\n");
        fclose(input_file);
        return 1;
    }
    fclose(input_file);
 
    float impactValue = atof(impactStr); 
    float impactPct = impactValue * 10000.0f;

    printf("Parsed impact: %f\n", impactValue);
    printf("Scaled impact: %.4f\n", impactPct); 
    FILE* output_file = fopen("/spiffs/payload", "w");
    if (output_file) {
        fprintf(output_file,
            "{ \"task\": \"esp32dex_price_impact\", \"type\": \"impact\", \"impact\": %.4f }",
            impactPct);
        fclose(output_file);
    }

    return 0;
}
