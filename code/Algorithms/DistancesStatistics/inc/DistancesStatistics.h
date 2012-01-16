#ifndef __DistancesStatistics__
#define __DistancesStatistics__

#include "gmllib.h"

struct DistancesStatisticsThreadData
{
	GML::Utils::GTFVector<UInt32>		Histogram;
	GML::ML::MLRecord					SecRec;
};

class DistancesStatistics: public GML::Algorithm::IMLAlgorithm
{
	enum {
		COMMAND_NONE = 0,
		COMMAND_COMPUTE,		
	};
	enum {
		THREAD_COMMAND_NONE = 0,
		THREAD_COMMAND_COMPUTE_PositiveToNegative,
		THREAD_COMMAND_COMPUTE_PositiveToPositive,
		THREAD_COMMAND_COMPUTE_NegativeToPositive,
		THREAD_COMMAND_COMPUTE_NegativeToNegative,
		THREAD_COMMAND_COMPUTE_MinPositiveToNegative,
		THREAD_COMMAND_COMPUTE_MinPositiveToPositive,
		THREAD_COMMAND_COMPUTE_MinNegativeToPositive,
		THREAD_COMMAND_COMPUTE_MinNegativeToNegative,	
		THREAD_COMMAND_COMPUTE_FeaturesCount,		
	};
	enum {
		METHOD_PositiveToNegative = 0,
		METHOD_PositiveToPositive,
		METHOD_NegativeToPositive,
		METHOD_NegativeToNegative,
		METHOD_MinPositiveToNegative,
		METHOD_MinPositiveToPositive,
		METHOD_MinNegativeToPositive,
		METHOD_MinNegativeToNegative,
		METHOD_ComputeFeaturesCount,
	};
	GML::ML::MLRecord					MainRecord;
	GML::Utils::GTFVector<UInt32>		Histogram;
	GML::Utils::GString					ResultFile;
	GML::Utils::GString					FeaturesWeightFile;
	double								HistogramMinValue,HistogramMaxValue,HistogramStep;	
	UInt32								Method;
	bool								UseWeightsForFeatures;
	bool								Ignore0ValuesInHistogram;
	double								*featWeight;
	GML::Utils::Indexes					indexesPozitive,indexesNegative;

	bool 								LoadFeatureWeightFile();
	bool								CreatePozitiveAndNegativeIndexes();
	double 								GetDistance(GML::ML::MLRecord &r1,GML::ML::MLRecord &r2);
	void								OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);
	bool								OnInitThreadData(GML::Algorithm::MLThreadData &thData);
	UInt32								ValueToHistogramIndex(double value);
	bool								CreateHistogram(GML::Utils::GTFVector<UInt32> &histoObject,bool showFinalMessage);
	bool 								ComputeHistogram(GML::Algorithm::MLThreadData &thData,GML::Utils::Indexes &class1,GML::Utils::Indexes &class2);
	bool 								ComputeHistogramMin(GML::Algorithm::MLThreadData &thData,GML::Utils::Indexes &class1,GML::Utils::Indexes &class2);
	bool 								ComputeFeatureCountHistogram(GML::Algorithm::MLThreadData &thData);
	void								MergeThreadHistograms();
	bool								SaveHistogram();
	void								Compute();
public:
	DistancesStatistics();

	bool								Init();
	void								OnExecute();
};

#endif

