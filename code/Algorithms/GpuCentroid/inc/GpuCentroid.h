#ifndef __GpuCentroid__
#define __GpuCentroid__

#include <CL/opencl.h>
#include <cstring>
#include <climits>

#include "gmllib.h"

#define ONEMEGABYTE     (1024*1024)
#define ONEKILOBYTE     (1024)

#define MAX_INST_PLATF  5
#define MAX_INST_DEV    10

#define GPU_NR_KERNELS      1
#define GPU_MAX_MEM_POOLS   100

#define INFOMSG(...) { {char z123[4096]; sprintf_s(z123,4096, __VA_ARGS__) ;notif->Info("[%s] -> %s",ObjectName, z123);} };
#define INFOTHDMSG(...) { {char z123[4096]; sprintf_s(z123,4096, __VA_ARGS__) ;notif->Info("[%s] [Th:%d] -> %s",ObjectName, thData.ThreadID, z123);} };
#define ERRORMSG(...){ {char z123[4096]; sprintf_s(z123,4096, __VA_ARGS__) ;notif->Error("[%s] -> %s",ObjectName, z123);} };

#define NULLCHECKMSG(val, ...){ if (val==NULL) { ERRORMSG(__VA_ARGS__); return false; } };    

#define NULLCHECK(val) {if (val==NULL) { return false; }; };

#define CHECKMSG(val, ...){ if (val==false) { ERRORMSG(__VA_ARGS__); return false; }; };
#define CHECK(val){ if (val==false) return false;}; 

#define CLCHECK(val) { if (val!=CL_SUCCESS) return false;}
#define CLDISPLAYERR { notif->Error("[%s] -> Line: %d; OpenCL Error: %s", ObjectName, __LINE__, GetCLErrorMsg(ErrorCode));}
#define CLCHECKMSG(...) { if (ErrorCode!=CL_SUCCESS){ ERRORMSG(__VA_ARGS__); CLDISPLAYERR(ErrorCode); return false;}}

#define DRAWLINE    INFOMSG("-----------------------------------------------------------------------");

using namespace GML::Utils;
using namespace GML::DB;
using namespace GML::ML;
using namespace GML::Algorithm;


struct ThDataMinDist {
    float*  RecData;
    float*  CurrRec;
    float*  OutDist;

    size_t     NrRec;
    size_t     NrFeats;
    size_t     RecPerDev;    
};

struct GpuCentroidThreadData
{
	ThDataMinDist   MinDist;
};

class GpuCentroid: public GML::Algorithm::IMLAlgorithm
{
	enum {
		COMMAND_NONE = 0,
        COMMAND_COMPUTE,
		//Add extra commands here
	};
    enum {
        ALG_METHOD_POS_ONLY =0,
        ALG_METHOD_NEG_ONLY,
        ALG_METHOD_BOTH
    };
	enum {
		THREAD_COMMAND_NONE = 0,
        THREAD_COMMAND_MIN_DIST_POS2NEG,
        THREAD_COMMAND_MIN_DIST_NEG2POS,
        THREAD_COMMAND_EXTRACT_POSITIVE_CLUSTERS,
        THREAD_COMMAND_EXTRACT_NEGATIVE_CLUSTERS
		//Add extra thread commands here
	};

    struct GpuKernelInfo {
        cl_kernel       Kernel;
        size_t          WorkGroupSz;
        size_t          LocalMemSz;
    };

    struct GpuCommand {
        cl_command_queue    Cmd;
        GpuKernelInfo       kern[GPU_NR_KERNELS];
        cl_device_id        DevId;        
        cl_ulong            MaxAllocSz;
        cl_ulong            MaxGlobSz;
        cl_ulong            MaxLocalSz;
    };

    struct GpuMemPool {
        cl_mem      Mem;
        cl_ulong    NrRec;
    };

	GML::ML::MLRecord		MainRecord;

	void				OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);
	bool				OnInitThreadData(GML::Algorithm::MLThreadData &thData);
public:
	GpuCentroid();
    ~GpuCentroid();

	bool				Init();
	void				OnExecute();    
protected:
    // public configuration parameters
    GML::Utils::GString		optPosCentFileName, optNegCentFileName;
    UInt32      optMinCentroidSize, optMethod;
    bool        optOutputClusters;

    // methods
    bool MainAlgorithm();
    bool CreateAlgorithmDatabases();
    bool InitializeGpuInfo();

    char *GetCLErrorMsg(cl_int err);
    bool GpuComputeMain(MLThreadData &thData, UInt32 ThreadCommand);
    bool GpuComputeClusters(MLThreadData & thData, UInt32 threadCommand);
private:

    __inline UInt32 GetRealIdx (UInt32 idx, float IsPos);

    float   *PtrPosData, *PtrNegData;
    UInt32  *IndexPosRec, *IndexNegRec;
    int     NrPosRec, NrNegRec;
    int     NrFeats;

    float   ThreadsResultMinDist[MAX_INST_DEV];               

    // gpu related variables
    cl_platform_id      clPlatform;
    cl_device_id        clDeviceIds[MAX_INST_DEV];    

    GpuMemPool  clMemPoolPos [GPU_MAX_MEM_POOLS]; cl_ulong NrMemPoolPos;
    GpuMemPool  clMemPoolNeg [GPU_MAX_MEM_POOLS]; cl_ulong NrMemPoolNeg;

    cl_uint             clNrPlatforms;
    cl_uint             clNrDevices;
    cl_context          clContext;

    GpuCommand              *gcmd;
    GpuCentroidThreadData   GcThData;

    float * DistVectPos2Neg;
    float * DistVectNeg2Pos;

    bool EnableDebug;
    int  progress[MAX_INST_DEV];
};

#endif

