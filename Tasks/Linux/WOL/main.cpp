#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <array>
#include <math.h>
#include <vector>
#include <ctime>
#include <cstdio>
#include <string>
#include <algorithm>  
#include <cctype>   
#include <deque>     
#include <unordered_set>

#include "Common.h"
#include "CGridLibrary.h"
#include "CGridTask.h" 
#include "CSystemUtil.h" 
void Exit(const std::string& rstrTaskPayloadFile, const std::string& rstrReason, const int& riCode)
{
    std::ofstream outputFile(rstrTaskPayloadFile);
    if (outputFile.is_open()) {
        outputFile << rstrReason << std::endl;
        outputFile.close();
    } else {
        printf("Failed to write to file: %s\n", rstrTaskPayloadFile.c_str());
    }
    printf("%s\n", rstrReason.data());
    exit(riCode);
}
 

int send_wol(const char *mac_str) {
    unsigned char magic_packet[102];
    unsigned char mac[6];
    int i;
 
    if (sscanf(mac_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
               &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) != 6) {
        fprintf(stderr, "Invalid MAC address format\n");
        return 1;
    }
 
    memset(magic_packet, 0xFF, 6);                 
    for (i = 1; i <= 16; i++)                      
        memcpy(&magic_packet[i * 6], mac, 6);
 
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }
 
    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval));
 
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9);
    addr.sin_addr.s_addr = inet_addr("255.255.255.255");  
 
    for(int x = 0; x < 10; x++)
    {
        if (sendto(sockfd, magic_packet, sizeof(magic_packet), 0,
                   (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("sendto");
            close(sockfd);
            return 1;
        }
    }

    close(sockfd);
    return 0;
}

int main(int argc, char *argv[]) 
{
    std::string strTaskPayloadFile = argv[1];
    std::ifstream file(strTaskPayloadFile);
    if (!file) return 1;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string strSerializedString = buffer.str();

    std::istringstream ss(strSerializedString);
    std::string strMacAddress;
    std::getline(ss, strMacAddress, ',');   



    return send_wol(strMacAddress.c_str());
}
