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
	std::string strInputFile;  
	std::string strOutputFile;   
    std::string strAppend;   

 	
 	// Split by ','
	std::istringstream ss(strSerializedString);

    std::getline(ss, strGridHostname, ',');
    std::getline(ss, strInputFile, ',');
    std::getline(ss, strOutputFile, ',');  
    std::getline(ss, strAppend, ',');  


    // Spit out the parsed values
    std::cout << "GridServer: "<< strGridHostname << std::endl;
    std::cout << "Input file: "<< strInputFile << std::endl;
    std::cout << "Output file: " << strOutputFile << std::endl; 
    std::cout << "Append flag: " << strAppend << std::endl; 

    uint32_t uiAppend = std::stoi(strAppend);

    // Open the file for reading in binary mode
    std::ifstream inFile(strInputFile, std::ios::binary);

    // Check if the file is open
    if (!inFile) 
        Exit(strTaskPayloadFile,"Cant open file '"+strInputFile+"'",1); 

    // Determine the file size
    inFile.seekg(0, std::ios_base::end); // Move the cursor to the end of the file
    std::streamsize size = inFile.tellg(); // Get the position of the cursor (file size)
    inFile.seekg(0, std::ios_base::beg); // Move the cursor back to the start of the file

    // Create a vector to store the data
    std::vector<uint8_t> vBuffer(size);

    // Read the data all at once
    if (size > 0) {
        inFile.read(reinterpret_cast<char*>(vBuffer.data()), size);
    }

    // Close the file
    inFile.close();





    // Fire up GRID connection
 	if(CGridLibrary::Instance()->Initialize(strGridHostname) == false)
 		Exit(strTaskPayloadFile,"Cant connect to grid '"+strGridHostname+"'",1); 		

 	if(CGridLibrary::Instance()->AddTelemetry(strOutputFile,uiAppend,vBuffer) == false)
        Exit(strTaskPayloadFile,"Failed to add file '"+strInputFile+"'",1);    
                
    
 	Exit(strTaskPayloadFile,"Job done, added "+strOutputFile+" append: "+strAppend,0);
}
