#ifndef __GEN_DIST_ALG__
#define __GEN_DIST_ALG__

#include "gmllib.h"



#define SAVE_CACHE_SIZE		1024

class GenericDistAlgorithm : public GML::Algorithm::IMLAlgorithm
{
protected:
	enum {
		COMMAND_NONE = 0,
		COMMAND_COMPUTE
	};

	// proprietati


	GML::Utils::Indexes				indexesPozitive,indexesNegative;
	GML::Utils::GTFVector<UInt8>	RecordsStatus;

	
public:
	GenericDistAlgorithm();
	

	virtual bool					OnInit();
	virtual bool					OnCompute()=0;

	bool							Init();
	void							OnExecute();
	bool							CreatePozitiveAndNegativeIndexes();
};


#endif

