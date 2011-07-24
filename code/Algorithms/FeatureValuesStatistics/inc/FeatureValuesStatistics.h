#ifndef __FeatureValuesStatistics__
#define __FeatureValuesStatistics__

#include "gmllib.h"

struct ValueInfos
{
	double	Value;
	double	Label;
};

struct FeatureValuesStatisticsThreadData
{
	UInt32	FeatIndex;
};

class FeatureValuesStatistics: public GML::Algorithm::IMLAlgorithm
{
	enum {
		COMMAND_NONE = 0,
		COMMAND_COMPUTE,		
	};
	enum {
		THREAD_COMMAND_NONE = 0,
		THREAD_COMMAND_COMPUTE_VALUES
		//Add extra thread commands here
	};
	GML::ML::MLRecord					MainRecord;
	GML::Utils::GString					ResultFile;
	GML::Utils::GTFVector<ValueInfos>	Vi;
	GML::Utils::GTFVector<double>		Labels;
	
	bool								InitLabels();
	
	void								OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);
	bool								OnInitThreadData(GML::Algorithm::MLThreadData &thData);
	bool								ComputeValues(GML::Algorithm::MLThreadData &thData);
	bool								ComputeValuesForFeature(UInt32 index,GML::Utils::File &f);
	bool								Compute();							
public:
	FeatureValuesStatistics();

	bool				Init();
	void				OnExecute();
};

#endif

