
#include <math.h>
#include <vector>


#include "Common.h"
#include "CGridLibrary.h"
#include "CGridTask.h"
#include "CSerializer.h"

int main(int argc, const char * argv[])
{
    
    int iTasksCount = 8;
    int iResolution = 1024;
    int iStart = time(0);

    // We want to provide the grid server, therefore ensure we got it
    if (argc<5)
    {
        //
        printf("./simpleproject <grid_server> <number of tasks> <resolution> <outputfile>\n");
        
        //
        return -1;
    }
    
    // We will hold the list of Task ID's from the grid here
    std::string strGridServer           = argv[1];                  // The address of the GridMan server
    iTasksCount                         = atoi(argv[2]);
    iResolution                         = atoi(argv[3]);
    std::string strOutputFile           = std::string(argv[4]); // The output file name
    std::string strGridProjectName      = "RAYTRACER";          // The name of the project on GridServer
    std::string strTask                 = "raytracertask";             // The name of the task binary
    std::vector<uint32_t>               vMyTasks;                   // Vector containing our tasks ids on the server
    std::vector<tProjectInfo>           vProjects;                  // We will store the list of projects on the server in this vector
    
    int iScreenWidth = iResolution;
    int iScreenHeight = iResolution;
    
    // 0. Initialize the GRID and check if version/connection are OK
    if (CGridLibrary::Instance()->Initialize(strGridServer) == true)
    {
        // We're Online
        printf("[INFO] Grid Online, Raytracing image resolution=%ix%i, split to %i tasks\n", iResolution,iResolution, iTasksCount);
    }
    else
    {
        // Can't connect
        printf("[INFO] Can't connect to 0GRID Master\n");
        
        //
        return -1;
    }
    
    // 0
    // Let's ensure we don't have a duplicate project with this name
    if (CGridLibrary::Instance()->RemoveProject(strGridProjectName) == false)
    {
        printf("[INFO] Error, couldn't remove project on the Grid\n");
        return -1;
    }
    
    // 1
    // Create Project on the GRID, meaning - we create a PROJECT for which we add tasks later on
    // Project after being created is set to 'STOPPED', we will release it later on.
    // This means that tasks added won't run until we start it.
    if (CGridLibrary::Instance()->CreateProject(strGridProjectName) == false)
    {
        printf("[INFO] Error, couldn't create project on the Grid\n");
        return -1;
    }
    
    //
    printf("[INFO] Project created.\n");
    printf("[INFO] Uploading Tasks to Project\n");

    // 2
    // Start the project
    printf("[INFO] Starting project\n");
    CGridLibrary::Instance()->StartProject(strGridProjectName);
    
    // 3
    // Let's build and send 10 tasks to the server
    for (int a = 0; a < iTasksCount; a++)
    {
        // Create Serializer to set the payload
        CSerializer _Serializer;
        
        // Store the number
        _Serializer.Write_uint32(iScreenWidth);
        _Serializer.Write_uint32(iScreenHeight);
        _Serializer.Write_uint32(a);
        _Serializer.Write_uint32(iScreenWidth/iTasksCount);
        
        // _TaskInfo will hold GridServer unique id of the added task, we want to store it
        tTaskInfo _TaskInfo;
        
        // Prepare and send out the task to the GRID for computing
        if (CGridLibrary::Instance()->AddTask(strTask, strGridProjectName, _Serializer.GetBuffer(), _TaskInfo) == true)
        {
            //
            printf("[INFO] Task %i / %i Added, Unique ID: %i\n", a, iTasksCount, _TaskInfo.m_uiTaskID);
            
            // Store the unique ID for later usage
            vMyTasks.push_back(_TaskInfo.m_uiTaskID);
            
        }
        else
        {
            //
            printf("[INFO] Error, cannot send Task\n");
            
            //
            return -1;
        }
    }

    
    // 4
    // It's time to periodically check if our project is completed, we do it by
    // asking GridMan for the Project details.
    printf("[INFO] Waiting for the project %s to finish on grid\n", strGridProjectName.data());

    //
    bool bFinished = false;

    // We will store the index of our project here, yeah could be done easier.
    int iProjectId = -1;
    
    //
    while (bFinished == false)
    {


        // Clear the list of projects previously received
        vProjects.clear();
        
        // Get a list of projects on Grid Server and store it in our vector
        if(CGridLibrary::Instance()->GetProjects(vProjects) == false)
            return -1;
        
        // Iterate and find our project
        for (int a = 0; a < vProjects.size(); a++)
        {
            // Matches
            if (vProjects[a].m_strName == strGridProjectName)
            {
                // Print out the progress
                printf("[INFO] Completed : %i%% Failed: %i Success: %i Running: %i\n",
                       vProjects[a].m_uiProgress,
                       vProjects[a].m_uiFailedTasks,
                       vProjects[a].m_uiCompletedTasks,
                       vProjects[a].m_uiRunningTasks);
                
                // We know it is completed, so bail out.
                if (vProjects[a].m_uiProgress == 100)
                {
                    bFinished = true;
                    iProjectId = a;
                }
            }
        }
        
        // Give it a while
        CSystemUtil::Sleep(10000);
    }
    
    // 5
    // Okay, let's dump some data.
    printf("[INFO] %s Finished after %i seconds\n",vProjects[iProjectId].m_strName.data(), time(0) - iStart);
    printf("[INFO] Total Tasks: %i\n", vProjects[iProjectId].m_uiTotalTasks);
    printf("[INFO] Completed Tasks: %i\n", vProjects[iProjectId].m_uiCompletedTasks);
    printf("[INFO] Running Tasks: %i\n", vProjects[iProjectId].m_uiRunningTasks);
    printf("[INFO] Failed Tasks: %i\n", vProjects[iProjectId].m_uiFailedTasks);
    
     
    // Store calculated values here, we want to sum all values at the end for fun
    std::vector<uint8_t> vTasksValues;
    
    // Now let's read the payload from each of the tasks
    for( int a = 0; a < vMyTasks.size(); a++ )
    {
        // Get The task by it's unique ID and store details in _TInfo
        tTaskInfo _TInfo;
        if(CGridLibrary::Instance()->GetTask(vMyTasks[a], _TInfo) == true)
        {
        
            printf(" - Task %i, State %i, Payload Size %i, Binary Size %i, Started: %i, Ended: %i\n",
                   _TInfo.m_uiTaskID,
                   _TInfo.m_uiTaskState,
                   _TInfo.m_uiPayloadSize,
                   _TInfo.m_uiBinarySize,
                   _TInfo.m_uiStartTime,
                   _TInfo.m_uiEndTime);
        
            // Init Serializer
            CSerializer _Serializer;
            
            // Set it with the Task Payload
            _Serializer.Set(_TInfo.m_vPayload);
            
            
            // Read uint32 from Payload
            uint64_t uiPayloadDataSize = _Serializer.Read_uint64();
            
            // Display the calculated value
            printf(" - Returned: %lld bytes of payload\n", uiPayloadDataSize);
            
            // Rewrite the values
            for(int i = 0; i < uiPayloadDataSize; i++)
            {
                uint8_t _Byte = _Serializer.Read_uint8();
                vTasksValues.push_back(_Byte);
            }
            
            
        }
        else
        {
            printf("[INFO] Error, couldnt' obtain Task %lld data\n", vMyTasks[a]);
            return -1;
        }
    }

    //
    
    FILE *f;
    int w = iResolution;
    int h = iResolution;
    int k = w*h;
    int s = 4*k;
    int filesize = 54 + s;
    int dpi = 72;
    double factor = 39.375;
    int m = static_cast<int>(factor);
    
    int ppm = dpi*m;
    
    unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0,0,0, 54,0,0,0};
    unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0,24,0};
    
    bmpfileheader[ 2] = (unsigned char)(filesize);
    bmpfileheader[ 3] = (unsigned char)(filesize>>8);
    bmpfileheader[ 4] = (unsigned char)(filesize>>16);
    bmpfileheader[ 5] = (unsigned char)(filesize>>24);
    
    bmpinfoheader[ 4] = (unsigned char)(w);
    bmpinfoheader[ 5] = (unsigned char)(w>>8);
    bmpinfoheader[ 6] = (unsigned char)(w>>16);
    bmpinfoheader[ 7] = (unsigned char)(w>>24);
    
    bmpinfoheader[ 8] = (unsigned char)(h);
    bmpinfoheader[ 9] = (unsigned char)(h>>8);
    bmpinfoheader[10] = (unsigned char)(h>>16);
    bmpinfoheader[11] = (unsigned char)(h>>24);
    
    bmpinfoheader[21] = (unsigned char)(s);
    bmpinfoheader[22] = (unsigned char)(s>>8);
    bmpinfoheader[23] = (unsigned char)(s>>16);
    bmpinfoheader[24] = (unsigned char)(s>>24);
    
    bmpinfoheader[25] = (unsigned char)(ppm);
    bmpinfoheader[26] = (unsigned char)(ppm>>8);
    bmpinfoheader[27] = (unsigned char)(ppm>>16);
    bmpinfoheader[28] = (unsigned char)(ppm>>24);
    
    bmpinfoheader[29] = (unsigned char)(ppm);
    bmpinfoheader[30] = (unsigned char)(ppm>>8);
    bmpinfoheader[31] = (unsigned char)(ppm>>16);
    bmpinfoheader[32] = (unsigned char)(ppm>>24);
    
    f = fopen(strOutputFile.c_str(),"wb");
    
    fwrite(bmpfileheader,1,14,f);
    fwrite(bmpinfoheader,1,40,f);

    //
    for (int i = 0; i < vTasksValues.size(); i++)
    {

        unsigned char color[1];
        color[0] = static_cast<unsigned char>((int)(vTasksValues[i]));
        
        fwrite(color,1,1,f);
    }
    
    fclose(f);
 
    
    printf("[INFO] Project completed\n");
    
    
    return 0;
}
