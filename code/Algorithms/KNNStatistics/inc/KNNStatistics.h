#ifndef __MAP_TEMPLATE__
#define __MAP_TEMPLATE__

#include "gmllib.h"

struct RecInfo
{
	double	Label;
	double	Dist;
};
struct RecDist
{
	UInt32	Index;
	double	Dist;
};
struct KNNStatThData
{
	GML::Utils::GTFVector<RecDist>	Dist;
	GML::ML::MLRecord				SecRec;
};
class KNNStatistics: public GML::Algorithm::IMLAlgorithm
{
protected:
	RecInfo					*rInfo;
	void					OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);
	bool					OnInitThreadData(GML::Algorithm::MLThreadData &thData);

	bool					ComputeDist(GML::Algorithm::MLThreadData &thData);
	void					Compute();
public:
	KNNStatistics();

	bool					Init();
	void					OnExecute();
};

#endif

