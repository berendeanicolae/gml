#ifndef __PRE_CACHE_H_
#define __PRE_CACHE_H_

#include "gmllib.h"
#include "PVMMacros.h"
#include "PVMDefinesInclude.h"
#include "PVMDefinesTypes.h"
#include "KernelWrapper.h"

#include "TemplateFunctions.h"
#include <cstring>
#include <cmath>

#define PRECACHE_FILE_HEADER_MAGIC	  "PVM-PRECACHE-V1"
#define KPRIME_FILE_HEADER_MAGIC	  "PVM-KPRIME-V1"
#define EQNORM_FILE_HEADER_MAGIC	  "PVM-EQNORM-V1"

#define PRECACHE_FILE_HEADER_MAGIC_SZ	32
#define PRECACHE_NR_WORK_BUFFERS		2

//---------------------------------------------------------------------------
typedef IMP::Vector<pvm_float> pvmFloatVectorT;
typedef IMP::Vector<pvm_float *> pvmFloatRefVectorT;
//---------------------------------------------------------------------------
using namespace GML::Utils;

class PreCache : GML::Utils::GMLObject {

public:

	struct InheritData {
		GML::ML::IConnector*	con;
		GML::Utils::INotifier*	notif;	

		UInt32		varKernelType;
		double		varKernelParamDouble;
		Int32		varKernelParamInt;

		UInt32 varBlockFileSize;
		UInt32 varBlockStart;
		UInt32 varBlockCount;
		UInt32 varBlockCountTotal;

		GML::Utils::GString	varBlockFilePrefix;				
	};

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

	typedef IMP::Vector<KPrimePair> KPrimePairVectorT;

	struct BlockLoadHandle {
		UInt32 blkNr;
		UInt32 recCount;
		UInt32 recStart;

		pvm_float* KERN;
		pvm_float* NORM;
		KPrimePair* KPRM;
	};

private:

	enum FileType {
		FileTypeKernel,
		FileTypeKPrime,
		FileTypeNorm
	};

	static DWORD WINAPI ThreadWrapper(LPVOID args);

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

	UInt32 TotalNrBlocks;
	UInt32 RecPerBlock;

	UInt32 SizePerLine;

	// internal procedures
	int GetNrRecPerBlock(int MinNr, int MaxNr);
	int GetNrRecPerBlockNonRecursive(int MaxNr);
	int GetSizeOfBlock(int BlockNr);	
	bool GetBlockFileName (UInt32 BlockId, FileType fType, GML::Utils::GString &BlockFileName);
	
	// asynchronous block loading procedure
	DWORD AtLoadNextBlock();
	

	UInt32 AtBlockId; 
	bool AtKillThread;
	HANDLE AtEventWorking, AtEventWaiting;
	
	unsigned char *AtBuffer[PRECACHE_NR_WORK_BUFFERS];
	PreCache::BlockLoadHandle blockHandle[PRECACHE_NR_WORK_BUFFERS];

	UInt32 AtIdxLoading, AtIdxExecuting;
		
public:
	// constructors
	PreCache();
	~PreCache();

	// main procedure
	bool SetInheritData(PreCache::InheritData &InhData);
	bool SetParentAlg(GML::Algorithm::IMLAlgorithm* _alg);
	bool PreComputeGram();
	bool ThreadPrecomputeBlock(GML::Algorithm::MLThreadData &thData);

	bool GetKernelAt(UInt32 line,UInt32 row, pvm_float* KernelStorage,UInt32 NrRecInBlock, UInt32 RecStart, pvm_float *KVal);

	// asynchronous block loading interface
	bool AtInitLoading();
	bool AtSignalStartLoading(UInt32 blockId);
	BlockLoadHandle* AtWaitForCompletion();

	// merge kprime files 
	bool MergeKPrimeFiles();
	bool PreComputeNorm();	
};

#endif //__PRE_CACHE_H_