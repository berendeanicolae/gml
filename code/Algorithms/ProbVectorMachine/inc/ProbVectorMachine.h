#ifndef __ProbVectorMachine__
#define __ProbVectorMachine__

#include "gmllib.h"
#include "PVMDefinesInclude.h"
#include "PreCache.h"

class ProbVectorMachine: public GML::Algorithm::IMLAlgorithm
{

public:
	enum {
		THREAD_COMMAND_NONE = 0,
		THREAD_COMMAND_TEST_PROC_SPEED,
		THREAD_COMMAND_PRECOMPUTE_BATCH,
		//Add extra thread commands here
	};

private:
	// structures use for thread map actions
	struct Map_TestSpeedCompute
	{
		int nrLines;
		int nrRecords;
		pvm_float * buffer;
	};
	
	// data structures to be used by threads during Map sessions
	Map_TestSpeedCompute MapTsc;		

	enum {
		COMMAND_NONE = 0,
        COMNAND_TEST_MACHINE_SPEED,
		COMMAND_PRECOMPUTE,
		COMMAND_TEMP_KERNEL_FNCTS,
		//Add extra commands here
	};
	enum {
		KERN_TYPE_POLY = 0,
		KERN_TYPE_GAUS,
		//todo: asucila: add here mode kernel types
	};

private:
	GML::ML::MLRecord		MainRecord;	

	// related class instances
	PreCache InstPreCache;

	void	OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);
	bool	OnInitThreadData(GML::Algorithm::MLThreadData &thData);

    bool    TestMachineSpeed();
	bool	PreComputeKernelValues();

public:
	ProbVectorMachine();

	bool	Init();
	void	OnExecute();    
    bool	ThreadTestCompSpeed(GML::Algorithm::MLThreadData & thData);

	// variables to control the algorithm flow
	unsigned int VarKernelType;
	unsigned int VarPreCacheFileSize;
	unsigned int VarPreCacheBatchStart;
	unsigned int VarPreCacheBatchCount;

	GML::Utils::GString	VarPreCacheFilePattern;
};

#endif

