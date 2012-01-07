#ifndef __FeaturesStatistics__
#define __FeaturesStatistics__

#include "gmllib.h"

struct FeaturesInformations
{
	UInt32			Index;	
	double			*fnValue;
	double			compareValue;
};
struct FeaturesStatisticsThreadData
{
	GML::ML::FeatureInformation		*Feats;
};

class FeaturesStatistics: public GML::Algorithm::IMLAlgorithm
{
	enum {
		COMMAND_NONE = 0,
		COMMAND_COMPUTE,		
	};
	enum {
		THREAD_COMMAND_NONE = 0,
		THREAD_COMMAND_COMPUTE,
	};
	GML::Utils::GTFVector<FeaturesInformations>	ComputedData;
	GML::Utils::GTFVector<GML::ML::FeatureInformation> Feats;
	GML::ML::MLRecord				MainRecord;
	GML::Utils::GString				SortProps,WeightFileType;
	UInt32							MinPoz,MaxPoz,MinNeg,MaxNeg;
	GML::Utils::GString				ResultFile;
	GML::Utils::GString				FeaturesWeightFile;
	UInt32							columnWidth;
	UInt32							featureColumnWidth;
	UInt32							sortBy;
	UInt32							saveFeatureWeightFile;
	UInt32							sortDirection;
	bool							notifyResults;
	bool							showFeatureName;
	bool							AdjustToNumberOfFeatures;
	double							multiplyFactor;

	UInt32							statFuncCount;
	
	bool 				CreateHeaders(GML::Utils::GString &str);
	bool 				CreateRecordInfo(FeaturesInformations &finf,GML::Utils::GString &str);
	bool 				Validate(GML::ML::FeatureInformation *fi);
	void 				PrintStats();
	void 				SaveToFile();
	void 				SaveFeatureWeightFile();
	void				OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);
	bool 				OnComputeFeatureCounters(GML::Algorithm::MLThreadData &thData);
	bool				OnInitThreadData(GML::Algorithm::MLThreadData &thData);
	void				Compute();
public:
	FeaturesStatistics();

	bool				Init();
	void				OnExecute();
};

#endif

