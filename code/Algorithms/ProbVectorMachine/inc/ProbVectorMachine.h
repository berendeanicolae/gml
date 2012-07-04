#ifndef __ProbVectorMachine__
#define __ProbVectorMachine__

#include "gmllib.h"
#include "PVMDefinesInclude.h"
#include "PreCache.h"
#include "TemplateStructures.h"


class ProbVectorMachine: public GML::Algorithm::IMLAlgorithm
{

public:
	enum {
		THREAD_COMMAND_NONE = 0,
		THREAD_COMMAND_TEST_PROC_SPEED,
		THREAD_COMMAND_PRECOMPUTE_BLOCK,
		THREAD_COMMAND_WINDOW_UPDATE,
		THREAD_COMMAND_WINDOW_UPDATE_DIRECT_PROJECT,
		THREAD_COMMAND_COMPUTE_SCORE,
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
		COMMAND_CLASSIFY,
		COMMAND_BLOCK_SCORE_COMPUTATION,
		//Add extra commands here
	};	

	struct UpdateStr {
		pvm_float* alpha;
		pvm_float  sigmaVal;
		pvm_float  b;
		pvm_float  score;
		pvm_float  firstMember;
		bool	   infeas_eq;

		void reset(int recCount);
	};

	struct StateFileHeader {
		UInt32 blkNr;
		UInt32 recCount;
		UInt32 recStart;
		UInt32 totalRecCount;
	};

	pvmFloatVectorT scoreUpdates;
	pvmFloatRefVectorT kerHelper;



private:
	GML::ML::MLRecord		MainRecord;	
	GML::ML::IConnector		*conectorTest;

	// related class instances
	PreCache InstPreCache;

	void ProbVectorMachine::PreCacheInstanceInit();

	void	OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);
	bool	OnInitThreadData(GML::Algorithm::MLThreadData &thData);

    bool    TestMachineSpeed();
	bool	PreCacheCall(UInt32 cmd);
	bool	IterateBlockTraining();

	bool	PerformBlockTraining(UInt32 blkIdx, PreCache::BlockLoadHandle *handle);
	bool	PerformWindowUpdate(GML::Algorithm::MLThreadData &thData);

	bool	PerformBlockTrainingDirectProjection(UInt32 blkIdx, PreCache::BlockLoadHandle *handle);	
	bool	PerformWindowUpdateDirectProjection(GML::Algorithm::MLThreadData &thData);

	
	bool	BlockScoreComputation();
	bool	PrepareAndExecuteBlockScoreComputation(UInt32 blkIdx, PreCache::BlockLoadHandle *handle);
	bool	ComputeBlockScore(GML::Algorithm::MLThreadData &thData);

	void	PrepareKerHelper(pvm_float *ker_src, int line_count, int nrRec);
	inline  pvm_float KerAt(UInt32 line,UInt32 row, pvm_float *ker, UInt32 nrRec, UInt32 blockRecStart);
	inline	pvm_float KerAtHelper(UInt32 line, UInt32 row);

	bool	ProjectSolutionToValidAverages(pvm_float *alpha, pvm_float *sigma, pvm_float &b, PreCache::KPrimePair *kprime);
	bool	ProjectSolutionToHypeplanes(pvmFloatVectorT &w0, pvm_float b0, pvmFloatVectorT &w1, pvm_float b1, pvmFloatVectorT &x, int prefOff);
	bool	DistanceToSemiSpace(pvmFloatVectorT &w, pvm_float b, pvmFloatVectorT &xSol, double &dist);
	bool	ScaleSolution(pvmFloatVectorT &w0, pvm_float b0, pvmFloatVectorT &w1, pvm_float b1, pvmFloatVectorT &x);

public:
	ProbVectorMachine();

	bool	Init();
	void	OnExecute();    
    bool	ThreadTestCompSpeed(GML::Algorithm::MLThreadData & thData);
	bool	LastBlockTraining();
	bool	LastBlockTrainingDirectProjection();
	bool	DumpDefaultStateVariables();
	bool	GatherBlockStates();
	bool	GatherBlockStatesDirectProjection();
	bool	ClasifyDataset();
	bool	InitExtraConnections();
	void	DebugTesting1();
	void	DebugTesting(PreCache::BlockLoadHandle *handle, UInt32 nrRecords);
	// variables to control the algorithm flow
	UInt32 varKernelType;
	UInt32 varBlockFileSize;
	UInt32 varBlockStart;
	UInt32 varBlockCount;
	UInt32 varBlockCountTotal;

	double varLambda;
	double varT;
	UInt32 varWindowSize, varIterNr, varNrUpdatesPerNormBlock, vadNrUpdatesPerLastBlock;

	Int32		varKernelParamInt;
	pvm_double  varKernelParamDouble;

	GML::Utils::GString	varBlockFilePrefix;
	GML::Utils::GString	varFeatureWeightFile;
	GML::Utils::GString	varModelFile;
	GML::Utils::GString	varConectorTest;
};

#endif

