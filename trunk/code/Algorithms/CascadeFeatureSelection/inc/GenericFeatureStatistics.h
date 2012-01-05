#ifndef __GenericFeatureStatistics__
#define __GenericFeatureStatistics__

#include "gmllib.h"

#define MAX_PATH_DEPTH	64

struct FeatureCounters
{
	UInt32	CountPozitive;
	UInt32	CountNegative;
	UInt32	CountTotalPozitive;
	UInt32	CountTotalNegative;
	
	void	Reset();
	void	Add(FeatureCounters &fc);
};
struct FeatureScore
{
	double	Score;
	UInt32	Index;
};
struct CascadeFeatureSelectionThreadData
{
	GML::Utils::GTFVector<FeatureCounters>	Counters;
	UInt32									workingRecordsCount;
	// add thread specific data here
};
class Stats
{
public:
	GML::Utils::GString		Name;
	GML::ML::FeatStatComputeFunction fnCompute;
	
	Stats();
	Stats(Stats &ref);
};

class GenericFeatureStatistics: public GML::Algorithm::IMLAlgorithm
{
protected:
	enum {
		COMMAND_NONE = 0,
		COMMAND_COMPUTE,		
	};
	enum {
		THREAD_COMMAND_NONE = 0,
		THREAD_COMMAND_COMPUTE_FEATURES_COUNTERS,
		THREAD_COMMAND_REMOVE_RECORDS,
	};
	GML::ML::MLRecord						MainRecord;	
	GML::Utils::GTFVector<FeatureCounters>	FeatCounters;
	bool*									RemovedRecords;	
	bool*									RemovedFeatures;	
	GML::Utils::GTFVector<FeatureScore>		FeatScores;		
	GML::Utils::GString						ResultFileName;
	GML::Utils::GTVector<Stats>				StatsData;
	GML::ML::FeatureInformation				finf;	
	
	// variable inter thread
	GML::Utils::GString						featName;
	GML::Utils::GString						computeMethod;	
	UInt32									computeMethodIndex;
	UInt32									featToRemove;
	UInt32									workingRecordsCount;
	UInt32									TreePath[MAX_PATH_DEPTH];
	UInt32									TreePathSize;
	bool									callThreadComputeExtraDataFunction;
	
	void									OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);
	bool									OnInitThreadData(GML::Algorithm::MLThreadData &thData);
	bool									OnComputeFeatureCounters(GML::Algorithm::MLThreadData &thData);
	bool									OnComputeRemoveIndexes(GML::Algorithm::MLThreadData &thData);	
	bool									AddNewStatFunction(char *name,GML::ML::FeatStatComputeFunction);
	void									CreateWorkingList();
	double									ComputeScore(FeatureCounters &counter);	
	void									ComputeScoresAndSort();
	
	virtual bool							OnThreadComputeExtraData(UInt32 recordIndex,GML::Algorithm::MLThreadData &thData);
	virtual void							OnCompute()=0;
public:
	GenericFeatureStatistics();

	bool				Init();
	void				OnExecute();
};

#endif

