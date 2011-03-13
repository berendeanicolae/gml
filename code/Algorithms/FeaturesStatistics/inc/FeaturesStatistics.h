#include "gmllib.h"

#define STATS_FNC_COUNT	6

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
	double			fnValue[STATS_FNC_COUNT];

	bool			operator< (FeaturesInformations &a);
	bool			operator> (FeaturesInformations &a);
};
struct Stats
{
	char *Name;
	double (*fnCompute) ( FeaturesInformations *info);

	void	Create(char *name,double (*_fnCompute) ( FeaturesInformations *info));
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
	GML::DB::IDataBase							*db;
	GML::ML::IConnector							*con;
	GML::Utils::ThreadParalelUnit				*tpu;
	FeaturesThreadData							All;
	Stats										StatsData[STATS_FNC_COUNT];
	GML::Utils::GTVector<FeaturesInformations>	ComputedData;
	GML::Utils::GString							SortProps;
public:
	FeaturesThreadData				*fData;
private:
	// proprietati
	GML::Utils::GString				Conector;
	GML::Utils::GString				DataBase;
	GML::Utils::GString				Notifier;
	GML::Utils::GString				ResultFile;

	UInt32							threadsCount;
	UInt32							columnWidth;
	UInt32							sortBy;
	UInt32							sortDirection;
	bool							notifyResults;

	bool							CreateFeaturesInfo(FeaturesThreadData *fInfo);
	bool							Compute();
	void							PrintStats();
	void							SaveToFile();
	bool							CreateHeaders(GML::Utils::GString &str);
	bool							CreateRecordInfo(FeaturesInformations &finf,GML::Utils::GString &str);
	void							Sort();
public:
	FeaturesStatistics();

	void							OnRunThreadCommand(FeaturesThreadData &ftd,UInt32 command);
	bool							Init();
	void							OnExecute(char *command);
};


