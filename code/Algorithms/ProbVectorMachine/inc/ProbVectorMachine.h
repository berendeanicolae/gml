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
		THREAD_COMMAND_PRECOMPUTE_BLOCK,
		//Add extra thread commands here
	};

private:
	// structures use for thread map actions
	struct Map_TestSpeedCompute {
		int nrLines;
		int nrRecords;
		pvm_float * buffer;
	};
	
	// data structures to be used by threads during Map sessions
	Map_TestSpeedCompute MapTsc;		

	enum {
		COMMAND_NONE = 0,
        COMMAND_DEBUG_TESTS,
		COMMAND_PRECOMP_GRAM,
		COMMAND_MERGE_KPRIME,
		COMMAND_PRECOMP_NORM,
		COMMAND_BLOCK_TRAINING,
		//Add extra commands here
	};	

	// state of the algorithm read from the disk, or initialized here
	pvm_float*	alphaOrig,*sigmaPOrig,*sigmaMOrig;

private:
	GML::ML::MLRecord		MainRecord;	

	// related class instances
	PreCache InstPreCache;
	void ProbVectorMachine::PreCacheInstInit();

	void	OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);
	bool	OnInitThreadData(GML::Algorithm::MLThreadData &thData);

    bool    TestMachineSpeed();
	bool	PreCacheCall(UInt32 cmd);
	bool	IterateBlockTraining();

public:
	ProbVectorMachine();

	bool	Init();
	void	OnExecute();    
    bool	ThreadTestCompSpeed(GML::Algorithm::MLThreadData & thData);
	bool PerfomBlockTraining(UInt32 blkIdx, PreCache::BlockLoadHandle *handle);

	// variables to control the algorithm flow
	UInt32 varKernelType;
	UInt32 varBlockFileSize;
	UInt32 varBlockStart;
	UInt32 varBlockCount;

	Int32		varKernelParamInt;
	pvm_double  varKernelParamDouble;

	GML::Utils::GString	varBlockFilePrefix;
	GML::Utils::GString	varFeatureWeightFile;
	GML::Utils::GString varAlgoIterationState;
};

#endif

