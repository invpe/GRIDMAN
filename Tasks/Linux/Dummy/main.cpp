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
int main(int argc, const char * argv[])
{

	std::string strTaskPayloadFile = argv[1];
   
 	Exit(strTaskPayloadFile,"Done",0);
}
