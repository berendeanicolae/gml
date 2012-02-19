#ifndef __PRE_CACHE_H_
#define __PRE_CACHE_H_

#include "gmllib.h"
#include "PVMMacros.h"
#include "PVMDefinesInclude.h"
#include "PVMDefinesTypes.h"
#include "KernelWrapper.h"

#include <cstring>

#define PRECACHE_FILE_HEADER_MAGIC	 "PVM-PRECACHE-V1"
#define KPRIME_FILE_HEADER_MAGIC	 "PVM-KPRIME-V1"

using namespace GML::Utils;

class PreCache : GML::Utils::GMLObject {

public:

	struct InheritData {
		GML::ML::IConnector*	con;
		GML::Utils::INotifier*	notif;	

		UInt32		VarKernelType;
		pvm_float	VarKernelParamDouble;
		Int32		VarKernelParamInt;

		UInt32 VarPreCacheFileSize;
		UInt32 VarPreCacheBatchStart;
		UInt32 VarPreCacheBatchCount;

		GML::Utils::GString	VarPreCacheFilePrefix;
		
		~InheritData() {}
	};

private:

	struct PreCacheFileHeader {
		unsigned char Magic[32];
		UInt32 NrRecordsTotal;
		UInt32 NrFeatures;
		UInt32 RecordStart;
		UInt32 NrRecords;
	};

	struct KPrimePair {
		pvm_float pos;
		pvm_float neg;
	};

	struct Map_PreCacheComputeBatch {
		UInt32 CurrBatchNr;
		UInt32 RecPerBatch;
		UInt32 RecStart;
		UInt32 RecCount;
		UInt32 BatchNr;
		pvm_float* KernelBuffer;
		KPrimePair* KPrimeBuffer;
	};
	Map_PreCacheComputeBatch pccb;

	// configuration data
	InheritData id;
	GML::Algorithm::IMLAlgorithm* alg;
	GML::ML::IConnector*	con;
	GML::Utils::INotifier*	notif;

	UInt32 NrRec, NrFts;

	UInt32 TotalNrBatches;
	UInt32 RecPerBatch;

	UInt32 SizePerLine;

	// internal procs
	int GetNrRecPerBatch(int MinNr, int MaxNr);
	int GetSizeOfBatch(int BatchNr);
	bool GetKernelAt(int line, int row, pvm_float* KernelStorage, int NrRecInBatch, pvm_float *KVal);
	
public:
	// constructors
	PreCache();

	// destructor
	~PreCache();	

	// main procedure
	bool SetInheritData(PreCache::InheritData &InhData);
	bool SetParentAlg(GML::Algorithm::IMLAlgorithm* _alg);
	bool PreCompute();
	bool ThreadPrecomputeBatch(GML::Algorithm::MLThreadData &thData);
};

#endif //__PRE_CACHE_H_