#ifndef __MAP_TEMPLATE__
#define __MAP_TEMPLATE__

#include "gmllib.h"

struct Cluster
{
	double	*Weight;
	double	Label;
	UInt32	Count;
	UInt32	ElementsCount;

	bool	Save(char *fname);
	bool	Load(char *fName);
};
struct ClustersList
{
	Cluster	*Clusters;
	UInt32	Count;

	bool	Create(UInt32 clusterCount,UInt32 featCount);
	bool	Save(char *fName);
};

class BinaryKMeans: public GML::Algorithm::IMLAlgorithm
{
	enum {
		INITIAL_RANDOM_VALUES,
		INITIAL_RANDOM_ELEMENTS,
	};
	enum {
		SAVE_METHOD_NORMAL=0,
		SAVE_METHOD_CSV,
		SAVE_METHOD_BOTH,
	};
	ClustersList			Clusters;
	UInt32					K;
	UInt32					MaxIterations;
	UInt32					InitialClusters;
	UInt32					ResultFileType;
	GML::Utils::GString		tempStr;
	GML::Utils::GString		ResultFileName;
	double					minRandomValue,maxRandomValue;
	bool					SaveAfterEachIteration;


	GML::ML::MLRecord		MainRecord;

	bool					SaveData(char *fName);

	bool					InitRandomValues();
	bool					InitRandomElements();

	void					OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);
	bool					OnInitThreadData(GML::Algorithm::MLThreadData &thData);

	bool					ComputeDistances(GML::Algorithm::MLThreadData &thData);

	bool					Train(UInt32 iteration);
	void					PerformTrain();
public:
	BinaryKMeans();

	bool					Init();
	void					OnExecute();
};

#endif

