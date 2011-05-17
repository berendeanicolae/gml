#ifndef __MAP_TEMPLATE__
#define __MAP_TEMPLATE__

#include "gmllib.h"

struct Cluster
{
	double	*Weight;
	UInt32	Count;
	UInt32	ElementsCount;
};
struct ClustersList
{
	Cluster	*Clusters;
	UInt32	Count;

	bool	Create(UInt32 clusterCount,UInt32 featCount);
};

class KMeans: public GML::Algorithm::IMLAlgorithm
{
	enum {
		INITIAL_RANDOM_VALUES,
		INITIAL_RANDOM_ELEMENTS,
	};

	ClustersList			Clusters;
	UInt32					K;
	UInt32					MaxIterations;
	UInt32					InitialClusters;
	GML::Utils::GString		tempStr;


	void					OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);
	bool					OnInitThreadData(GML::Algorithm::MLThreadData &thData);

	bool					ComputeDistances(GML::Algorithm::MLThreadData &thData);

	bool					Train(UInt32 iteration);
	void					PerformTrain();
public:
	KMeans();

	bool					Init();
	void					OnExecute();
};

#endif

