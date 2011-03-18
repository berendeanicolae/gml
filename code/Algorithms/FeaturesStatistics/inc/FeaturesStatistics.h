#include "gmllib.h"

struct FeaturesInfo
{
	UInt32 PozitiveCount;
	UInt32 NegativeCount;
};
class FeaturesInformations
{
public:
	UInt32			Index;
	double			countPozitive;
	double			countNegative;
	double			totalPozitive;
	double			totalNegative;
	double			*fnValue;

	bool			operator< (FeaturesInformations &a);
	bool			operator> (FeaturesInformations &a);
};
class Stats
{
public:
	GML::Utils::GString		Name;
	double					(*fnCompute) ( FeaturesInformations *info);	
	
	Stats();
	Stats(Stats &ref);
};
struct FeaturesThreadData
{
	GML::ML::MLRecord				Record;
	GML::Utils::Interval			Range;
	FeaturesInfo					*FI;
	UInt32							totalNegative;
	UInt32							totalPozitive;
public:
	FeaturesThreadData() { FI=NULL; }
};
class FeaturesStatistics: public GML::Algorithm::IAlgorithm
{
	enum
	{
		COMMAND_NONE = 0,
		COMMAND_COMPUTE
	};
	GML::DB::IDataBase							*db;
	GML::ML::IConnector							*con;
	GML::Utils::ThreadParalelUnit				*tpu;
	FeaturesThreadData							All;
	GML::Utils::GTVector<Stats>					StatsData;
	GML::Utils::GTVector<FeaturesInformations>	ComputedData;
	GML::Utils::GString							SortProps,WeightFileType;
public:
	FeaturesThreadData				*fData;
private:
	// proprietati
	GML::Utils::GString				Conector;
	GML::Utils::GString				DataBase;
	GML::Utils::GString				Notifier;
	GML::Utils::GString				ResultFile;
	GML::Utils::GString				FeaturesWeightFile;

	UInt32							threadsCount;
	UInt32							columnWidth;
	UInt32							sortBy;
	UInt32							saveFeatureWeightFile;
	UInt32							sortDirection;
	bool							notifyResults;
	double							multiplyFactor;

	bool							AddNewStatFunction(char *name,double (*_fnCompute) ( FeaturesInformations *info));
	bool							CreateFeaturesInfo(FeaturesThreadData *fInfo);
	bool							Compute();
	void							PrintStats();
	void							SaveToFile();
	void							SaveFeatureWeightFile();
	bool							CreateHeaders(GML::Utils::GString &str);
	bool							CreateRecordInfo(FeaturesInformations &finf,GML::Utils::GString &str);
	void							Sort();
public:
	FeaturesStatistics();

	void							OnRunThreadCommand(FeaturesThreadData &ftd,UInt32 command);
	bool							Init();
	void							OnExecute();
};


