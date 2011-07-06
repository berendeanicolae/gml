#ifndef __MAP_TEMPLATE__
#define __MAP_TEMPLATE__

#include "gmllib.h"

struct FeaturesInfo
{
	UInt32 PozitiveCount;
	UInt32 NegativeCount;
};

struct BDTThreadData
{
	GML::Utils::GTFVector<FeaturesInfo>		FeaturesCount;
	GML::Utils::GTFVector<UInt32>			*IndexRecords;
	UInt32									totalPozitive;
	UInt32									totalNegative;

	void	Clear();
	void	Add(BDTThreadData &obj);
	bool	Create(UInt32 nrFeatures);
};

class BinaryDecisionTree: public GML::Algorithm::IMLAlgorithm
{
	GML::ML::MLRecord		MainRecord;


	void					OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);
	bool					OnInitThreadData(GML::Algorithm::MLThreadData &thData);

	bool					ComputeFeatureStats(GML::Algorithm::MLThreadData &thData);
	bool					ComputeFeaturesStatistics();

public:
	BinaryDecisionTree();

	bool					Init();
	void					OnExecute();
};

#endif

