#include "gmllib.h"

struct FeaturesInfo
{
	UInt32 PozitiveCount;
	UInt32 NegativeCount;
};
struct FeaturesThreadData
{
	GML::ML::MLRecord				Record;
	GML::Utils::Interval			Range;
	FeaturesInfo					*FI;
public:
	FeaturesThreadData() { FI=NULL; }
};
class FeaturesStatistics: public GML::Algorithm::IAlgorithm
{
	GML::DB::IDataBase				*db;
	GML::ML::IConnector				*con;
	GML::Utils::ThreadParalelUnit	*tpu;
	FeaturesThreadData				All;
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


