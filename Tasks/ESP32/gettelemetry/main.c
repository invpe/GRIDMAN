/**
* Test wrapper GetTelemetry
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
    if(GetTelemetry("test/uploadtelemetry"))
    {
        FILE *fTele = NULL;
        fTele = fopen("/spiffs/telemetry","r");
        char buffer[64]; 
        fread(buffer, 1, sizeof(buffer), fTele);
        fclose(fTele);
        buffer[64]='\0';
        printf("GOT: %s\n", buffer);

        // Store to payload
        FILE* output_file = fopen("/spiffs/payload", "w");
        if (output_file) {
            fprintf(output_file,"%s",buffer);
            fclose(output_file);
        }

    } 
    return 0;
}
