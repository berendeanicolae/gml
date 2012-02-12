#ifndef __PRE_CACHE_H_
#define __PRE_CACHE_H_

#include "gmllib.h"
#include "PVMMacros.h"
#include "PVMDefinesInclude.h"
#include "PVMDefinesTypes.h"

#include "KernelWrapper.h"

#include <cstring>

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

		GML::Utils::GString	VarPreCacheFilePattern;
		
		~InheritData() {}
	};

private:

	struct Map_PreCacheComputeBatch {
		UInt32 CurrBatchNr;
		UInt32 RecStart;
		UInt32 RecCount;
		pvm_float* Buffer;
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