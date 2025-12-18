/**
* Test wrapper UploadTelemetry
 */

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
    // Zapisz wynik jako JSON
    FILE* output_file = fopen("/spiffs/payload", "w");
    if (output_file) {
        fprintf(output_file,"testowy payload\n");
        fclose(output_file);
    }

    UploadTelemetry("/spiffs/payload","test/uploadtelemetry",1);

    return 0;
}
