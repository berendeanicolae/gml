#ifndef __PRE_CACHE_H_
#define __PRE_CACHE_H_

#include "gmllib.h"
#include "PVMMacros.h"
#include "PVMDefinesInclude.h"
#include "PVMDefinesTypes.h"
#include "KernelWrapper.h"

#include <cstring>
#include <cmath>

#define PRECACHE_FILE_HEADER_MAGIC	 "PVM-PRECACHE-V1"
#define KPRIME_FILE_HEADER_MAGIC	 "PVM-KPRIME-V1"
#define EQNORM_FILE_HEADER_MAGIC	 "PVM-EQNORM-V1"

#define PRECACHE_FILE_HEADER_MAGIC_SZ 32

#define PRECACHE_NR_WORK_BUFFERS	2

using namespace GML::Utils;

class PreCache : GML::Utils::GMLObject {

public:

	struct InheritData {
		GML::ML::IConnector*	con;
		GML::Utils::INotifier*	notif;	

		UInt32		varKernelType;
		double		varKernelParamDouble;
		Int32		varKernelParamInt;

		UInt32 varPreCacheFileSize;
		UInt32 varPreCacheBlockStart;
		UInt32 varPreCacheBlockCount;

		GML::Utils::GString	varPreCacheFilePrefix;				
	};

private:

	struct PcThreadInfo {
		void*   Object;
		UInt32 	FuncId;
	};
	enum PcThreadFuncId {
		LoadBlock=0,
	};
	enum FileType {
		FileTypeKernel,
		FileTypeKPrime,
		FileTypeNorm
	};

	static DWORD WINAPI ThreadWrapper(LPVOID args);

	struct PreCacheFileHeader {
		char Magic[PRECACHE_FILE_HEADER_MAGIC_SZ];
		UInt32 NrRecordsTotal;
		UInt32 NrFeatures;
		UInt32 RecordStart;
		UInt32 NrRecords;
		UInt64 BlockSize;
	};

	struct KPrimePair {
		pvm_float pos;
		pvm_float neg;
	};

	struct Map_PreCacheComputeBlock {
		UInt32 CurrBlockNr;
		UInt32 RecPerBlock;
		UInt32 RecStart;
		UInt32 RecCount;
		UInt32 BlockNr;
		pvm_float* KernelBuffer;
		KPrimePair* KPrimeBuffer;
	};
	Map_PreCacheComputeBlock pccb;

	// configuration data
	InheritData id;
	GML::Algorithm::IMLAlgorithm* alg;
	GML::ML::IConnector*	con;
	GML::Utils::INotifier*	notif;

	UInt32 NrRec, NrFts;
	UInt32 NrPosRec, NrNegRec;

	UInt32 TotalNrBlockes;
	UInt32 RecPerBlock;

	UInt32 SizePerLine;

	// internal procs
	int GetNrRecPerBlock(int MinNr, int MaxNr);
	int GetSizeOfBlock(int BlockNr);
	bool GetKernelAt(UInt32 line,UInt32 row, pvm_float* KernelStorage,UInt32 NrRecInBlock, pvm_float *KVal);
	bool GetBlockFileName (UInt32 BlockId, FileType fType, GML::Utils::GString &BlockFileName);
	
	// asynchronous block loading procedure
	DWORD AtLoadNextBlock();
	UInt32 AtBlockId; 
	bool AtKillThread;
	HANDLE AtEventWorking, AtEventWaiting;
	unsigned char *AtBuffer[PRECACHE_NR_WORK_BUFFERS];
	UInt32 AtIdxLoading, AtIdxExecuting;
		
public:
	// constructors
	PreCache();

	// destructor
	~PreCache();	

	// main procedure
	bool SetInheritData(PreCache::InheritData &InhData);
	bool SetParentAlg(GML::Algorithm::IMLAlgorithm* _alg);
	bool PreCompute();
	bool ThreadPrecomputeBlock(GML::Algorithm::MLThreadData &thData);
	bool TestAtLoading();
	// merge kprime files 
	bool MergeKPrimeFiles();
	bool PreComputeNorm();
};

#endif //__PRE_CACHE_H_