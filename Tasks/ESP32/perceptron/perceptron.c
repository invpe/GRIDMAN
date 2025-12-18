#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "perceptron_core.h"
 

int local_main() {
    printf("Neural Updater (Perceptron) Task\n"); 

    FILE* input_file = fopen("/spiffs/payload", "r");
    if (!input_file) {
        printf("Can't open input payload\n");
        return 1;
    }

    char perceptronConfigPath[128];
    int numFeatures = 0;
    if (fscanf(input_file, "%127[^,],%d", perceptronConfigPath, &numFeatures) != 2) {
        printf("Failed to read telemetry path and numFeatures\n");
        fclose(input_file);
        return 1;
    }

    if (numFeatures > MAX_FEATURES) numFeatures = MAX_FEATURES;
    float features[MAX_FEATURES] = {0.0f};
    for (int i = 0; i < numFeatures; i++) {
        if (fscanf(input_file, ",%f", &features[i]) != 1) {
            printf("Failed to read feature %d\n", i);
            fclose(input_file);
            return 1;
        }
    }

    float fTarget = 0.0f;
    float fLearningRate = 0.0f;
    int iIterations = 0;

    if (fscanf(input_file, ",%f", &fTarget) != 1 ||
        fscanf(input_file, ",%f", &fLearningRate) != 1 ||
        fscanf(input_file, ",%d", &iIterations) != 1) {
        printf("Failed to read target or learning parameters\n");
        fclose(input_file);
        return 1;
    }

    fclose(input_file);

    printf("Telemetry: %s\n", perceptronConfigPath);
    printf("Target: %f\n", fTarget);
    printf("Features: %d\n", numFeatures);
    for (int i = 0; i < numFeatures; i++) {
        printf("Feature %d: %.4f\n", i, features[i]);
    }

    NNConfig config;
    config.numFeatures = numFeatures;

    // Load perceptron config from the telemetry system
    printf("Loading perceptron config from %s\n", perceptronConfigPath);
    if (!GetTelemetry(perceptronConfigPath)) {
        initDefaultConfig(&config, config.numFeatures);
     
    }
    else
    {
        if(!loadConfigFromFile(&config, "/spiffs/telemetry"))
            initDefaultConfig(&config, config.numFeatures);
    } 

    //
    printf("Training perceptron tgt=%f\n", fTarget); 
    for (int iter = 0; iter < iIterations; iter++) {
        perceptronTrain(&config, features, fTarget, fLearningRate);
    }

    //
    printf("Storing perceptron configuration to telemetry\n");
    saveConfigToFile(&config, "/spiffs/config");
    UploadTelemetry("/config", perceptronConfigPath, 0);


    float finalPrediction = perceptronPredict(&config, features);
    FILE* output_file = fopen("/spiffs/payload", "w");
    if (output_file) {
        fprintf(output_file, "{\"task\":\"esp32dex_perceptron\",\"type\":\"ml_perceptron\",\"prediction\": %.4f}", finalPrediction);
        fclose(output_file);
    }

    printf("Neural updater finished.\n");
    return 0;
}
