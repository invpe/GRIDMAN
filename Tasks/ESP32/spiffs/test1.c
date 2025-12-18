#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>   
#include <sys/unistd.h>
#include <sys/stat.h>

char* local_main(const char* arg,size_t len) {
     
    printf("Welcome to task\n");
    printf("Inputpayload: %s\n", arg);  
    FILE *f = fopen("/spiffs/hello.txt","r");
    if(f != NULL)
    {
        char line[64];
        fgets(line, sizeof(line), f);
        printf("Read: %s\n", line);
        fclose(f);
    }
    else printf("Cant open file\n");


    return NULL;
}
