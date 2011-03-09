#include "gmllib.h"

struct FeaturesInfo
{
	UInt32 PozitiveCount;
	UInt32 NegativeCount;
};
struct FeaturesInformations
{
	double			countPozitive;
	double			countNegative;
	double			totalPozitive;
	double			totalNegative;
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

#define STATS_FNC_COUNT	4

class FeaturesStatistics: public GML::Algorithm::IAlgorithm
{
	GML::DB::IDataBase				*db;
	GML::ML::IConnector				*con;
	GML::Utils::ThreadParalelUnit	*tpu;
	FeaturesThreadData				All;
	Stats							StatsData[STATS_FNC_COUNT];
public:
	FeaturesThreadData				*fData;
private:
	// proprietati
	GML::Utils::GString				Conector;
	GML::Utils::GString				DataBase;
	GML::Utils::GString				Notifier;

	UInt32							threadsCount;

	bool							CreateFeaturesInfo(FeaturesThreadData *fInfo);
	bool							Compute();
	void							PrintStats();
public:
	FeaturesStatistics();

	void							OnRunThreadCommand(FeaturesThreadData &ftd,UInt32 command);
	bool							Init();
	void							OnExecute(char *command);
};


