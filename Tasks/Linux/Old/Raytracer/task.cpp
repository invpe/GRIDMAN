//
//  main.cpp
// 
#include <math.h>
#include <iostream>
#include <stdio.h>
#include <assert.h>

#include "CSerializer.h"
#include "CGridTask.h"


#include "CScene.h"
#include "CRayTracer.h"

/*

 */
int main(int argc, char **argv)
{
    
    
    // Sanity check
    if(argc < 2)
    {
        printf("Error, no payload passed as an argument\n");
        return -1;
    }
    
    // Dump some handy output
    std::string strPayload = argv[1];
    printf("Started with payload: %s\n", strPayload.data());
    
    

	// Incoming / Outgoing Payloads
    // We will store our Incoming payload and Outgoung payload in two
    // vectors of uint8_t 1bytes
	std::vector<uint8_t> m_vIncomingPayload;
	std::vector<uint8_t> m_vOutgoingPayload;


    // 1. Initialize the Task by reading in the Payload data from
    // provided filename
	if (CGridTask::Init(m_vIncomingPayload, strPayload) == false)
	{
        // This means, that Worker can have some problems
        // See Worker log
		printf("Info: Error, couldn't initialize the Payload\n");

		//
		return -1;
	}  

	// Initialize Serializer to read in the payload data
	CSerializer _Serializer;
    
    // Set Serializer data
	_Serializer.Set(m_vIncomingPayload);

	// 2. Parse the Task-Specific Payload
	uint32_t uiWidth        = _Serializer.Read_uint32();
	uint32_t uiHeight       = _Serializer.Read_uint32();
    uint32_t uiPart         = _Serializer.Read_uint32();
    uint32_t uiPartWidth    = _Serializer.Read_uint32();
    int iStart = time(0);
    
    printf("Info: Raytracing <%i x 0> <%i x %i> Size <%i x %i> Part: %i PartWidth: %i\n", uiPart * uiPartWidth, (uiPart * uiPartWidth) + uiPartWidth,uiHeight, uiWidth, uiHeight, uiPart, uiPartWidth);
    
    // Render Part of the scene
    CRayTracer _Tracer;
    _Tracer.Init(uiWidth, uiHeight);
    
    // Clear the Serialized data
    _Serializer.Reset();
    
    
    // Store the data size *3 (RGB)
    _Serializer.Write_uint64((uiPartWidth*uiHeight) * 3);
    
    printf("Info: Payloadsize %i\n",uiPartWidth*uiHeight*3);
    
    //
    for(int x = uiPart * uiPartWidth; x < (uiPart * uiPartWidth) + uiPartWidth; x++)
    {
        for(int y = 0; y < uiHeight; y++)
        {
            CVector3 vRGB =  _Tracer.RayTrace(x, y);
            
            int R = vRGB.x * 255;
            int G = vRGB.y * 255;
            int B = vRGB.z * 255;
            
            if(R>255)R=255;
            if(G>255)G=255;
            if(B>255)B=255;
            
            
            
            _Serializer.Write_uint8(static_cast<int>(R));
            _Serializer.Write_uint8(static_cast<int>(G));
            _Serializer.Write_uint8(static_cast<int>(B));
            
            
        }
    }
    
    printf("Info: Raytracing completed (%i seconds)\n", time(0) - iStart);
   
    
    printf("Info: Job done\n");
    
	// 4. Deinit sending out the Task and Payload back to the GridMan Server
	CGridTask::Deinit(_Serializer.GetBuffer(), strPayload);
    
	
	//
	return 0;
}

