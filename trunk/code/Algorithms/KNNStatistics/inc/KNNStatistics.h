#ifndef __MAP_TEMPLATE__
#define __MAP_TEMPLATE__

#include "gmllib.h"

struct RecInfo
{
	double	Label;
	double	MinDistSimilar;
	double	MinDistNotSimilar;
	double	MaxDistSimilar;
	double	MaxDistNotSimilar;
	UInt16	ProcAdd;
	UInt16	ProcCount;
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
		Method_UseK = 0,

	};
protected:
	RecInfo					*rInfo;

	UInt32					K;
	UInt32					Method;
	//UInt32					columnWidth;
	GML::ML::MLRecord		MainRecord;
	GML::Utils::GString		ResultFileName;

	void					OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);
	bool					OnInitThreadData(GML::Algorithm::MLThreadData &thData);

	void					ComputeParts(ComputePartsInfo &cpi,GML::Utils::GTFVector<RecDist> &Dist,UInt32 start,UInt32 end,double label,bool reset);
	bool					ComputeDist(GML::Algorithm::MLThreadData &thData);
	void					Compute();

	bool					CreateRecordInfo(UInt32 index,GML::Utils::GString &str);
	bool					CreateHeaders(GML::Utils::GString &str);
	bool					SaveData();
public:
	KNNStatistics();

	bool					Init();
	void					OnExecute();
};

#endif

