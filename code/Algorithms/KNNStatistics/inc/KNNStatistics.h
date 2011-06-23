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
struct ComputePartsInfo
{
	double	SumSame,SumDiff;
	double	CountSame,CountDiff;
};
struct KNNStatThData
{
	GML::Utils::GTFVector<RecDist>	Dist;
	GML::ML::MLRecord				SecRec;
};
class KNNStatistics: public GML::Algorithm::IMLAlgorithm
{
	enum
	{
		test,
	};
protected:
	RecInfo					*rInfo;

	UInt32					K;
	UInt32					Method;
	GML::ML::MLRecord		MainRecord;

	void					OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);
	bool					OnInitThreadData(GML::Algorithm::MLThreadData &thData);

	void					ComputeParts(ComputePartsInfo &cpi,GML::Utils::GTFVector<RecDist> &Dist,UInt32 start,UInt32 end,double label,bool reset);
	bool					ComputeDist(GML::Algorithm::MLThreadData &thData);
	void					Compute();
public:
	KNNStatistics();

	bool					Init();
	void					OnExecute();
};

#endif

