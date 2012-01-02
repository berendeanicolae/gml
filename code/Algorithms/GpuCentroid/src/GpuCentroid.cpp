#include "GpuCentroid.h"

char* GpuProgramSource [GPU_NR_KERNELS] = {{
    "__kernel void EuclidDist("
        "__global  float* RecData, "
        "const unsigned int NrRec, "
        "__global  float* CurrRec, "
        "const unsigned int NrFeats, "
        "__global  float* OutDist, "
        "const unsigned int OutStart) "
    "{"
        "int gid = get_global_id(0);"        
        "float sum=0;"
        "for(int i=0;i<NrFeats;i++)"
            "sum += (CurrRec[i] - RecData[gid*NrFeats+i])*(CurrRec[i] - RecData[gid*NrFeats+i]);"
        "OutDist[OutStart + gid] = sum;        "
    "}"
}};

char * GpuCentroid::GetCLErrorMsg(cl_int err) {
    switch (err) {
        case CL_SUCCESS:                          return _strdup("Success!");
        case CL_DEVICE_NOT_FOUND:                 return _strdup("Device not found.");
        case CL_DEVICE_NOT_AVAILABLE:             return _strdup("Device not available");
        case CL_COMPILER_NOT_AVAILABLE:           return _strdup("Compiler not available");
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:    return _strdup("Memory object allocation failure");
        case CL_OUT_OF_RESOURCES:                 return _strdup("Out of resources");
        case CL_OUT_OF_HOST_MEMORY:               return _strdup("Out of host memory");
        case CL_PROFILING_INFO_NOT_AVAILABLE:     return _strdup("Profiling information not available");
        case CL_MEM_COPY_OVERLAP:                 return _strdup("Memory copy overlap");
        case CL_IMAGE_FORMAT_MISMATCH:            return _strdup("Image format mismatch");
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:       return _strdup("Image format not supported");
        case CL_BUILD_PROGRAM_FAILURE:            return _strdup("Program build failure");
        case CL_MAP_FAILURE:                      return _strdup("Map failure");
        case CL_INVALID_VALUE:                    return _strdup("Invalid value");
        case CL_INVALID_DEVICE_TYPE:              return _strdup("Invalid device type");
        case CL_INVALID_PLATFORM:                 return _strdup("Invalid platform");
        case CL_INVALID_DEVICE:                   return _strdup("Invalid device");
        case CL_INVALID_CONTEXT:                  return _strdup("Invalid context");
        case CL_INVALID_QUEUE_PROPERTIES:         return _strdup("Invalid queue properties");
        case CL_INVALID_COMMAND_QUEUE:            return _strdup("Invalid command queue");
        case CL_INVALID_HOST_PTR:                 return _strdup("Invalid host pointer");
        case CL_INVALID_MEM_OBJECT:               return _strdup("Invalid memory object");
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:  return _strdup("Invalid image format descriptor");
        case CL_INVALID_IMAGE_SIZE:               return _strdup("Invalid image size");
        case CL_INVALID_SAMPLER:                  return _strdup("Invalid sampler");
        case CL_INVALID_BINARY:                   return _strdup("Invalid binary");
        case CL_INVALID_BUILD_OPTIONS:            return _strdup("Invalid build options");
        case CL_INVALID_PROGRAM:                  return _strdup("Invalid program");
        case CL_INVALID_PROGRAM_EXECUTABLE:       return _strdup("Invalid program executable");
        case CL_INVALID_KERNEL_NAME:              return _strdup("Invalid kernel name");
        case CL_INVALID_KERNEL_DEFINITION:        return _strdup("Invalid kernel definition");
        case CL_INVALID_KERNEL:                   return _strdup("Invalid kernel");
        case CL_INVALID_ARG_INDEX:                return _strdup("Invalid argument index");
        case CL_INVALID_ARG_VALUE:                return _strdup("Invalid argument value");
        case CL_INVALID_ARG_SIZE:                 return _strdup("Invalid argument size");
        case CL_INVALID_KERNEL_ARGS:              return _strdup("Invalid kernel arguments");
        case CL_INVALID_WORK_DIMENSION:           return _strdup("Invalid work dimension");
        case CL_INVALID_WORK_GROUP_SIZE:          return _strdup("Invalid work group size");
        case CL_INVALID_WORK_ITEM_SIZE:           return _strdup("Invalid work item size");
        case CL_INVALID_GLOBAL_OFFSET:            return _strdup("Invalid global offset");
        case CL_INVALID_EVENT_WAIT_LIST:          return _strdup("Invalid event wait list");
        case CL_INVALID_EVENT:                    return _strdup("Invalid event");
        case CL_INVALID_OPERATION:                return _strdup("Invalid operation");
        case CL_INVALID_GL_OBJECT:                return _strdup("Invalid OpenGL object");
        case CL_INVALID_BUFFER_SIZE:              return _strdup("Invalid buffer size");
        case CL_INVALID_MIP_LEVEL:                return _strdup("Invalid mip-map level");
        default:                                  return _strdup("Unknown");
    }
}

GpuCentroid::GpuCentroid()
{
	ObjectName = "GpuCentroid";
    threadsCount = 3;

	//Add extra commands here                                                                                          
	SetPropertyMetaData("Command","!!LIST:None=0,Compute!!");    
    LinkPropertyToString("PosCentFileName",optPosCentFileName,"gpu-centroids-positive.txt","The name of the file where the positive the centroids will be saved!");
    LinkPropertyToString("NegCentFileName",optNegCentFileName,"gpu-centroids-negative.txt","The name of the file where the negative the centroids will be saved!");
    LinkPropertyToUInt32("MinCentroidSize",optMinCentroidSize, 100, "The min nr of individuals in a centroid");
    LinkPropertyToBool("OutputClusterHashes",optOutputClusters, false, "Should we output the clusters?");
    LinkPropertyToUInt32("Method",optMethod,0,"!!LIST:PositiveCentroids=0,NegativeCentroids,AllCentroids!!");   

    PtrPosData = NULL;
    PtrNegData = NULL;
    NrPosRec = 0;
    NrNegRec = 0;

    IndexPosRec = NULL;
    IndexNegRec = NULL;

    gcmd = NULL;
}

GpuCentroid::~GpuCentroid()
{
    delete PtrPosData;
    delete PtrNegData;
    delete IndexPosRec;
    delete IndexNegRec;
}

bool GpuCentroid::Init()
{
    CHECK(InitConnections());
	CHECK(InitThreads());
    CHECK(SplitMLThreadDataRange(con->GetRecordCount()));

    CHECKMSG(con->CreateMlRecord(MainRecord), "Unable to create MainRecord");
	
	CHECK(InitializeGpuInfo());
	return true;
}
void GpuCentroid::OnRunThreadCommand(MLThreadData &thData,UInt32 threadCommand)
{
    //INFOMSG("CALL: OnRunThreadCommand");
	switch (threadCommand)
	{
    case THREAD_COMMAND_MIN_DIST_POS2NEG:
        if (GpuComputeMain(thData, threadCommand)==false) {
            ERRORMSG("Failed to compute min dist on GPU");
            return;
        }
        break;
    case THREAD_COMMAND_MIN_DIST_NEG2POS:
        if (GpuComputeMain(thData, threadCommand)==false) {
            ERRORMSG("Failed to compute min dist on GPU");
            return;
        }
        break;
    case THREAD_COMMAND_EXTRACT_POSITIVE_CLUSTERS:
        if (GpuComputeMain(thData, threadCommand)==false) {
            ERRORMSG("Failed to compute positive clusters");
            return;
        }
        break;
    case THREAD_COMMAND_EXTRACT_NEGATIVE_CLUSTERS:
        if (GpuComputeMain(thData, threadCommand)==false) {
            ERRORMSG("Failed to compute positive clusters");
            return;
        }
        break;
	case THREAD_COMMAND_NONE:			
	    return;
    // add extra thread command processes here
	};
}
bool GpuCentroid::OnInitThreadData(MLThreadData &thData)
{
    //INFOMSG("CALL: OnInitThreadData");		
	thData.Context = (void*)&GcThData;
	return true;
}

void GpuCentroid::OnExecute()
{
	switch (Command)
	{
		case COMMAND_NONE:
			notif->Error("[%s] -> Nothing to do , select another command ",ObjectName);
			break;
        case COMMAND_COMPUTE:            
            MainAlgorithm();
            break;
		default:
			notif->Error("[%s] -> Unknown command ID: %d",ObjectName,Command);
			break;
	}	
}

bool GpuCentroid::MainAlgorithm()
{       
    // used to temp store the distances computed by every work group    
    DWORD start, stop;

    DRAWLINE;    
    CHECK(CreateAlgorithmDatabases());
    
    DistVectPos2Neg = new float[NrPosRec];
    CHECKMSG(DistVectPos2Neg, "Failed to alloc memory for DistVectPos2Neg");
    DistVectNeg2Pos = new float[NrNegRec];
    CHECKMSG(DistVectNeg2Pos, "Failed to alloc memory for DistVectNeg2Pos");

    if (optMethod == ALG_METHOD_POS_ONLY || optMethod == ALG_METHOD_BOTH) {
        DRAWLINE;    
        INFOMSG("Computing min distance for every positive to the negative side");
        start = GetTickCount();
        ExecuteParalelCommand(THREAD_COMMAND_MIN_DIST_POS2NEG);
        stop = GetTickCount();
        INFOMSG ("The computing took: %d sec", (stop-start)/1000);

        DRAWLINE;    
        INFOMSG("Computing dist from every positive to every positive to extract clusters");
        start = GetTickCount();
        ExecuteParalelCommand(THREAD_COMMAND_EXTRACT_POSITIVE_CLUSTERS);
        stop = GetTickCount();
        INFOMSG ("The computing took: %d sec", (stop-start)/1000);
    }

    if (optMethod == ALG_METHOD_NEG_ONLY || optMethod == ALG_METHOD_BOTH) {
        DRAWLINE;    
        start = GetTickCount();
        INFOMSG("Computing min distance for every negative to the positive side");
        ExecuteParalelCommand(THREAD_COMMAND_MIN_DIST_NEG2POS);
        stop = GetTickCount();
        INFOMSG ("The computing took: %d sec", (stop-start)/1000);
    
        DRAWLINE;    
        INFOMSG("Computing dist from every negative to every negative to extract clusters");
        start = GetTickCount();
        ExecuteParalelCommand(THREAD_COMMAND_EXTRACT_NEGATIVE_CLUSTERS);
        stop = GetTickCount();
        INFOMSG ("The computing took: %d sec", (stop-start)/1000);
     }
    
    return true;
}

bool GpuCentroid::CreateAlgorithmDatabases()
{
    int NrRec = con->GetRecordCount();
    float *negptr,*posptr;
    
    INFOMSG("Getting the number of records for each class");
    for (int i=0;i<NrRec;i++) {
        CHECK(con->GetRecord(MainRecord, i));
        if (MainRecord.Label == 1) NrPosRec++;
        else NrNegRec++;
    }

    INFOMSG("Trying to allocate enough memory for database");

    CHECK(con->GetRecord(MainRecord, 0));
    NrFeats = MainRecord.FeatCount;

    PtrPosData = new float[NrPosRec*NrFeats];
    NULLCHECKMSG(PtrPosData, "Unable to allocate memory for positive database");

    PtrNegData = new float[NrNegRec*NrFeats];
    NULLCHECKMSG(PtrPosData, "Unable to allocate memory for negative database");

    IndexPosRec = new UInt32[NrPosRec];
    NULLCHECKMSG(PtrPosData, "Unable to allocate memory for positive database index");

    IndexNegRec = new UInt32[NrNegRec];
    NULLCHECKMSG(PtrPosData, "Unable to allocate memory for negative database index");

    INFOMSG("Moving data from connector to databases");
    negptr = PtrNegData;
    posptr = PtrPosData;    

    int IdxPos=0, IdxNeg =0;
    for (int i=0;i<NrRec;i++) {        
        CHECK(con->GetRecord(MainRecord, i));
        if (MainRecord.Label == 1) {
            IndexPosRec[IdxPos++] = i;
            for (int j=0;j<NrFeats;j++)
                posptr[j] = (float)MainRecord.Features[j];
            posptr += NrFeats;
        } else {
            IndexNegRec[IdxNeg++] = i;
            for (int j=0;j<NrFeats;j++)
                negptr[j] = (float)MainRecord.Features[j];
            negptr += NrFeats;
        }
    }
    INFOMSG("Finished creating hostside-databases");

    // create gpu memory locations 
    cl_ulong SizePerRecord,NrRecPerAlloc, RecNow, RecUntilNow;        
    cl_int  ErrorCode; 
    float * ptr;

    SizePerRecord = NrFeats*sizeof(float);
    NrRecPerAlloc = gcmd[0].MaxAllocSz/SizePerRecord;    

    RecNow = 0;
    RecUntilNow = 0;
    ptr = PtrNegData;
    NrMemPoolNeg  = (SizePerRecord*NrNegRec)/gcmd[0].MaxAllocSz+1;    
    for (int i=0;i<NrMemPoolNeg;i++) {
        if (i==NrMemPoolNeg-1) RecNow = NrNegRec-RecUntilNow;
        else RecNow = NrRecPerAlloc;
        
        clMemPoolNeg[i].NrRec = RecNow;
        // the alloc
        clMemPoolNeg[i].Mem = clCreateBuffer(clContext, CL_MEM_USE_HOST_PTR|CL_MEM_READ_ONLY , RecNow*SizePerRecord, ptr, &ErrorCode);        
        CHECKMSG(!clMemPoolNeg[i].Mem && ErrorCode!=CL_SUCCESS, "Error alloc in gpu global memory");        

        // for the next loop
        RecUntilNow += RecNow;
        ptr += RecNow*NrFeats;
    }

    RecNow = 0;
    RecUntilNow = 0;
    ptr = PtrPosData;
    NrMemPoolPos  = (SizePerRecord*NrNegRec)/gcmd[0].MaxAllocSz+1;    
    for (int i=0;i<NrMemPoolPos;i++) {
        if (i==NrMemPoolPos-1) RecNow = NrPosRec-RecUntilNow;
        else RecNow = NrRecPerAlloc;
        
        clMemPoolPos[i].NrRec = RecNow;
        // the alloc
        clMemPoolPos[i].Mem = clCreateBuffer(clContext, CL_MEM_USE_HOST_PTR|CL_MEM_READ_ONLY , RecNow*SizePerRecord, ptr, &ErrorCode);
        CHECKMSG(!clMemPoolPos[i].Mem && ErrorCode!=CL_SUCCESS, "Error alloc in gpu global memory");        

        // for the next loop
        RecUntilNow += RecNow;
        ptr += RecNow*NrFeats;
    }
    INFOMSG("Finished creating device-databases");

    return true;
}

bool GpuCentroid::InitializeGpuInfo()
{     
    cl_int          ErrorCode;     // error code returned from api calls
    cl_program      clProgram;        // compute program        

    // Connect to a compute device        
    ErrorCode = clGetPlatformIDs(MAX_INST_PLATF, &clPlatform, &clNrPlatforms);
    CLCHECKMSG("Failed to find a platform!");
    INFOMSG("GPU: We have %d platforms", clNrPlatforms);    

    // Get a device of the appropriate type
    ErrorCode = clGetDeviceIDs(clPlatform, CL_DEVICE_TYPE_GPU, MAX_INST_DEV, clDeviceIds, &clNrDevices);
    CLCHECKMSG("Failed to get device ids!");
    INFOMSG("GPU: We have %d devices", clNrDevices);
    
    // Create a compute context
    clContext = clCreateContext(0, clNrDevices, clDeviceIds, NULL, NULL, &ErrorCode);
    CLCHECKMSG("GPU: Failed to create context for my devices");        

    gcmd = new GpuCommand[clNrDevices];
    for (UInt32 i=0;i<clNrDevices;i++) {
        gcmd[i].DevId = clDeviceIds[i];   

        // Create the command queue to control the devices
        gcmd[i].Cmd = clCreateCommandQueue(clContext, gcmd[i].DevId, CL_QUEUE_PROFILING_ENABLE, &ErrorCode);
        CLCHECKMSG("GPU: Failed to create command queue for device");        

        DRAWLINE;
        // Get global memory size
        ErrorCode = clGetDeviceInfo(gcmd[i].DevId, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(gcmd[i].MaxGlobSz), &(gcmd[i].MaxGlobSz), NULL);
        CLCHECKMSG("GPU: Failed to retrieve max global mem size");
        INFOMSG("GPU: Device %d total global memory size   = %d MB", i, gcmd[i].MaxGlobSz/ONEMEGABYTE);

        // Get max memory alloc size
        ErrorCode = clGetDeviceInfo(gcmd[i].DevId, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(gcmd[i].MaxAllocSz), &(gcmd[i].MaxAllocSz), NULL);
        CLCHECKMSG("GPU: Failed to retrieve max mem alloc size");        
        INFOMSG("GPU: Device %d max memory allocation size = %d MB", i, gcmd[i].MaxAllocSz/ONEMEGABYTE);

        // Get local memory size
        ErrorCode = clGetDeviceInfo(gcmd[i].DevId, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(gcmd[i].MaxLocalSz), &(gcmd[i].MaxLocalSz), NULL);
        CLCHECKMSG("GPU: Failed to retrieve max local mem size");                
        INFOMSG("GPU: Device %d total local memory size    = %d KB", i, gcmd[i].MaxLocalSz/ONEKILOBYTE);
    }

    // Create the compute program from the source buffer
    clProgram = clCreateProgramWithSource(clContext, GPU_NR_KERNELS, (const char**)GpuProgramSource, NULL, &ErrorCode);
    CLCHECKMSG("GPU: Failed to create program with the gpu source code");        

    // Build the program executable
    ErrorCode = clBuildProgram(clProgram, 0, NULL, NULL, NULL, NULL);
    if (ErrorCode != CL_SUCCESS) {
        size_t len;
        char buffer[2048];
        ERRORMSG("GPU: Failed to build program executable! The build log follows");        
        clGetProgramBuildInfo(clProgram, clDeviceIds[0], CL_PROGRAM_BUILD_LOG,sizeof(buffer), buffer, &len);
        ERRORMSG(buffer);
        return false;;
    }

    for (UInt32 i=0;i<clNrDevices;i++) {
        // Create the compute kernel in the program
        gcmd[i].kern[0].Kernel = clCreateKernel(clProgram, "EuclidDist", &ErrorCode);
        CLCHECKMSG("GPU: Failed to create compute kernel!");        
    
        // Get the maximum work group size for executing the kernel on the device
        for (UInt32 j=0;j<GPU_NR_KERNELS;j++) {
            ErrorCode = clGetKernelWorkGroupInfo(gcmd[i].kern[j].Kernel, clDeviceIds[i], CL_KERNEL_WORK_GROUP_SIZE, sizeof(gcmd[i].kern[j].WorkGroupSz), &(gcmd[i].kern[j].WorkGroupSz), NULL);    
            CLCHECKMSG("GPU: Failed to retrieve kernel work group info!");

            ErrorCode = clGetKernelWorkGroupInfo(gcmd[i].kern[j].Kernel, clDeviceIds[i], CL_KERNEL_LOCAL_MEM_SIZE, sizeof(gcmd[i].kern[j].LocalMemSz), &(gcmd[i].kern[j].LocalMemSz), NULL);    
            CLCHECKMSG("GPU: Failed to retrieve kernel local memory info!");        
        }
    }

    return true;
}

bool GpuCentroid::GpuComputeMain(MLThreadData &thData, UInt32 ThreadCommand)
{
    cl_int          ErrorCode;
    cl_mem          OutDistMem, CurrRecMem;         

    float *thCurrRecPtr = NULL, *thOutDistVect, *thDistVect;
    char  thFileClust[MAX_PATH] = "";
    int    thNrRecFrom = 0, thNrRecTo = 0, thRecIndexStart;
    cl_ulong OutStart= 0;    
    cl_ulong thMemPoolSz;
    cl_ulong RecordSz = NrFeats*sizeof(float);
    GpuMemPool*  thMemPool;
    GML::Utils::File    thFileObj;

    // we exit if we have more threads than we need
    if (thData.ThreadID >= clNrDevices) return true;

    switch(ThreadCommand) {
    case THREAD_COMMAND_MIN_DIST_POS2NEG: {
            thCurrRecPtr = PtrPosData;
            thNrRecFrom = NrPosRec;        
            thNrRecTo   = NrNegRec;
            thMemPool = (GpuMemPool*)&clMemPoolNeg[0];
            thMemPoolSz = NrMemPoolNeg;
            thOutDistVect = DistVectPos2Neg;
            thFileClust[0]=0;
            break;
        }
    case THREAD_COMMAND_MIN_DIST_NEG2POS:{
            thCurrRecPtr = PtrNegData;
            thNrRecFrom = NrNegRec;
            thNrRecTo   = NrPosRec;
            thMemPool = (GpuMemPool*)&clMemPoolPos[0];
            thMemPoolSz = NrMemPoolPos;
            thOutDistVect = DistVectNeg2Pos;
            thFileClust[0]=0;
            break;
        }
    case THREAD_COMMAND_EXTRACT_POSITIVE_CLUSTERS:{
            thCurrRecPtr = PtrPosData;
            thNrRecFrom = NrPosRec;
            thNrRecTo   = NrPosRec;
            thMemPool = (GpuMemPool*)&clMemPoolPos[0];
            thMemPoolSz = NrMemPoolPos;
            thOutDistVect = NULL;
            thDistVect = DistVectPos2Neg;
            sprintf_s(thFileClust, MAX_PATH, "th%d-%s", thData.ThreadID, optPosCentFileName.GetText());
            break;
        }
    case THREAD_COMMAND_EXTRACT_NEGATIVE_CLUSTERS:{
            thCurrRecPtr = PtrNegData;
            thNrRecFrom = NrNegRec;
            thNrRecTo   = NrNegRec;
            thMemPool = (GpuMemPool*)&clMemPoolNeg[0];
            thMemPoolSz = NrMemPoolNeg;
            thOutDistVect = NULL;
            thDistVect = DistVectNeg2Pos;
            sprintf_s(thFileClust, MAX_PATH, "th%d-%s", thData.ThreadID, optNegCentFileName.GetText());
            break;
       }

    default:
        CHECKMSG(false, "Unknown command for this routine");
    }
    
    if (thFileClust[0]!=0)  {
        INFOTHDMSG("output file: %s", thFileClust);
        CHECKMSG(thFileObj.Create(thFileClust), "Failed to create output file");            
    }

    thCurrRecPtr = (float*)(thCurrRecPtr+thData.ThreadID*(thNrRecFrom/clNrDevices+1)*NrFeats);
    thRecIndexStart = thData.ThreadID*(thNrRecFrom/clNrDevices+1);
        
    if (thData.ThreadID==clNrDevices-1) 
        thNrRecFrom = thNrRecFrom - thData.ThreadID*(thNrRecFrom/clNrDevices+1);
    else
        thNrRecFrom = thNrRecFrom/clNrDevices+1;    

    float * Dist =  new float[thNrRecTo];
    CHECKMSG(Dist, "Failed to alloc memory for output dist");

    // memory buffer for output
    OutDistMem = clCreateBuffer(clContext, CL_MEM_USE_HOST_PTR|CL_MEM_WRITE_ONLY, thNrRecTo*sizeof(float), Dist, &ErrorCode);
    CHECKMSG(!OutDistMem && ErrorCode!=CL_SUCCESS, "Error alloc in gpu global memory");            

    // memory buffer for current 'from' record
    CurrRecMem = clCreateBuffer(clContext, CL_MEM_USE_HOST_PTR|CL_MEM_READ_ONLY , RecordSz, thCurrRecPtr, &ErrorCode);
    CHECKMSG(!CurrRecMem && ErrorCode!=CL_SUCCESS, "Error alloc in gpu global memory");        

    for (int i=0;i<thNrRecFrom;i++) {
        // write the current record to the device
        ErrorCode = clEnqueueWriteBuffer(gcmd[thData.ThreadID].Cmd, CurrRecMem, CL_FALSE, 0, RecordSz, thCurrRecPtr, 0, NULL, NULL);
        CLCHECKMSG("Failed to send memory buffer to device");
        
        OutStart = 0;
        // loop for every buffer
        for (int j=0;j<thMemPoolSz;j++) {
            ErrorCode  = clSetKernelArg(gcmd[thData.ThreadID].kern[0].Kernel, 0, sizeof(cl_mem), &(thMemPool[j].Mem));
            ErrorCode |= clSetKernelArg(gcmd[thData.ThreadID].kern[0].Kernel, 1, sizeof(unsigned int), &(thMemPool[j].NrRec));                 
            ErrorCode |= clSetKernelArg(gcmd[thData.ThreadID].kern[0].Kernel, 2, sizeof(cl_mem), &CurrRecMem);
            ErrorCode |= clSetKernelArg(gcmd[thData.ThreadID].kern[0].Kernel, 3, sizeof(unsigned int), &NrFeats);        
            ErrorCode |= clSetKernelArg(gcmd[thData.ThreadID].kern[0].Kernel, 4, sizeof(cl_mem), &OutDistMem);        
            ErrorCode |= clSetKernelArg(gcmd[thData.ThreadID].kern[0].Kernel, 5, sizeof(unsigned int), &OutStart); 
            CLCHECKMSG("Failed to set kernel arguments");
            
            size_t global = thMemPool[j].NrRec;
            ErrorCode = clEnqueueNDRangeKernel(gcmd[thData.ThreadID].Cmd, gcmd[thData.ThreadID].kern[0].Kernel, 1, NULL, &global, NULL, 0, NULL, NULL);
            CLCHECKMSG("Failed to run kernel");                                
            // for the next loop
            OutStart += thMemPool[j].NrRec;
        }
        // Wait for all commands to complete    
        clFinish(gcmd[thData.ThreadID].Cmd);
        ErrorCode = clEnqueueReadBuffer(gcmd[thData.ThreadID].Cmd, OutDistMem, CL_TRUE, 0, thNrRecTo*sizeof(float), Dist, 0, NULL, NULL ); 
        CLCHECKMSG("Failed to read data back from the device");

        if (ThreadCommand == THREAD_COMMAND_MIN_DIST_POS2NEG || ThreadCommand == THREAD_COMMAND_MIN_DIST_NEG2POS) {
            float sum = FLT_MAX;
            for (int k=0;k<thNrRecTo;k++) 
                if (sum > Dist[k]) sum = Dist[k];            
            thOutDistVect[thRecIndexStart+i] = sum;
        } else if (ThreadCommand==THREAD_COMMAND_EXTRACT_POSITIVE_CLUSTERS || ThreadCommand == THREAD_COMMAND_EXTRACT_NEGATIVE_CLUSTERS){
            UInt32 nr=0;
            for (int k=0;k<thNrRecTo;k++)
                if (k!=thRecIndexStart+i && Dist[k]<thDistVect[thRecIndexStart+i]) nr++;            
            
            // the part where we print the hashes to output files
            if (nr>=optMinCentroidSize) {
                char tmpBuf[1024];
                GML::Utils::GString     tmpStr;
                GML::DB::RecordHash     tmpRecHash;
                UInt64  size;
                bool IsPos = (ThreadCommand==THREAD_COMMAND_EXTRACT_POSITIVE_CLUSTERS)?true:false;

                CHECKMSG(con->GetRecordHash(tmpRecHash, GetRealIdx(thRecIndexStart+i, IsPos)), "Failed to get record hash from connector");
                CHECKMSG(tmpRecHash.ToString(tmpStr),"Failed to extract string from hash");
                
                sprintf_s(tmpBuf, 1024, "%s;maxdist=%.02f;size=%d;\n", tmpStr.GetText(), thDistVect[thRecIndexStart+i], nr);
                size = strlen(tmpBuf);
                CHECKMSG(thFileObj.Write(tmpBuf, size, &size),"Failed to write in file");

                if (optOutputClusters) {
                    // print the cluster hashes
                    for (int k=0;k<thNrRecTo;k++)
                        if (k!=thRecIndexStart+i && Dist[k]<thDistVect[thRecIndexStart+i]) {
                            CHECKMSG(con->GetRecordHash(tmpRecHash, GetRealIdx(k, IsPos)), "Failed to get record hash");
                            CHECKMSG(tmpRecHash.ToString(tmpStr),"Failed to extract string from hash");
                            sprintf_s(tmpBuf, 1024, "\t%s\n", tmpStr.GetText());
                            
                            size = strlen(tmpBuf);
                            CHECKMSG(thFileObj.Write(tmpBuf, size, &size),"Failed to write in file");
                        }
                    INFOTHDMSG("Center %5d; Dist: %d; Members: %d", thRecIndexStart+i, (int)thDistVect[thRecIndexStart+i], nr);
                }
            }
        }
        //INFOTHDMSG("POS %05d -> NEG; MIN DIST = %03.02f", i, sum);

        // for the next loop
        thCurrRecPtr += NrFeats;        
        if (i%1000==0) INFOTHDMSG("At record: %d", i);        
    }
             
    return true;
}

bool GpuCentroid::GpuComputeClusters(MLThreadData & thData, UInt32 threadCommand)
{

    return true;
}

__inline UInt32 GpuCentroid::GetRealIdx(UInt32 idx, float IsPos)
{
    if (IsPos) return IndexPosRec[idx];
    return IndexNegRec[idx];
}
