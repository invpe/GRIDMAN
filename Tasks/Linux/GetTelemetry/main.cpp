// SeemplyWork - NN - Task
#include <limits>
#include <sstream>
#include <math.h>
#include <vector>
#include <set>
#include <algorithm>
#include "Common.h"
#include "CGridLibrary.h"
#include "CGridTask.h"
#include "CSerializer.h"
#include "CBigInteger.h"
#include "CJZon.h"
 
CSerializer _Serializer;
  
void Exit(const std::string& rstrTaskPayloadFile, const std::string& rstrReason,const int& riCode)
{
	// Write rstrReason to the specified file
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
std::vector<uint8_t> LoadFileToVector(const std::string& filename) {
    std::ifstream inputFile(filename, std::ios::binary | std::ios::ate); // Open file in binary mode at the end
    if (!inputFile.is_open()) {
        Exit(filename, "Cannot open file " + filename, 1);
    }

    // Get the file size
    std::streamsize fileSize = inputFile.tellg();
    if (fileSize <= 0) {
        Exit(filename, "File is empty or an error occurred", 1);
    }

    // Resize vector and read file contents
    std::vector<uint8_t> buffer(fileSize);
    inputFile.seekg(0, std::ios::beg); // Go back to the beginning of the file
    if (!inputFile.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
        Exit(filename, "Failed to read file into buffer", 1);
    }

    inputFile.close();
    return buffer;
}
int main(int argc, const char * argv[])
{

	std::string strTaskPayloadFile = argv[1];
   
   	// Load
   	std::vector<uint8_t> fileData = LoadFileToVector(strTaskPayloadFile); 
    
	// Deserialize
	CSerializer _Serializer; 
	_Serializer.Set(fileData);
	std::string strSerializedString = _Serializer.Read_string();
	
    std::string strGridHostname;  
	std::string strTelemetryFile;  
	std::string strOutputFile;   

 	
 	// Split by ','
	std::istringstream ss(strSerializedString);

    std::getline(ss, strGridHostname, ',');
    std::getline(ss, strTelemetryFile, ',');
    std::getline(ss, strOutputFile, ',');  


    // Spit out the parsed values
    std::cout << "GridServer: "<< strGridHostname << std::endl;
    std::cout << "Telemetry: "<< strTelemetryFile << std::endl;
    std::cout << "Output file: " << strOutputFile << std::endl; 

    // Fire up GRID connection
 	if(CGridLibrary::Instance()->Initialize(strGridHostname) == false)
 		Exit(strTaskPayloadFile,"Cant connect to grid '"+strGridHostname+"'",1); 		

 	// Obtain telemetry file requested
 	std::vector<uint8_t> vData;
    if(CGridLibrary::Instance()->GetTelemetry(strTelemetryFile,vData) == false)
    	Exit(strTaskPayloadFile,"Cant get '"+strTelemetryFile+"' from grid filesystem", 1);

    CGridLibrary::Instance()->HeartBeat();
    
    // Store the download telemetry file to a temporary CSV
 	std::ofstream outputFile(strOutputFile);

    if (!outputFile.is_open())         
        Exit(strTaskPayloadFile,"Cant open file for writing CSV",1); 	
   
    // Write the data to the file as characters
    for (uint8_t byte : vData) 
        outputFile << static_cast<char>(byte);

    outputFile.close();
    
 	Exit(strTaskPayloadFile,"Job done",0);
}
