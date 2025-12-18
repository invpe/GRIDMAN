// perceptron_core.h 

#ifndef PERCEPTRON_CORE_H
#define PERCEPTRON_CORE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FEATURES 16
 
typedef struct {
    float weights[MAX_FEATURES];
    float bias;
    int numFeatures;
} NNConfig;

// --- Zamiennik dla expf bez relokacji ---
static float my_expf(float x) {
    if (x < -20.0f) return 0.0f;
    if (x > 20.0f) return 485165000.0f;
    float result = 1.0f;
    float term = 1.0f;
    for (int i = 1; i < 100; i++) {
        term *= x / i;
        result += term;
    }
    return result;
}

// --- Inicjalizacja wag i biasu ---
static void initDefaultConfig(NNConfig* config, int numFeatures) {
    if (numFeatures > MAX_FEATURES) numFeatures = MAX_FEATURES;
    config->numFeatures = numFeatures;
    for (int i = 0; i < numFeatures; i++) config->weights[i] = (float)i/(float)numFeatures;
    config->bias = 0.0f;
}

// --- Predykcja perceptronu ---
static float perceptronPredict(const NNConfig* config, const float* features) {
    float sum = config->bias;
    for (int i = 0; i < config->numFeatures; i++) sum += config->weights[i] * features[i];
    return 1.0f / (1.0f + my_expf(-sum));
}

// --- Trening perceptronu ---
static void perceptronTrain(NNConfig* config, const float* features, float target, float learningRate) {
    float prediction = perceptronPredict(config, features);
    float error = target - prediction;
    for (int i = 0; i < config->numFeatures; i++) {
        config->weights[i] += learningRate * error * features[i];
    }
    config->bias += learningRate * error;
}

// --- Zapis konfiguracji perceptronu do pliku ---
static int saveConfigToFile(const NNConfig* config, const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) return 0;
    fprintf(f, "%d,%.6f", config->numFeatures, config->bias);
    for (int i = 0; i < config->numFeatures; i++) fprintf(f, ",%.6f", config->weights[i]);
    fclose(f);
    return 1;
}

// --- Wczytanie konfiguracji perceptronu z pliku ---
static int loadConfigFromFile(NNConfig* config, const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return 0;

    char buffer[512];
    size_t bytesRead = fread(buffer, 1, sizeof(buffer) - 1, f);
    buffer[bytesRead] = '\0';
    fclose(f);

    char* token = strtok(buffer, ",");
    if (!token) return 0;
    config->numFeatures = atoi(token);
    if (config->numFeatures > MAX_FEATURES) config->numFeatures = MAX_FEATURES;

    token = strtok(NULL, ",");
    config->bias = token ? atof(token) : 0.0f;

    for (int i = 0; i < config->numFeatures; i++) {
        token = strtok(NULL, ",");
        config->weights[i] = token ? atof(token) : 0.0f;
    }

    return 1;
}

#endif // PERCEPTRON_CORE_H
