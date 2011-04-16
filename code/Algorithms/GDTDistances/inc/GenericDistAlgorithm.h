#ifndef __GEN_DIST_ALG__
#define __GEN_DIST_ALG__

#include "gmllib.h"

struct DistThreadData
{
	GML::ML::MLRecord	SetRec;
};

#define SAVE_CACHE_SIZE		1024

class GenericDistAlgorithm : public GML::Algorithm::IMLAlgorithm
{
protected:
	enum {
		COMMAND_NONE = 0,
		COMMAND_COMPUTE
	};
	enum {
		SAVE_HASHES_AS_TEXT = 0,
		SAVE_HASHES_AS_BINARY
	};

	// proprietati
	GML::Utils::GString				HashFileName;
	UInt32							HashStoreMethod;

	GML::Utils::Indexes				indexesPozitive,indexesNegative;
	GML::Utils::GTFVector<UInt8>	RecordsStatus;

	
public:
	GenericDistAlgorithm();


	bool							OnInitThreadData(GML::Algorithm::MLThreadData &thData);
	virtual bool					OnInit();
	virtual bool					OnCompute();

	bool							SaveHashResult();
	bool							Init();
	void							OnExecute();
	bool							CreatePozitiveAndNegativeIndexes();
};


#endif

