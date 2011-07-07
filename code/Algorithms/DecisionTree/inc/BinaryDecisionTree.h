#ifndef __MAP_TEMPLATE__
#define __MAP_TEMPLATE__

#include "gmllib.h"

struct FeaturesInfo
{
	UInt32	Index;
	UInt32	PozitiveCount;
	UInt32	NegativeCount;	
	double	Score;
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
	enum {
		COMMAND_NONE = 0,
		COMMAND_TRAIN
	};

	enum {
		COMPUTE_SCORE_IG = 0,
		COMPUTE_SCORE_SUM,
 		COMPUTE_SCORE_FREQ 
	};

	GML::ML::MLRecord				MainRecord;
	GML::Utils::GTFVector<UInt8>	RecordsStatus;
	GML::Utils::GString				HashBaseFileName;
	UInt32							ComputeScoreMethod;


	void					OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);
	bool					OnInitThreadData(GML::Algorithm::MLThreadData &thData);

	bool					ComputeFeatureStats(GML::Algorithm::MLThreadData &thData);
	bool					ComputeFeaturesStatistics(GML::Utils::GTFVector<UInt32> *Indexes,BDTThreadData	&all);
	void					ComputeScore(BDTThreadData	&all,double (*fnComputeScore)(FeaturesInfo &fi,UInt32 totalPozitive,UInt32 totalNegative));
	bool					SaveHashesForFeature(char *fileName,GML::Utils::GTFVector<UInt32> *Indexes,UInt32 featIndex,bool featureValue);
	bool					PerformTrain();
	void					PerformComputeScore(BDTThreadData	&all);

public:
	BinaryDecisionTree();

	bool					Init();
	void					OnExecute();
};

#endif

