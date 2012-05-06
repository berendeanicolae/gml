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
		THREAD_COMMAND_WINDOW_UPDATE,
		//Add extra thread commands here
	};	

private:
	// structures use for thread map actions
	struct MapTestSpeedCompute {
		int nrLines;
		int nrRecords;
		pvm_float * buffer;
	};

	// data structures to be used by threads during Map sessions
	MapTestSpeedCompute mapTsc;		

	struct MapWindowUpdate {
		pvm_float* uALPH; // winsz * nrrec
		pvm_float* uSIGM; // winsz
		pvm_float* san; // winsz
		pvm_float* pn;
		
		pvm_float* ALPH;
		pvm_float* SIGM;

		UInt32 winStart;
		UInt32 winSize;
		bool updateNeeded;
		pvm_float score;
		PreCache::BlockLoadHandle *bHandle;
	};

	MapWindowUpdate wu;

	enum {
		COMMAND_NONE = 0,
        COMMAND_DEBUG_TESTS,
		COMMAND_PRECOMP_GRAM,
		COMMAND_MERGE_KPRIME,
		COMMAND_PRECOMP_NORM,
		COMMAND_INIT_STATE_VARS,
		COMMAND_BLOCK_TRAINING,
		COMMAND_LAST_BLOCK_TRAINING,
		COMMAND_GATHER_BLOCK_STATES,
		//Add extra commands here
	};	

	struct UpdateStr {
		pvm_float* alpha;
		pvm_float  sigmaVal;
		pvm_float  b;
		pvm_float  score;
		pvm_float  firstMember;
	};

	struct StateFileHeader {
		UInt32 blkNr;
		UInt32 recCount;
		UInt32 recStart;
		UInt32 totalRecCount;
	};


private:
	GML::ML::MLRecord		MainRecord;	

	// related class instances
	PreCache InstPreCache;
	void ProbVectorMachine::PreCacheInstanceInit();

	void	OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);
	bool	OnInitThreadData(GML::Algorithm::MLThreadData &thData);

    bool    TestMachineSpeed();
	bool	PreCacheCall(UInt32 cmd);
	bool	IterateBlockTraining();

	bool	PerfomBlockTraining(UInt32 blkIdx, PreCache::BlockLoadHandle *handle);
	bool	PerformWindowUpdate(GML::Algorithm::MLThreadData &thData);

	inline  pvm_float KerAt(UInt32 line,UInt32 row, pvm_float* ker, UInt32 nrRec);

public:
	ProbVectorMachine();

	bool	Init();
	void	OnExecute();    
    bool	ThreadTestCompSpeed(GML::Algorithm::MLThreadData & thData);
	bool	LastBlockTraining();
	bool DumpDefaultStateVariables();
	bool GatherBlockStates();
	// variables to control the algorithm flow
	UInt32 varKernelType;
	UInt32 varBlockFileSize;
	UInt32 varBlockStart;
	UInt32 varBlockCount;

	double varLambda;
	pvm_float varT;
	UInt32 varWindowSize, varIterNr;

	Int32		varKernelParamInt;
	pvm_double  varKernelParamDouble;

	GML::Utils::GString	varBlockFilePrefix;
	GML::Utils::GString	varFeatureWeightFile;
};

#endif

