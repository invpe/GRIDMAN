 
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


// FIX FOR ERRNO -------------------------------------------------------------------
// This should serve as a self-contained fix for resolving the undefined symbols when dynamically loading your ELF binary on ESP32.

// The newlib library defines __errno as a macro. We undefine it here
// to avoid conflicts with our own global symbol definition.
#undef __errno

// Provide a function prototype for __errno_location() to ensure the correct
// return type is known before its first use. This function returns a pointer
// to an errno variable.
int * __errno_location(void);

// Declare and export a global __errno symbol.
// This symbol is marked with __attribute__((used)) so that the linker doesn't
// strip it away during optimization. It satisfies the dynamic ELF loader's
// need for a global __errno symbol.
__attribute__((used))
int * __errno = 0;

// Constructor function that initializes __errno at startup.
// The __attribute__((constructor)) makes sure this function runs before main
// (or local_main) is called.
__attribute__((constructor))
static void init_errno(void) {
    // Assign __errno the pointer returned by __errno_location(),
    // which points to our minimal errno variable.
    __errno = __errno_location();
}

// Minimal implementation of __errno_location().
// This function returns a pointer to a static variable that holds the errno value.
// For a single-threaded context this is sufficient. In a multi-threaded scenario,
// you'd typically need thread-local storage.
int * __errno_location(void) {
    // Static variable to store the errno value. This is the minimal implementation.
    static int local_errno = 0;
    return &local_errno;
}
// FIX FOR ERRNO -------------------------------------------------------------------


// Max inputs (prices) allowed, warrning ! number weights are tied with this! 
#define REQUIRED_INPUTS 10

float fSum = 0;
float my_expf(float x) {
    if (x < -20.0f) return 0.0f;      // underflow
    if (x > 20.0f) return 485165000.0f; // overflow clamp

    float result = 1.0f;
    float term = 1.0f;
    for (int i = 1; i < 100; ++i) {
        term *= x / i;
        result += term;
    }
    return result;
}

float sigmoid(float x) {
    return 1.0f / (1.0f + my_expf(-x));
}
float predict(float *inputs) {
    float weights[REQUIRED_INPUTS] = { 0.12, -0.04, 0.10, 0.07, -0.09, 0.06, 0.11, -0.05, 0.08, 0.13 };
    float bias = -0.15;

    // --- KROK 1: Znajdź min i max ---
    float min = inputs[0];
    float max = inputs[0];
    for (int i = 1; i < REQUIRED_INPUTS; i++) {
        if (inputs[i] < min) min = inputs[i];
        if (inputs[i] > max) max = inputs[i];
    }

    float range = max - min;
    if (range == 0.0f) range = 1.0f; // Zabezpieczenie: unikaj dzielenia przez 0

    // --- KROK 2: Normalizuj dane ---
    float norm[REQUIRED_INPUTS];
    for (int i = 0; i < REQUIRED_INPUTS; i++) {
        norm[i] = (inputs[i] - min) / range;
    }

    // --- KROK 3: Oblicz sumę perceptronu ---
    float sum = bias;
    for (int i = 0; i < REQUIRED_INPUTS; i++) {
        sum += norm[i] * weights[i];
    }

    fSum = sum;
    return sigmoid(sum);
}

 
int local_main()
{
    printf("DEXBrain Trend Predictor task\n"); 
    FILE* input_file = fopen("/spiffs/payload", "r");

    if (!input_file) {
        printf("Can't open input payload\n");
        return 1;
    }
 
    float prices[REQUIRED_INPUTS];    

    int i = 0;
    char line[64];
    
    while (fgets(line, sizeof(line), input_file) && i < REQUIRED_INPUTS) 
    { 
        long epoch;
        float price;

        if (sscanf(line, "%ld,%f", &epoch, &price) == 2) 
        {
            prices[i] = price;
            i++;

        } else {
            printf("Invalid line skipped: %s", line);
        }
    }

    fclose(input_file); 

    if (i != REQUIRED_INPUTS) {
        printf("Invalid input size: %d (expected %d)\n", i, REQUIRED_INPUTS);
        return 1;
    } 
  
    float result = predict(prices); 
    const char* prediction = (result > 0.5f) ? "UP" : "DOWN";  
 
    FILE* output_file = fopen("/spiffs/payload", "w");
    if (output_file) {
        fprintf(output_file,"{ \"task\": \"esp32dex_trendpredictor\", \"type\": \"predictor\", \"prediction\": \"%s\", \"confidence\": %.4f, \"sum\": %.6f }",prediction, result, fSum);
        fclose(output_file);
    }

 
    return 0;
}

